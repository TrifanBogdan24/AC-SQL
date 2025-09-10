mod structuri;
mod task1;
mod task3;

use crate::structuri::*;

use std::fmt::format;
use std::{env, process::exit, vec};
use crate::task1::citeste_secretariat;
use std::io::{self, BufRead};

use std::str::FromStr;

#[derive(Debug)]
struct Conditie {
    camp: String,
    op_comp: String,
    valoare: String
}


fn parseaza_conditiile_where(str_conditii: &str) -> Result<Vec<Conditie>, String> {
    let mut conditii: Vec<Conditie> = Vec::new();

    // Mai intai spargem dupa "AND"
    let conditii_strings: Vec<&str> = str_conditii
        .split("AND")
        .map(|s| s.trim()) // elimina spatiile la inceput si sfarsit
        .filter(|s| !s.is_empty()) // ignore empty tokens
        .collect();

    for cond_str in conditii_strings {
        // Fiecare conditie trebuie sa aiba cel putin 3 token-uri: camp, operator, valoare
        let mut tokens = cond_str.splitn(3, ' '); // split in maxim 3 parti

        let camp = tokens.next().ok_or(format!("Camp lipsa in conditie: '{}'", cond_str))?.trim();
        let op_comp = tokens.next().ok_or(format!("Operator lipsa in conditie: '{}'", cond_str))?.trim();
        let valoare = tokens.next().ok_or(format!("Valoare lipsa in conditie: '{}'", cond_str))?.trim();

        // Elimina ghilimelele de la inceput si sfarsit
        let valoare = valoare.trim_matches('"').to_string();

        conditii.push(Conditie {
            camp: camp.to_string(),
            op_comp: op_comp.to_string(),
            valoare,
        });
    }

    Ok(conditii)
}


fn match_num_on_conditie<T>(num: T, cond: &Conditie) -> Result<bool, String>
where
    T: FromStr + PartialEq + PartialOrd + std::fmt::Debug,
{
    let mut valoare: T;
    if let Ok(num) = cond.valoare.parse() {
        valoare = num;
    } else {
        return Err(format!("Valoarea furnizata pentru parametrul <num> nu este un numar: {:?}", num));
    }

    match cond.op_comp.as_str() {
        "=" =>
            Ok(num == valoare),
        "!=" =>
            Ok(num != valoare),
        "<" =>
            Ok(num < valoare),
        ">" =>
            Ok(num > valoare),
        "<=" =>
            Ok(num <= valoare),
        ">=" =>
            Ok(num >= valoare),
        _ => Err(format!("Operator invalid de comparatie (numerica) {:?}", cond.op_comp))
    }
}


fn match_student_on_conditie(student: &Student, cond: &Conditie) -> Result<bool, String> {
    match cond.camp.as_str() {
        "id" =>
            return match_num_on_conditie(student.id, cond),
        "nume" => {
            match cond.op_comp.as_str() {
                "=" =>
                    return Ok(student.nume == cond.valoare),
                "!=" =>
                    return Ok(student.nume != cond.valoare),
            _ => return Err(format!(
                "Operator invalid de comparatie pentru campul \"nume\": {:?}",
                cond.op_comp))
            }
        }
        "an_studiu" =>
            return match_num_on_conditie(student.an_studiu, cond),
        "statut" => {
            match cond.valoare.as_str() {
                "t" =>
                    return Ok(student.statut == 't'),
                "b" =>
                    return Ok(student.statut == 'b'),
                _ => return Err(format!(
                    "Valoarea de comparatie {:?} este invalida pentru statutul unui student",
                    cond.valoare))
            }
        }
        "medie_generala" =>
            return match_num_on_conditie(student.medie_generala, cond),
        _ => return Err(format!("Campul {:?} este invalid pentru tabela \"studenti\"", cond.camp))
    }
}

fn match_materie_on_conditie(materie: &Materie, cond: &Conditie) -> Result<bool, String> {
    match cond.camp.as_str() {
        "id" =>
            return match_num_on_conditie(materie.id, cond),
        "nume" => {
            match cond.op_comp.as_str() {
                "=" =>
                    return Ok(materie.nume == cond.valoare),
                "!=" =>
                    return Ok(materie.nume != cond.valoare),
            _ => return Err(format!(
                "Operator invalid de comparatie pentru campul \"nume\": {:?}",
                cond.op_comp))
            }
        }
        "nume_titular" => {
            match cond.op_comp.as_str() {
                "=" =>
                    return Ok(materie.nume_titular == cond.valoare),
                "!=" =>
                    return Ok(materie.nume_titular != cond.valoare),
            _ => return Err(format!(
                "Operator invalid de comparatie pentru campul \"nume_titular\": {:?}",
                cond.op_comp))
            }
        }
        _ => return Err(format!("Campul {:?} este invalid pentru tabela \"materii\"", cond.camp))
    }
}




fn match_inrolare_on_conditie(inrolare: &Inrolare, cond: &Conditie) -> Result<bool, String> {
    match cond.camp.as_str() {
        "id_student" =>
            return match_num_on_conditie(inrolare.id_student, cond),
        "id_materie" =>
            return match_num_on_conditie(inrolare.id_student, cond),
        "note" => {
            let mut tokens = cond.valoare.split_whitespace();

            let nota_laborator: f32 = tokens
                .next()
                .ok_or("Lipseste nota laborator")?
                .parse()
                .map_err(|_| "Nota laborator invalida")?;

            let nota_partial: f32 = tokens
                .next()
                .ok_or("Lipseste nota partial")?
                .parse()
                .map_err(|_| "Nota partial invalida")?;

            let nota_final: f32 = tokens
                .next()
                .ok_or("Lipseste nota final")?
                .parse()
                .map_err(|_| "Nota final invalida")?;

            match cond.op_comp.as_str() {
                "=" =>
                    return Ok(
                        nota_laborator == inrolare.note[0]
                        && nota_partial == inrolare.note[1]
                        && nota_final == inrolare.note[2]
                    ),
                "!=" =>
                    return Ok(
                            nota_laborator != inrolare.note[0]
                            || nota_partial != inrolare.note[1]
                            || nota_final != inrolare.note[2]
                        ),
                _ => return Err(format!(
                    "Operator invalid de comparatie pentru campul \"note\": {:?}",
                    cond.op_comp))
                }
            }
        _ => return Err(format!("Campul {:?} este invalid pentru tabela \"inrolari\"", cond.camp))
    }
}



// Trait general
trait Matchable {
    fn matches_condition(&self, cond: &Conditie) -> Result<bool, String>;
}

// Implementări pentru tipurile tale
impl Matchable for Student {
    fn matches_condition(&self, cond: &Conditie) -> Result<bool, String> {
        match_student_on_conditie(self, cond)
    }
}

impl Matchable for Materie {
    fn matches_condition(&self, cond: &Conditie) -> Result<bool, String> {
        match_materie_on_conditie(self, cond)
    }
}

impl Matchable for Inrolare {
    fn matches_condition(&self, cond: &Conditie) -> Result<bool, String> {
        match_inrolare_on_conditie(self, cond)
    }
}


fn match_on_all_conditii<T: Matchable>(item: &T, conditii: &[Conditie]) -> Result<bool, String> {
    for cond in conditii {
        match item.matches_condition(cond) {
            Ok(true) => continue,
            Ok(false) => return Ok(false),
            Err(message) => return Err(message),
        }
    }
    Ok(true)
}




fn select_from_studenti(s: &Secretariat, campuri: &[&str], conditii: &Vec<Conditie>) -> Result<(), String> {
    for student in &s.studenti {
        match match_on_all_conditii(student, conditii) {
            Ok(true) => (),            // Afiseaza intrarea din tabela
            Ok(false) => continue,
            Err(message) => return Err(message)
        }

        for idx in 0..campuri.len() {
            match campuri[idx] {
                "id" => print!("{}", student.id),
                "nume" => print!("{}", student.nume),
                "an_studiu" => print!("{}", student.an_studiu),
                "statut" => print!("{}", student.statut),
                "medie_generala" => print!("{:.2}", student.medie_generala),
                _ => return Err(format!("Camp invalid {:?}", campuri[idx])),
            }

            if idx != campuri.len() - 1 {
                print!(" ");
            }
        }

        println!("");

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
fn select(s: &Secretariat, query: &str) -> Result<(), String> {
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

    let mut campuri: Vec<&str>;
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

    let conditii: Vec<Conditie> = if let Ok(array) = parseaza_conditiile_where(str_conditii) {
        array
    } else {
        return Err("Eroare la parsarea conditiilor!".to_string());
    };

    match nume_tabela {
        "studenti" => {
            match select_from_studenti(s, &campuri, &conditii) {
                Ok(_) => Ok(()),
                Err(message) => return Err(message)
            }
        }
        _ =>  Err(format!(
            "Tabela {:?} nu exista in baza de date a facultati!",
            nume_tabela))
    }
}


fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 2 {
        eprintln!("[EROARE] Numar invalid de argumente!");
        println!("[INFO] Programul se asteapta sa fie apelat in felul urmator:");
        println!("       cargo run <file.db>");
        exit(255);
    }

    let mut secretariat = citeste_secretariat(&args[1]);

    let mut input = String::new();

    // Citeste <n> = numarul de interogari
    io::stdin().read_line(&mut input)
        .expect("Eroare la citirea numarului de interogari");
    let nr_interogari: usize = input.trim().parse()
        .expect("Numar invalid");
    
    // Citeste interogarile
    let stdin = io::stdin();
    for _ in 0..nr_interogari {
        let mut query = String::new();
        stdin.read_line(&mut query)
            .expect("Eroare la citirea query-ului");

        let query = query.trim();

        if query.starts_with("SELECT") {
            match select(&secretariat, query) {
                Ok(_) => (),
                Err(message) => eprintln!("[EROARE] {:}", message)
            }
        } else if query.starts_with("DELETE") {
        } else if query.starts_with("UPDATE") {
        } else {
            eprintln!("[EROARE] Interogare invalida!");
        }
    }
}
