#![cfg(test)]

use std::fs;
use crate::task1::citeste_secretariat;
use crate::task3::cripteaza_studenti;
use rstest::rstest;

// Funcție utilitară pentru compararea fișierelor
fn compara_fisiere(file1: &str, file2: &str) -> bool {
    let f1 = fs::read(file1).expect(&format!("[EROARE] la citirea fisierului {:?}", file1));
    let f2 = fs::read(file2).expect(&format!("[EROARE] la citirea fisierului {:?}", file2));
    f1 == f2
}

// Test parametrizat (table-based tests) cu rstest
#[rstest]
#[case(1)]
#[case(2)]
#[case(3)]
#[case(4)]
#[case(5)]
fn test_cripteaza_studenti(#[case] i: usize) {
    let db_input = format!("../tests/db/task3/test{}.db", i);
    let db_output = format!("../tests/output-rust-method/task3/test{}.db.enc", i);
    let db_ref = format!("../tests/ref/task3-rust-method/test{}.db.enc", i);

    let s = citeste_secretariat(&db_input);
    cripteaza_studenti(&s, "pclp1", "aa", &db_output);
    drop(s);

    assert!(
        compara_fisiere(&db_output, &db_ref),
        "Fisierul {} difera de referinta {}",
        db_output,
        db_ref
    );
}
