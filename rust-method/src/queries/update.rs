use crate::Secretariat;
use crate::Student;
use crate::Materie;
use crate::Inrolare;


use crate::queries::where_clause::*;
use crate::queries::where_clause::parseaza_conditiile_where;

use crate::task1::calculeaza_medii_generale;

trait Updateable {
    fn update_table_field(&mut self, field: &str, value: &str) -> Result<(), String>;
}


impl Updateable for Student {
    fn update_table_field(&mut self, field: &str, value: &str) -> Result<(), String> {
        match field {
            "id" => 
                self.id = value
                    .parse::<usize>()
                    .map_err(|_| format!("ID invalid: {:?}", value))?,
            "nume" =>
                self.nume = value.to_string(),
            "an_studiu" =>
                self.an_studiu = value
                    .parse::<u8>()
                    .map_err(|_| format!("An studiu invalid: {:?}", value))?,
            "statut" => {
                match value {
                    "t" => self.statut = 't',
                    "b" => self.statut = 'b',
                    _ => return Err(format!("Statut invalid: {:?}", value)),
                }
            }
            "medie_generala" =>
                self.medie_generala = value
                    .parse::<f32>()
                    .map_err(|_| format!("Medie invalida: {:?}", value))?,
            _ =>
                return Err(format!("Camp invalid {:?}", field))
        }

        Ok(())
    }
}


impl Updateable for Materie {
    fn update_table_field(&mut self, field: &str, value: &str) -> Result<(), String> {
        match field {
            "id" =>
                self.id = value
                    .parse::<usize>()
                    .map_err(|_| format!("ID invalid: {:?}", value))?,
            "nume" =>
                self.nume = value.to_string(),
            "nume_titular" =>
                self.nume_titular = value.to_string(),
            _ =>
                return Err(format!("Camp invalid {:?}", field))
        }

        Ok(())
    }
}


impl Updateable for Inrolare {
    fn update_table_field(&mut self, field: &str, value: &str) -> Result<(), String> {
        match field {

            "id_student" =>
                self.id_student = value
                    .parse::<usize>()
                    .map_err(|_| format!("ID invalid pentru student: {:?}", value))?,
            "id_materie" =>
                self.id_materie = value
                    .parse::<usize>()
                    .map_err(|_| format!("ID invalid pentru metid_materie: {:?}", value))?,
            "note" => {
                let mut tokens = value.split_whitespace();

                let nota_laborator: f32 = tokens
                    .next()
                    .ok_or_else(|| "Note invalide".to_string())?
                    .parse()
                    .map_err(|_| "Note invalide".to_string())?;

                let nota_partial: f32 = tokens
                    .next()
                    .ok_or_else(|| "Note invalide".to_string())?
                    .parse()
                    .map_err(|_| "Note invalide".to_string())?;

                let nota_final: f32 = tokens
                    .next()
                    .ok_or_else(|| "Note invalide".to_string())?
                    .parse()
                    .map_err(|_| "Note invalide".to_string())?;

                self.note[0] = nota_laborator;
                self.note[1] = nota_partial;
                self.note[2] = nota_final; 
            }
            _ =>
                return Err(format!("Camp invalid {:?}", field))
        }

        Ok(())
    }
}


/// Functie generica
fn update_table<T: Updateable + Matchable>(
    entries: &mut [T],
    conditii: &Vec<Conditie>,
    field: &str, value: &str
) -> Result<(), String>
{
    for entry in entries.iter_mut() {
        match match_on_all_conditii(entry, conditii) {
            Ok(true) => (),
            Ok(false) => continue,
            Err(message) => return Err(message),
        }

        entry.update_table_field(field, value)?;
    }

    Ok(())
}


/* Template:
UPDATE <tabel> SET <camp> = <valoare> WHERE <conditie>;
UPDATE <tabel> SET <camp> = <valoare> WHERE <cond1> AND <cond2>;
*/
pub fn update(s: &mut Secretariat, query: &str) -> Result<(), String> {
    // Cauta "UPDATE"
    let update_pos = query.find("UPDATE").ok_or("Lipseste 'UPDATE'")?;
    let mut ptr = &query[update_pos + "UPDATE".len()..];
    ptr = ptr.trim_start();

    // Extrage <tabel>
    let set_pos = ptr.find("SET").ok_or("Lipseste 'SET'")?;
    let nume_tabela = ptr[..set_pos].trim();
    ptr = &ptr[set_pos + "SET".len()..];
    ptr = ptr.trim_start();

    // Extrage "<camp> = <valoare>"
    let where_pos = ptr.find("WHERE").ok_or("Lipseste 'WHERE'")?;
    let camp_valoare = ptr[..where_pos].trim();
    ptr = &ptr[where_pos + "WHERE".len()..];
    ptr = ptr.trim_start();

    // Imparte <camp> de <valoare>
    let eq_pos = camp_valoare.find('=').ok_or("Lipseste '=' in expresia de update")?;
    let camp = camp_valoare[..eq_pos].trim();
    let mut valoare = camp_valoare[eq_pos + 1..].trim();

    // Sterge ghilimele daca exista
    valoare = valoare.trim_matches('"').trim_matches('\'');

    // Extrage conditii (TOT ce este dupa WHERE)
    let mut str_conditii = ptr.trim().to_string();

    // Sterge ';' de la final
    if str_conditii.ends_with(';') {
        str_conditii.pop();
    }


    // Aici poti sa folosesti deja `nume_tabela`, `camp`, `valoare` si `str_conditii`
    // exemplu: parseaza conditii
    let conditii = parseaza_conditiile_where(&str_conditii)?;

    match nume_tabela {
        "studenti" => update_table(&mut s.studenti, &conditii, camp, valoare),
        "materii"  => update_table(&mut s.materii, &conditii, camp, valoare),
        "inrolari" => {
            let result = update_table(&mut s.inrolari, &conditii, camp, valoare);
            calculeaza_medii_generale(s);
            result
        }
        _ => Err(format!(
            "Tabela {:?} nu exista in baza de date a facultati!",
            nume_tabela
        ))
    }
}
