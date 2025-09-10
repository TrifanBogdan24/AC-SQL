include!("../include/structuri.rs");

mod task1;
mod task3;


use std::{env, process::exit};
use crate::task1::citeste_secretariat;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 2 {
        eprintln!("[EROARE] Numar invalid de argumente!");
        println!("[INFO] Programul se asteapta sa fie apelat in felul urmator:");
        println!("       cargo run <file.db>");
        exit(255);
    }

    let mut secretariat = citeste_secretariat(&args[1]);
}
