#[derive(Debug, Clone)]
pub struct Student {
    pub id: u32,
    pub nume: String,
    pub an_studiu: u8,
    pub statut: char,
    pub medie_generala: f32
}

#[derive(Debug, Clone)]
pub struct Materie {
    pub id: u32,
    pub nume: String,
    pub nume_titular: String
}

#[derive(Debug, Clone)]
pub struct Inrolari {
    pub id_student: u32,
    pub id_materie: u32,
    pub note: [f32; 3]
}

#[derive(Debug, Default)]
pub struct Secretariat {
    pub studenti: Vec<Student>,
    pub materii: Vec<Materie>,
    pub inrolari: Vec<Inrolari>,
}