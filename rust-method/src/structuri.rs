#[derive(Debug, Clone, Default)]
pub struct Student {
    pub id: usize,
    pub nume: String,
    pub an_studiu: u8,
    pub statut: char,
    pub medie_generala: f32
}

#[derive(Debug, Clone, Default)]
pub struct Materie {
    pub id: usize,
    pub nume: String,
    pub nume_titular: String
}

#[derive(Debug, Clone, Default)]
pub struct Inrolare {
    pub id_student: usize,
    pub id_materie: usize,
    pub note: [f32; 3]
}

#[derive(Debug, Default)]
pub struct Secretariat {
    pub studenti: Vec<Student>,
    pub materii: Vec<Materie>,
    pub inrolari: Vec<Inrolare>,
}