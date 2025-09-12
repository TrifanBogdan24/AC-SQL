use crate::structuri::*;


const NR_BLOCKS: usize = 4;

fn xor(block: &[u8], key: &[u8]) -> Vec<u8> {
    block
        .iter()
        .zip(key.iter().cycle()) // repeta cheia daca e mai scurta
        .map(|(&b, &k)| b ^ k)
        .collect()

}

fn p_box(block: &[u8]) -> Vec<u8> {
    let block_len: usize = block.len();
    let mut array: Vec<u8> = vec![0; block_len];


    for i in 0..block.len() {
        let j: usize =
            (i * (block_len - 1) + 2) % block_len;
        array[j] = block[i];
    }

    array
}


fn split_into_blocks(secretariat: &Secretariat) -> Vec<Vec<u8>> {
    let mut bytes_studenti: Vec<u8> = Vec::new();

    for student in secretariat.studenti.iter() {
        // id
        bytes_studenti.extend_from_slice(&student.id.to_be_bytes());

        // an_studiu
        bytes_studenti.extend_from_slice(&student.an_studiu.to_be_bytes());

        // statut (char -> u32 -> bytes)
        bytes_studenti.extend_from_slice(&(student.statut as u32).to_be_bytes());

        // medie_generala
        bytes_studenti.extend_from_slice(&student.medie_generala.to_be_bytes());

        // nume (trebuie serializat separat ca string)
        bytes_studenti.extend_from_slice(student.nume.as_bytes());
    }

    // Adaugare padding (daca este cazul):
    while bytes_studenti.len() % 4 != 0 {
        bytes_studenti.push(0 as u8);
    }


    // Impartire in blocuri de aceasi dimensiune
    let mut blocks: Vec<Vec<u8>> = Vec::new();
    let block_length: usize = bytes_studenti.len() / NR_BLOCKS;
    for i in 0..NR_BLOCKS {
        let start = i * block_length;
        let end = start + block_length;
        blocks.push(bytes_studenti[start..end].to_vec());
    }

    blocks
}


pub fn cripteaza_studenti(secretariat: &Secretariat, key: &str, iv: &str, _cale_output: &str) {
    let mut blocks: Vec<Vec<u8>> = split_into_blocks(secretariat);

    let bytes_key: Vec<u8> = key.as_bytes().to_vec();
    let bytes_iv: Vec<u8> = iv.as_bytes().to_vec();

    blocks[0] = xor(&blocks[0], &bytes_iv);
    blocks[0] = xor(&blocks[0], &bytes_key);
    blocks[0] = p_box(&blocks[0]);

    for i in 1..NR_BLOCKS {
        blocks[i] = xor(&blocks[i], &blocks[i - 1]);
        blocks[i] = xor(&blocks[i], &bytes_key);
        blocks[i] = p_box(&blocks[i]);
    }
}