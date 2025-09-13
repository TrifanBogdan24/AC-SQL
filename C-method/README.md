# AC SQL

În cadrul acestui proiect am implementat o bază de date pentru gestionarea informațiilor
despre studenți, materii și relațiile dintre acestea dintr-o facultate.

Baza de date este administrată printr-un sistem de interogare și actualizare denumit **AC SQL**,
care include criptarea memoriei cu un algoritm simplificat de tip **CBC**.



## Structuri de Date folosite

Baza de date **secretariat** conține:
- vector de studenți + numărul total
- vector de materii + numărul total
- vector de înrolări (relații student-materie) + numărul total

```c
struct secretariat {
    student *studenti;     // Vector de studenti
    int nr_studenti;       // Numarul total de studenti

    materie *materii;      // Vector de materii
    int nr_materii;        // Numarul total de materii

    inrolare *inrolari;    // Vector de inscrieri (relatii student-materie)
    int nr_inrolari;       // Numarul total de inscrieri
};
```

### Intrare din tabela **studenti**
Câmpuri: ID unic, nume complet, an de studiu, statut (`buget` / `taxă`), medie generală.

```c
struct student {
    int id;                              // ID unic al studentului
    char nume[MAX_STUDENT_NAME];         // Nume complet
    int an_studiu;                       // Anul de studiu
    char statut;                         // 'b' (buget) sau 't' (taxă)
    float medie_generala;                // Media generală
}__attribute__((packed));
```

### Intrare din tabela **materii**
Câmpuri: ID unic, nume materie, nume titular.

```c
struct materie {
    int id;                // ID unic al materiei
    char *nume;            // Nume materie
    char *nume_titular;    // Profesor titular
};
```

### Intrare din tabela **inrolari**
Câmpuri: ID student, ID materie, note (laborator, parțial, final).

```c
struct inrolare {
    int id_student;                       // ID-ul studentului
    int id_materie;                       // ID-ul materiei
    float note[NUMBER_OF_GRADES];         // Notele studentului (laborator, parțial, final)
};
```

## ⛃ Task 1: crearea/ștergerea bazei de date

Funcția `citeste_secretariat` încarcă baza de date în memorie dintr-un fișier text ce combină formatele **.toml** și **CSV**:
- numele tabelelor sunt scrise între paranteze drepte (`[STUDENTI]`, `[MATERII]`, `[INROLARI]`)
- valorile din fiecare intrare sunt separate prin `,`

Pentru delimitarea secțiunilor am folosit un `enum Table`.
Atunci când citesc o linie `[ ... ]`, actualizez un **flag** de tip `Table`.
Astfel știu în ce tabel să inserez datele următoare.

Parsarea liniilor se face cu funcții standard pe șiruri: `strtok`, `snprintf`/`strncpy`, `strcmp`.

La început, vectorii pentru studenți/materii/înrolari au dimensiune 0.
Fiecare linie parsată este adăugată la final prin `realloc`.

Media generală a unui student se calculează ca
suma notelor obținute la materiile unde este înrolat, împărțită la numărul lor.

```c
void calculeaza_medii_generale(secretariat *s);
```

**Ștergerea bazei de date**: presupune eliberarea vectorilor și a câmpurilor alocate dinamic (`nume`, `nume_titular`), urmată de `free` pe structura principală.

---

## `>_` Task 2: interpretor de comenzi SQL

După încărcarea bazei de date, comenzile SQL sunt citite de la tastatură (**stdin**).

Am implementat un parser capabil să proceseze:
- `SELECT`: interogări
- `UPDATE`: modificări condiționate
- `DELETE`: ștergeri de intrări

Utilizatorul introduce un număr `n`, apoi cele `n` query-uri.

### Filtrare cu WHERE

Clauza `WHERE` este opțională. Condițiile sunt salvate într-o structură dedicată (`camp`, operator de comparație, valoare).

Pentru potrivire am implementat funcții de tip "pattern matching" pentru fiecare tabel, plus variante care verifică toate condițiile dintr-un array.

```c
typedef struct {
    char *camp;
    char *op_comp;  // Operatorul de comperatie
    char *valoare;
} conditie;

int match_student_on_conditie(student student, conditie cond);
int match_materie_on_conditie(materie materie, conditie cond);
int match_inrolare_on_conditie(inrolare inrolare, conditie cond);

int match_student_on_all_conditii(student student, int nr_conditii, conditie *conditii);
int match_materie_on_all_conditii(materie materie, int nr_conditii, conditie *conditii);
int match_inrolare_on_all_conditii(inrolare inrolare, int nr_conditii, conditie *conditii);
```  

### SELECT
- Suportă `*` (toate câmpurile) sau coloane specifice (cu un **flag** pentru globbing).
- Poate fi combinat cu `WHERE`.

### UPDATE
- Iterează toate intrările unui tabel.
- Dacă intrarea respectă condițiile, se face **pattern matching** pe numele câmpului din `SET` și valoarea se actualizează.

### DELETE
- Intrările care respectă condițiile sunt șterse:
  - memoria intrării e eliberată
  - vectorul este **shiftat la stânga**
  - numărul de elemente se decrementează
  - vectorul e redimensionat cu `realloc` sau eliberat complet dacă devine gol

Ștergerile din `studenti` și `materii` afectează automat și tabela `inrolari`.

```c
typedef enum {
    ID_STUDENT,
    ID_MATERIE
} id_type;


void DELETE_FROM_inrolari_by_id(secretariat *secretariat, id_type type, int id);
```


## 🔐 Task 3: criptarea bazei de date
- Am folosit `unsigned char` pentru a reprezenta valori hexadecimale.
- **Serializarea** vectorului de studenți în octeți:
  - dimensiunea unui student se calculează adunând dimensiunile câmpurilor (nu cu `sizeof(struct student)`)
  - pentru fiecare student, copiez câmpurile în buffer cu `memcpy`,
    iar poziția curentă se actualizează prin aritmetică pe pointeri (incrementare cu dimensiunea câmpului serializat)
- Funcțiile `XOR` și `P_BOX` modifică datele in-place.
- `P_BOX` copiază într-un buffer auxiliar, aplică permutarea și rescrie vectorul original.


## Practici de Coding Style
- Am folosit funcții sigure (`snprintf`, `strncpy`) în locul lui `strcpy`.
- Gestionarea memoriei dinamice a fost făcută atent (alocare și eliberare la timp).
- Acoladele nu se deschid pe o linie nouă.



## 💡 Idei finale

Provocările principale pe care le-am întâmpinat:
- **Ștergerea sigură a unui element din vectori dinamici**:
  Pe tabela **materii**, care conține două câmpuri alocate dinamic,
  a fost necesar să folosesc `memmove` pentru a **shifta la stânga** memoria vectorului, evitând memory leak-uri.

- Transformarea vectorului de studenți într-o secvență de octeți a fost mai complicată decât părea:
    nu am putut să folosesc direct `sizeof(struct student)`,
    ci am calculat dimensiunea fiecărui câmp în bytes.

- **Precizia numerelor zecimale**
  Am învățat că pentru calculele de medii,
  `double` oferă precizie mult mai bună decât `float`.

- Operațiile pe sirurile de caractere în C
