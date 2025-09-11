mod structuri;
mod task1;
mod task3;

mod queries {
    pub mod select;
    pub mod where_clause;
}

use crate::structuri::*;
use crate::queries::select::select;
use crate::queries::where_clause::*;


use std::fmt::format;
use std::{env, process::exit, vec};
use crate::task1::citeste_secretariat;
use std::io::{self, BufRead};

use std::str::FromStr;





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
