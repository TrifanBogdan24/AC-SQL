# AC SQL

Am implementat o bază de date pentru gestionarea studenților, materiilor și
înrolărilor dintr-o facultate.

Baza de date este administrată printr-un interpretor SQL minimal, numit
**AC SQL**, care include criptare în memorie printr-o variantă simplificată de
**CBC**.

## Structuri de Date

Baza de date **secretariat** conține:
- vector de studenți
- vector de materii
- vector de înrolări (relații student-materie)

```rs
#[derive(Debug, Default)]
pub struct Secretariat {
    pub studenti: Vec<Student>,
    pub materii: Vec<Materie>,
    pub inrolari: Vec<Inrolare>,
}
```

### Intrare din tabela **studenti**

```rs
#[derive(Debug, Clone, Default)]
pub struct Student {
    pub id: usize,
    pub nume: String,
    pub an_studiu: u8,
    pub statut: char,        // 'b' (buget) sau 't' (taxă)
    pub medie_generala: f32
}
```

### Intrare din tabela **materii**

```rs
#[derive(Debug, Clone, Default)]
pub struct Materie {
    pub id: usize,
    pub nume: String,
    pub nume_titular: String
}
```

### Intrare din tabela **inrolari**

```rs
#[derive(Debug, Clone, Default)]
pub struct Inrolare {
    pub id_student: usize,
    pub id_materie: usize,
    pub note: [f32; 3]       // Notele studentului (laborator, parțial, final)
}
```

## ⛃ Task 1: crearea bazei de date

Funcția `citeste_secretariat` încarcă baza de date dintr-un fișier text
care combină **.toml** și **CSV**:
- numele tabelelor sunt între paranteze drepte (`[STUDENTI]`, etc.)
- valorile sunt separate prin `,`

Pentru delimitare am folosit un `enum TableType`.
O linie `[ ... ]` setează un flag, indicând în ce tabel inserez datele.

Media generală a unui student = suma notelor împărțită la numărul de materii.

Pentru erori de citire am folosit `unwrap()`, care oprește execuția la
date invalide.

## `>_` Task 2: interpretor SQL

Task-ul 2 a fost partea cea mai voluminoasă.
Am început să modularizez proiectul pentru claritate.

Rust nu este **OOP**, dar oferă **polimorfism** prin generice și traits.
Astfel, în loc să scriu câte o funcție `DELETE_FROM_` pentru fiecare tabel
(ca în C), am folosit o singură funcție generică.

### Filtrare cu WHERE

Am definit structura `Conditie` cu câmp, operator și valoare.
```rs
#[derive(Debug)]
pub struct Conditie {
    pub camp: String,
    pub op_comp: String,
    pub valoare: String
}
```

Pentru verificare, am implementat un **trait** `Matchable`, cu metode care
fac pattern matching pe câmp și operator.

Funcția `match_on_all_conditii` verifică o intrare pe rând; dacă o condiție
nu se potrivește, întoarce `false`.

### SELECT

Dacă se folosește `*`, am setat un flag care selectează toate câmpurile.

### UPDATE

Iterează toate intrările.
Dacă respectă condițiile, actualizează valoarea prin pattern matching pe
câmpul din `SET`.

### DELETE

Intrările care respectă condițiile sunt șterse cu `.remove()` din `Vec`.
Ștergerile din `studenti` și `materii` actualizează automat tabela
`inrolari`.


```rs
enum IdType {
    IdStudent,
    IdMaterie,
}


fn delete_from_inrolari_by_id(s: &mut Secretariat, id_type: IdType, id: usize);
```

## 🔐 Task 3: criptare

- Am folosit `u8` pentru valori hexadecimale.
- **Serializare**: în loc de `memcpy` și pointeri (C), în Rust am iterat
studenții și am adăugat câmpurile prin `.to_be_bytes()`.
- Funcțiile `XOR` și `P_BOX` întorc un nou vector de octeți.
- Am folosit `file.flush()` pentru a forța scrierea pe disc.

## Redenumirea lui **main.rs**

La `cargo init` se creează implicit `main.rs`.
Pentru a reflecta structura variantei în C (`task1.c`, `task2.c`, etc.),
am schimbat intrarea principală în `Cargo.toml`.

```rs
[[bin]]
path = "src/task2.rs"
```

## Modularizare

Fiecare fișier `.rs` este un modul.
Modulele sunt declarate în fișierul principal și importate cu `use crate::...`.

```rs
pub mod structuri;
pub mod task1;
pub mod task3;

pub mod queries {
    pub mod where_clause;
    pub mod select;
    pub mod delete;
    pub mod update;
}

mod tests {
    pub mod test_task1;
    pub mod test_task3;
}
```

## ✅ Teste Unitare

Am învățat terminologia **table-based testing**.
Am scris o singură funcție de test care rulează automat pe mai multe cazuri,
comparând fișiere de ieșire cu fișiere de referință.

## 💡 Idei finale

Am învățat cum se propagă erorile în Rust:
- Operatorul `?`: întoarce valoarea din `Ok`, sau propagă eroarea mai departe.
- Funcția `.unwrap()`: extrage valoarea din `Ok`, dar dă `panic!` la `Err`.

Am folosit funcții care întorc `Result<(), String>` pentru propagarea
erorilor.

De asemenea, am înțeles rolul lui `T` din funcțiile generice: marchează un
tip de date abstract, eliminând cod duplicat.
