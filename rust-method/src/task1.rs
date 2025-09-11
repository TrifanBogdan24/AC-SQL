use crate::structuri::*;

use std::fs::File;
use std::io::{BufRead, BufReader};
use std::process;


enum TableType {
    TabelaStudenti,
    TabelaMaterii,
    TabelaInrolari,
    None
}


fn parseaza_intrare_student(linie: &str) -> Student {
    let mut tokens = linie.split(',');

    let id_str = tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();
    let nume = tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();
    let an_str = tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();
    let statut_str = tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();

    Student {
        id: id_str.parse().expect("ID invalid"),
        nume: nume.to_string(),
        an_studiu: an_str.parse().expect("An invalid"),
        statut: statut_str.chars().next().expect("Statut invalid"),
        medie_generala: 0.0,
    }
}


fn parseaza_intrare_materie(linie: &str) -> Materie {
    let mut tokens = linie.split(",");

    let id_str = tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();
    let nume = tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();
    let nume_titular = tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();

    Materie {
        id: id_str.parse().expect("ID invalid"),
        nume: nume.to_string(),
        nume_titular: nume_titular.to_string()
    }
}


fn parseaza_intrare_inrolare(linie: &str) -> Inrolare {
    let mut tokens = linie.split(','); 

    let id_student_str = tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();

    let id_materie_str = tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();

    let note_str = tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();

    let mut note_tokens = note_str.split_whitespace();

    let nota_laborator_str = note_tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();

    let nota_partial_str = note_tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();

    let nota_final_str = note_tokens.next()
        .expect("Fisierul .db nu respecta formatul dorit!")
        .trim();

    Inrolare {
        id_student: id_student_str.parse().unwrap(),
        id_materie: id_materie_str.parse().unwrap(),
        note: [
            nota_laborator_str.parse().unwrap(),
            nota_partial_str.parse().unwrap(),
            nota_final_str.parse().unwrap(),
        ],
    }
}


pub fn adauga_student(s: &mut Secretariat, id: u32, nume: &String, an_studiu: u8, statut: char, medie_generala: f32) {
    let student = Student {
        id: id,
        nume: nume.clone(),
        an_studiu: an_studiu,
        statut: statut,
        medie_generala: medie_generala
    };

    s.studenti.push(student);
}


fn parseaza_linie(linie: &str, s: &mut Secretariat, table_type: &TableType) {
    match table_type {
        TableType::TabelaStudenti => {
            let student = parseaza_intrare_student(linie);
            /* Pentru debug: println!("{:?}", student); */
            adauga_student(s, student.id, &student.nume, student.an_studiu, student.statut, student.medie_generala);
        }
        TableType::TabelaMaterii => {
            let materie = parseaza_intrare_materie(linie);
            /* Pentru debug: println!("{:?}", materie); */
            s.materii.push(materie);
        }
        TableType::TabelaInrolari => {
            let inrolare = parseaza_intrare_inrolare(linie);
            /* Pentru debug: println!("Inrolari: {:?}", inrolare); */
            s.inrolari.push(inrolare);
        }
        _ => ()   // Nu se intampla nimic
    }
}


pub fn calculeaza_medii_generale(s: &mut Secretariat) -> () {
    let mut idx_inrolare: usize = 0;
    let nr_inrolari: usize = s.inrolari.len();

    for student in s.studenti.iter_mut() {
        // Calculeaza media generala a unui student:
        let mut suma_notelor: f32 = 0.0f32;
        let mut nr_materii: usize = 0;

        while idx_inrolare < nr_inrolari
            && s.inrolari[idx_inrolare].id_student == student.id {
            suma_notelor += s.inrolari[idx_inrolare].note[0];
            suma_notelor += s.inrolari[idx_inrolare].note[1];
            suma_notelor += s.inrolari[idx_inrolare].note[2];

            nr_materii += 1;
            idx_inrolare += 1;
        }

        student.medie_generala = suma_notelor / (nr_materii as f32);
    }
}

pub fn citeste_secretariat(nume_fisier: &str) -> Secretariat {
    let mut secretariat = Secretariat::default();
    let file = File::open(nume_fisier).unwrap_or_else(|_| {
        eprintln!("Eroare: nu pot deschide fișierul {}", nume_fisier);
        process::exit(255);
    });

    let mut table_type = TableType::None;

    let reader = BufReader::new(file);

    for (i, line) in reader.lines().enumerate() {
        match line {
            Ok(line_string) => {
                match line_string.as_str() {
                    "[STUDENTI]" => table_type = TableType::TabelaStudenti,
                    "[MATERII]" => table_type = TableType::TabelaMaterii,
                    "[INROLARI]" => table_type = TableType::TabelaInrolari,
                    _ => parseaza_linie(&line_string, &mut secretariat, &table_type),
                }
            }
            Err(_) => {
                eprintln!("Eroare la citirea liniei {}.", i + 1);
                continue;
            }
        }
    }

    calculeaza_medii_generale(&mut secretariat);
    secretariat
}