use crate::Secretariat;
use crate::Student;
use crate::Materie;
use crate::Inrolare;

use crate::queries::where_clause::*;


// Trait generic
trait Selectable {
    fn print_table_field(&self, field: &str) -> Result<(), String>;
}

impl Selectable for Student {
    fn print_table_field(&self, field: &str) -> Result<(), String> {
        match field {
            "id" => print!("{}", self.id),
            "nume" => print!("{}", self.nume),
            "an_studiu" => print!("{}", self.an_studiu),
            "statut" => print!("{}", self.statut),
            "medie_generala" =>
                print!("{:.2}", 
                    (self.medie_generala * 100.0f32).round() as f32 / 100.0f32),
            _ => return Err(format!("Camp invalid {:?}", field)),
        }
        Ok(())
    }
}

impl Selectable for Materie {
    fn print_table_field(&self, field: &str) -> Result<(), String> {
        match field {
            "id" => print!("{}", self.id),
            "nume" => print!("{}", self.nume),
            "nume_titular" => print!("{}", self.nume_titular),
            _ => return Err(format!("Camp invalid {:?}", field)),
        }
        Ok(())
    }
}


impl Selectable for Inrolare {
    fn print_table_field(&self, field: &str) -> Result<(), String> {
        match field {
            "id_student" => print!("{}", self.id_student),
            "id_materie" => print!("{}", self.id_materie),
            "note" => print!("{:.2} {:.2} {:.2}", 
                             self.note[0], self.note[1], self.note[2]),
            _ => return Err(format!("Camp invalid {:?}", field)),
        }
        Ok(())
    }
}

// Functie generica
fn select_from_table<T: Selectable + Matchable>(
    entries: &[T],
    campuri: &[&str],
    conditii: &Vec<Conditie>,
) -> Result<(), String> {
    for entry in entries {
        match match_on_all_conditii(entry, conditii) {
            Ok(true) => (),
            Ok(false) => continue,
            Err(message) => return Err(message),
        }

        for (idx, field) in campuri.iter().enumerate() {
            entry.print_table_field(field)?;
            if idx != campuri.len() - 1 {
                print!(" ");
            }
        }

        println!();
    }

    Ok(())
}






/* Template:
SELECT * FROM <tabel>;
SELECT <campuri> FROM <tabel>;
SELECT <campuri> FROM <tabel> WHERE <camp> <operator> <valoare>;
SELECT <campuri> FROM <tabel> WHERE <cond1> AND <cond2>;

Campuri - unul sau mai multe nume de coloane, despartite prin virgula ','
*/
pub fn select(s: &Secretariat, query: &str) -> Result<(), String> {
    let select_pos = query.find("SELECT")
        .ok_or_else(|| "SELECT nu a fost gasit in query".to_string())?;
    
    let mut ptr = &query[select_pos + "SELECT".len()..];
    ptr = ptr.trim_start();

    let from_pos = ptr.find("FROM")
        .ok_or_else(|| "FROM nu a fost gasit dupa SELECT".to_string())?;

    // Extrage <campuri>
    let str_campuri: &str = ptr[..from_pos].trim();
    let is_select_all: bool = str_campuri == "*";

    // Extrage <nume_tabela>
    ptr = &ptr[from_pos + "FROM".len()..];
    ptr = ptr.trim_start();

    let where_pos = ptr.find("WHERE");
    let nume_tabela = if let Some(pos) = where_pos {
        ptr[..pos].trim()
    } else {
        // pana la ';'
        if let Some(end) = ptr.find(';') {
            ptr[..end].trim()
        } else {
            ptr.trim()
        }
    };

    // Extrage <conditii>, daca exista WHERE
    let mut str_conditii = "";
    if let Some(pos) = where_pos {
        ptr = &ptr[pos + "WHERE".len()..];
        ptr = ptr.trim_start();
        if let Some(end) = ptr.find(';') {
            str_conditii = ptr[..end].trim();
        } else {
            str_conditii = ptr.trim();
        }
    }

    let campuri: Vec<&str>;
    if is_select_all {
        campuri = match nume_tabela {
            "studenti" => vec!["id", "nume", "an_studiu", "statut", "medie_generala"],
            "materii" => vec!["id", "nume", "nume_titular"],
            "inrolari" => vec!["id_student", "id_materie", "note"],
            _ =>  return Err(format!(
                "Tabela {:?} nu exista in baza de date a facultati!",
                nume_tabela))
        }
    } else {
        // Imarte campurile de ","
        campuri = str_campuri
            .split(',')
            .map(|s| s.trim())
            .collect();
    }

    let conditii: Vec<Conditie> = parseaza_conditiile_where(str_conditii)?;

    match nume_tabela {
        "studenti" => select_from_table(&s.studenti, &campuri, &conditii),
        "materii"  => select_from_table(&s.materii, &campuri, &conditii),
        "inrolari" => select_from_table(&s.inrolari, &campuri, &conditii),
        _ => Err(format!(
            "Tabela {:?} nu exista in baza de date a facultati!",
            nume_tabela
        ))
    }
}
