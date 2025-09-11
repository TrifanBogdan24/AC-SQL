use crate::Student;
use crate::Materie;
use crate::Inrolare;

use std::str::FromStr;

#[derive(Debug)]
pub struct Conditie {
    pub camp: String,
    pub op_comp: String,
    pub valoare: String
}

// Trait general
pub trait Matchable {
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


pub fn match_on_all_conditii<T: Matchable>(item: &T, conditii: &[Conditie]) -> Result<bool, String> {
    for cond in conditii {
        match item.matches_condition(cond) {
            Ok(true) => continue,
            Ok(false) => return Ok(false),
            Err(message) => return Err(message),
        }
    }
    Ok(true)
}


pub fn parseaza_conditiile_where(str_conditii: &str) -> Result<Vec<Conditie>, String> {
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

        let camp = tokens.next().ok_or(format!("Camp lipsa in conditie: {:?}", cond_str))?.trim();
        let op_comp = tokens.next().ok_or(format!("Operator lipsa in conditie: {:?}", cond_str))?.trim();
        let valoare = tokens.next().ok_or(format!("Valoare lipsa in conditie: {:?}", cond_str))?.trim();

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
    let valoare: T;
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
