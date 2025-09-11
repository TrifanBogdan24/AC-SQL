use crate::Secretariat;

use crate::queries::where_clause::*;
use crate::queries::where_clause::parseaza_conditiile_where;

trait Updateable {
    fn update_table_field(&self, field: &str, value: &str) -> Result<(), String>;
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
    let mut valoare = camp_valoare[eq_pos + 1..].trim().to_string();

    // Sterge ghilimele daca exista
    valoare = valoare.trim_matches('"').trim_matches('\'').to_string();

    // Extrage conditii (TOT ce este dupa WHERE)
    let mut str_conditii = ptr.trim().to_string();

    // Sterge ';' de la final
    if str_conditii.ends_with(';') {
        str_conditii.pop();
    }

    // Aici poti sa folosesti deja `nume_tabela`, `camp`, `valoare` si `str_conditii`
    // exemplu: parseaza conditii
    let conditii = parseaza_conditiile_where(&str_conditii)?;

    println!("Tabela: {}", nume_tabela);
    println!("Camp: {}", camp);
    println!("Valoare: {}", valoare);
    println!("Conditii: {:?}", conditii);

    Ok(())
}
