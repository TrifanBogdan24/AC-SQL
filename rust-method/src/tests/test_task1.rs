#![cfg(test)]

use crate::structuri::Secretariat;
use crate::task1::*;

const TEST_DB: &str = "../tests/db/small.db";
const EPSILONE: f32 = 1e-3;

#[test]
fn test_citire_secretariat() {
    let s: Secretariat = citeste_secretariat(TEST_DB);

    assert!(s.studenti.len() == 10, "Numar stdenti gresit!");
    assert!(s.materii.len() == 9, "Numar materii gresit!");
    assert!(s.inrolari.len() == 25, "Numar inrolari gresit!");

    assert!(
        s.studenti[4].nume == "Popescu Adrian".to_string()
        && s.studenti[4].id == 4
        && s.studenti[4].an_studiu == 3
        && s.studenti[4].statut == 'b'
        && (s.studenti[4].medie_generala - 7.74).abs() < EPSILONE,
        "Studentii au fost salvati gresit!"
    );
    
    assert!(
        s.materii[3].id == 3
        && s.materii[3].nume == "Fizica".to_string()
        && s.materii[3].nume_titular == "Petrescu Cristina".to_string(),
        "Materiile au fost salvate gresit!"
    );

    assert!(
        s.inrolari[5].id_student == 2 && s.inrolari[5].id_materie == 0
        && (s.inrolari[5].note[0] - 2.22).abs() < EPSILONE
        && (s.inrolari[5].note[1] - 1.50).abs() < EPSILONE
        && (s.inrolari[5].note[2] - 3.08).abs() < EPSILONE,
        "Inrolarile au fost salvate gresit!"
    );
}


#[test]
fn test_adauga_student() {
    let mut s: Secretariat = citeste_secretariat(TEST_DB);
    
    adauga_student(&mut s, 11, "Popescu Adrian", 3, 'b', 9.99 as f32);

    assert!(
        s.studenti[10].nume == "Popescu Adrian".to_string()
        && s.studenti[10].id == 11
        && s.studenti[10].an_studiu == 3
        && s.studenti[10].statut == 'b',
        "Studentul nu a fost adaguat corect!"
    )

}