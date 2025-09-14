# ⛃ AC SQL

Un mic experiment de SQL făcut de la zero, în C și Rust.
Am construit o bază de date pentru o facultate (studenți, materii, înscrieri) și un interpretor care înțelege comenzi,
de exemplu `SELECT`, `UPDATE` și `DELETE`.
Pe deasupra, am adăugat și criptare cu un algoritm simplificat de tip **CBC** (Cipher Block Chaining), ca să protejez datele.

Practic, am vrut să văd cât de greu e să faci un "mini–Postgres" în C și Rust.

Răspunsul meu: **greu**, dar extrem de fun.

## Implementări multiple

| Lang. | Directory |
| :---: | :---: |
| C     | [`C-method/`](./C-method/) |
| Rust  | [`rust-method/`](./rust-method/) |

## 🗂 Structura bazei de date

Trei tabele principale:
- **studenti**: informații despre fiecare student (nume, an, medie, statut)
- **materii**: denumire + profesor titular
- **inrolari**: legăturile dintre studenți și materii, cu notele aferente

Datele sunt ținute în structuri C și gestionate dinamic (cu `malloc`, `realloc`, `free`).


## ⚙️ Cum funcționează

1. **Citirea bazei de date**
   - Pornesc de la un fișier text (o combinație de TOML + CSV).
   - Îl parsez linie cu linie și încarc structurile în memorie.
   - Folosesc un **flag** pentru a ști din ce tabel fac parte liniile citite.

2. **Interpretor SQL**
   - Comenzile se citesc de la tastatură (`stdin`).
   - Sunt implementate:
     - `SELECT`: afișare, cu suport pentru `WHERE`
     - `UPDATE`: modificare condiționată
     - `DELETE`: ștergere (inclusiv relațiile aferente din alte tabele)

   Am scris un parser simplu care sparge query-urile și le aplică pe datele din memorie.
   Aici am simțit cel mai mult ce înseamnă să scrii un interpretor "de mână".

3. **Criptare**
   - Vectorul de studenți poate fi criptat.
   - Am folosit un **CBC** simplificat: `XOR`, permutări (`P-Box`) și un `IV`.
   - Totul se face byte cu byte: cu `unsigned char` în C, respectiv `u8` în Rust.

## 🔐 Criptarea bazei de date

Pentru a proteja datele, vectorul de studenți este criptat cu o variantă simplificată de **CBC (Cipher Block Chaining)**.

Pașii algoritmului **CBC**:
1. Transform vectorul de studenți într-o succesiune de octeți
2. Împart vectorul în 4 blocuri de lungimi egale (ultimul bloc completat cu **padding** de `0x00` dacă e nevoie)
3. Criptez primul bloc:
   - 3.1. `XOR` între `block[0]` și `iv` (initialization vector)
   - 3.2. S-Box (`XOR`) cu cheia de criptare
   - 3.3. P-Box (permutare) pe `block[0]`
4. Criptez celelalte blocuri:
   - 4.1. `XOR` între `block[i]` și `block[i-1]`
   - 4.2. S-Box (`XOR`) cu cheia
   - 4.3. P-Box (permutare)
6. Blocurile rezultate sunt scrise byte cu byte în fișierul de ieșire


## 💡 Ce am învățat


**Idei generale**:
- Cum să parsezi text și să construiești un interpretor minimal
- Cât de complicat (și interesant) e să implementezi corect ceva ce pare banal în SQL

**Implementarea în C**:
- Gestionarea memoriei dinamice în C și evitarea **memory leak**-urilor
- De ce merită să folosești funcții sigure (`snprintf`, `strncpy`) și să eviți clasicul `strcpy`

**Implementarea în Rust**:
- Cum să structurez un proiect Rust **modular**, pe foldere și fișiere
- Propagarea erorilor într-un mod elegant folosind **pattern matching**, operatorul `?` și `.unwrap()`
- Scrierea de **teste unitare table-based**, reutilizând aceeași funcție generică alături de framework-ul [rstest](https://crates.io/crates/rstest)
- Folosirea **funcțiilor generice** pentru a elimina cod duplicat prin **polimorfism**

## 🔎 De ce mi se pare interesant proiectul

Am plecat de la niște structuri C/Rust și am ajuns cu un mini–SQL engine care:
- știe să ruleze interogări,
- respectă relații între tabele,
- și poate chiar să cripteze datele.

Un fel de *bază de date în miniatură*, dar care m-a făcut să înțeleg mai bine
ce se află în spatele unui **SGBD** real.
