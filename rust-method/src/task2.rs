mod structuri;
mod task1;
mod task3;

use crate::structuri::*;

use std::{env, process::exit, vec};
use crate::task1::citeste_secretariat;
use std::io::{self, BufRead};


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


/* Template:
SELECT * FROM <tabel>;
SELECT <campuri> FROM <tabel>;
SELECT <campuri> FROM <tabel> WHERE <camp> <operator> <valoare>;
SELECT <campuri> FROM <tabel> WHERE <cond1> AND <cond2>;

Campuri - unul sau mai multe nume de coloane, despartite prin virgula ','
*/
fn SELECT(s: &Secretariat, query: &str) {
    let select_pos = query.find("SELECT");
    if select_pos.is_none() {
        return;
    }
    let mut ptr = &query[select_pos.unwrap() + "SELECT".len()..];

    ptr = ptr.trim_start();

    let from_pos = ptr.find("FROM");
    if from_pos.is_none() {
        return;
    }

    // Extrage <campuri>
    let str_campuri: &str = ptr[..from_pos.unwrap()].trim();
    let is_select_all: bool = str_campuri == "*";

    // Extrage <nume_tabela>
    ptr = &ptr[from_pos.unwrap() + "FROM".len()..];
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

    // Debug output
    println!("Campuri: '{}'", str_campuri);
    println!("Select all: {}", is_select_all);
    println!("Nume tabela: '{}'", nume_tabela);
    println!("Conditii: '{}'", str_conditii);

    let mut campuri: Vec<&str>;
    if is_select_all {
        campuri = match nume_tabela {
            "studenti" => vec!["id", "nume", "an_studiu", "statut", "medie_generala"],
            "materii" => vec!["id", "nume", "nume_titular"],
            "inrolari" => vec!["id_student", "id_materie", "note"],
            _ => { 
                eprintln!("[EROARE] Tabela {:?} nu exista in baza de date a facultati!", nume_tabela);
                return;
            }
        };
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
        eprintln!("Eroare la parsare");
        return;
    };

    println!("{:?}", conditii);
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
            SELECT(&secretariat, query);
        } else if query.starts_with("DELETE") {
        } else if query.starts_with("UPDATE") {
        } else {
            eprintln!("[EROARE] Interogare invalida!");
        }
    }
}
