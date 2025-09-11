use crate::Secretariat;


use crate::queries::where_clause::*;

/*
TODO: daca stergerea se fac din tabelele Studenti/Materii,
      sterge intrarile asociate din Inrolari
 */

fn delete_from_table<T: Matchable>(
    entries: &mut Vec<T>,
    conditii: &Vec<Conditie>
) -> Result<(), String>
{
    let mut idx: usize = 0;
    while idx < entries.len() {
        match match_on_all_conditii(&entries[idx], conditii)? {
            true => {
                entries.remove(idx);
            }
            false =>
                idx += 1
        }

    }

    Ok(())
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
        "studenti" => delete_from_table(&mut s.studenti, &conditii),
        "materii" => delete_from_table(&mut s.materii, &conditii),
        "inrolari" => delete_from_table(&mut s.inrolari, &conditii),
        _ => Err(format!(
            "Tabela {:?} nu exista in baza de date a facultati!",
            nume_tabela
        ))
    }
}
