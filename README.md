# 🚀 AC SQL

Un mic experiment de SQL făcut de la zero, în C.  
Am construit o bază de date pentru o facultate (studenți, materii, înscrieri) și un interpretor care înțelege comenzi,
de exemplu `SELECT`, `UPDATE` și `DELETE`.  
Pe deasupra, am adăugat și criptare cu un algoritm simplificat de tip **CBC** (Cipher Block Chaining), ca să protejez datele.

Practic, am vrut să văd cât de greu e să faci un „mini–Postgres” în C.

Răspunsul meu: **greu**, dar extrem de fun.

> Citește și [README-technical.md](./README-technical.md) pentru a înțelege implementarea proiectului.

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

2. **Interpretor SQL**  
   - Comenzile se citesc de la tastatură (`stdin`).  
   - Sunt implementate:  
     - `SELECT` – afișare, cu suport pentru `WHERE`  
     - `UPDATE` – modificare condiționată  
     - `DELETE` – ștergere (inclusiv relațiile aferente din alte tabele)  

   Am scris un parser simplu care sparge query-urile și le aplică pe datele din memorie.
   Aici am simțit cel mai mult ce înseamnă să scrii un interpretor „de mână”.

3. **Criptare**  
   - Vectorul de studenți poate fi criptat.  
   - Am folosit un CBC simplificat: `XOR`, permutări (`P-Box`) și un `IV`.  
   - Totul se face byte cu byte, cu `unsigned char`.


## 💡 Ce am învățat

- Gestionarea memoriei dinamice în C și evitarea **memory leak**-urilor  
- Cum să parsezi text și să construiești un interpretor minimal  
- Cât de complicat (și interesant) e să implementezi corect ceva ce pare banal în SQL  
- De ce merită să folosești funcții sigure (`snprintf`, `strncpy`) și să eviți clasicul `strcpy`


## 🔎 De ce mi se pare interesant proiectul

Am plecat de la niște structuri C și am ajuns cu un mini–SQL engine care:  
- știe să ruleze interogări,  
- respectă relații între tabele,  
- și poate chiar să cripteze datele.  

Un fel de *bază de date în miniatură*, dar care m-a făcut să înțeleg mai bine
ce se află în spatele unui **SGBD** real.
