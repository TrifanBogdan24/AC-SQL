use crate::Secretariat;
use crate::Student;
use crate::Materie;
use crate::Inrolare;

use crate::queries::where_clause::*;


enum IdType {
    IdStudent,
    IdMaterie,
}


fn delete_from_inrolari_by_id(s: &mut Secretariat, id_type: IdType, id: usize) {
    let mut idx = 0;

    while idx < s.inrolari.len() {
        let remove: bool = match id_type {
            IdType::IdStudent => s.inrolari[idx].id_student == id,
            IdType::IdMaterie => s.inrolari[idx].id_materie == id,
        };

        if remove {
            s.inrolari.remove(idx);
        } else {
            idx += 1;
        }
    }
}

/// Trait pentru elementele unei tabele
trait TableEntry: Matchable {
    /// returneaza ID-urile care trebuie sterse din Inrolari
    fn related_ids_to_delete(&self) -> Vec<(IdType, usize)>;
}


impl TableEntry for Student {
    fn related_ids_to_delete(&self) -> Vec<(IdType, usize)> {
        vec![(IdType::IdStudent, self.id)]
    }
}

impl TableEntry for Materie {
    fn related_ids_to_delete(&self) -> Vec<(IdType, usize)> {
        vec![(IdType::IdMaterie, self.id)]
    }
}

impl TableEntry for Inrolare {
    fn related_ids_to_delete(&self) -> Vec<(IdType, usize)> {
        Vec::new() // default: nu sterge nimic
    }
}



/// Functie generica de stergere dintr-o tabela
fn delete_from_table<T: TableEntry>(
    entries: &mut Vec<T>,
    conditii: &Vec<Conditie>,
) -> Result<Vec<(IdType, usize)>, String> {
    let mut idx = 0;
    let mut related_ids = Vec::new();

    while idx < entries.len() {
        if match_on_all_conditii(&entries[idx], conditii)? {
            related_ids.extend(entries[idx].related_ids_to_delete());
            entries.remove(idx);
        } else {
            idx += 1;
        }
    }

    Ok(related_ids)
}



/* Template
DELETE FROM <tabel> WHERE <conditie>;
DELETE FROM <tabel> WHERE <conditie1> AND <conditie2>;
*/
pub fn delete(s: &mut Secretariat, query: &str) -> Result<(), String> {
    // Gaseste "DELETE" urmat de cel putin un spatiu si "FROM"
    let delete_from_pos = query.find("DELETE")
        .ok_or_else(|| "DELETE nu a fost gasit".to_string())?;

    // Avanseaza peste "DELETE"
    let mut ptr = &query[delete_from_pos + "DELETE".len()..];

    // Trebuie sa existe cel putin un spatiu
    if !ptr.starts_with(char::is_whitespace) {
        return Err("trebuie cel putin un spatiu intre DELETE si FROM".to_string());
    }

    // Sare peste toate spatiile
    ptr = ptr.trim_start();

    // Verifica ca urmeaza "FROM"
    if !ptr.starts_with("FROM") {
        return Err("DELETE trebuie urmat de FROM".to_string());
    }

    // Avanseaza peste "FROM"
    ptr = &ptr["FROM".len()..];
    ptr = ptr.trim_start();

    // Gaseste WHERE daca exista
    let where_pos = ptr.find("WHERE");

    // Extrage numele tabelei
    let nume_tabela = if let Some(pos) = where_pos {
        ptr[..pos].trim()
    } else {
        // pana la ';' sau sfarsitul stringului
        if let Some(end) = ptr.find(';') {
            ptr[..end].trim()
        } else {
            ptr.trim()
        }
    };

    // Extrage conditiile daca exista
    let str_conditii = if let Some(pos) = where_pos {
        let mut cond_ptr = &ptr[pos + "WHERE".len()..];
        cond_ptr = cond_ptr.trim_start();
        if let Some(end) = cond_ptr.find(';') {
            cond_ptr[..end].trim()
        } else {
            cond_ptr.trim()
        }
    } else {
        ""
    };


    // Aici poti implementa logica de stergere din Secretariat
    let conditii: Vec<Conditie> = parseaza_conditiile_where(str_conditii)?;
    
    match nume_tabela {
        "studenti" => {
            let related_ids = delete_from_table(&mut s.studenti, &conditii)?;
            for (id_type, id) in related_ids {
                delete_from_inrolari_by_id(s, id_type, id);
            }
            Ok(())
        }
        "materii" => {
            let related_ids = delete_from_table(&mut s.materii, &conditii)?;
            for (id_type, id) in related_ids {
                delete_from_inrolari_by_id(s, id_type, id);
            }
            Ok(())
        }
        "inrolari" => {
            delete_from_table(&mut s.inrolari, &conditii)?;
            Ok(())
        }
        _ => Err(format!(
            "Tabela {:?} nu exista in baza de date a facultati!",
            nume_tabela
        )),
    }
}
