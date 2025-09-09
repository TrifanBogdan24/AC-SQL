#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task1.h"
#include "trim.h"
#include "die.h"

#define BUFFER_LENGTH 256
#define SEP         ",;"
#define SPACE_CHARS " \r\t"


typedef enum {
    TABELA_STUDENTI,
    TABELA_MATERII,
    TABELA_INROLARI,
    NONE
} Table;


void print_student(student student) {
    printf("[STUDENT]\n");
    printf("Nume student: \"%s\"\n", student.nume);
    printf("ID: %d\n", student.id);
    printf("An studiu: %d\n", student.an_studiu);
    printf("Statut: %c (\"%s\")\n",
        student.statut,
        (student.statut == 'b') ? "buget" : "taxa");
    printf("Medie generala: %.2f\n", student.medie_generala);
    printf("\n");
}

void print_materie(materie materie) {
    printf("[MATERIE]\n");
    printf("Nume materie: \"%s\"\n", materie.nume);
    printf("ID: %d\n", materie.id);
    printf("Nume titular de curs: \"%s\"\n", materie.nume_titular);
    printf("\n");
}

void print_inrolare(inrolare inrolare) {
    printf("[INROLARE]\n");
    printf("ID student: %d\n", inrolare.id_student);
    printf("ID materie: %d\n", inrolare.id_materie);
    printf("Note:\n");
    printf("\t- Laborator: %.2f\n", inrolare.note[0]);
    printf("\t- (Examen) Partial: %.2f\n", inrolare.note[1]);
    printf("\t- (Examen) Final: %.2f\n", inrolare.note[2]);
    printf("\n");
}

student parseaza_intrare_student(char *linie) {
    student student;
    student.medie_generala = 0.0f;

    char *token = strtok(linie, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    trim(token);
    student.id = atoi(token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    trim(token);
    snprintf(student.nume, MAX_STUDENT_NAME, "%s", token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    trim(token);
    student.an_studiu = atoi(token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    trim(token);
    student.statut = token[0];

    return student;
}


materie parseaza_intrare_materie(char *linie) {
    materie materie;

    char *token = strtok(linie, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    trim(token);
    materie.id = atoi(token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    materie.nume = (char *) malloc(BUFFER_LENGTH * sizeof(char));
    trim(token);
    snprintf(materie.nume, MAX_STUDENT_NAME, "%s", token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    materie.nume_titular = (char *) malloc(BUFFER_LENGTH * sizeof(char));
    trim(token);
    snprintf(materie.nume_titular, BUFFER_LENGTH, "%s", token);

    return materie;
}



inrolare parseaza_intrare_inrolare(char *linie) {
    inrolare inrolare;

    char *token = strtok(linie, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    trim(token);
    inrolare.id_student = atoi(token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    trim(token);
    inrolare.id_materie = atoi(token);

    token = strtok(NULL, SPACE_CHARS);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    inrolare.note[0] = (float) atof(token);    // Laborator

    token = strtok(NULL, SPACE_CHARS);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    inrolare.note[1] = (float) atof(token);    // (Examen) Partial

    token = strtok(NULL, SPACE_CHARS);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!\n");
    inrolare.note[2] = (float) atof(token);    // (Examen) Final

    return inrolare;
}


void adauga_student(secretariat *s, int id, char *nume, int an_studiu, char statut, float medie_generala) {
    // TODO(student): 1.2
    s->nr_studenti += 1;

    s->studenti = realloc(s->studenti, s->nr_studenti * sizeof(student));
    DIE(s->studenti == NULL, "realloc: nu s-a putut redimensiona vectorul studentilor.\n");

    student student;
    student.id = id;
    snprintf(student.nume, MAX_STUDENT_NAME, "%s", nume);
    student.an_studiu = an_studiu;
    student.statut = statut;
    student.medie_generala = medie_generala;

    s->studenti[s->nr_studenti - 1] = student;
}

void adauga_materie(secretariat *s, materie materie) {
    s->nr_materii += 1;
    s->materii = realloc(s->materii, s->nr_materii * sizeof(materie));
    DIE(s->materii == NULL, "realloc: nu s-a putut redimensiona vectorul materiilor.\n");
    s->materii[s->nr_materii - 1] = materie;
}

void adauga_inrolare(secretariat *s, inrolare inrolare) {
    s->nr_inrolari += 1;
    s->inrolari = realloc(s->inrolari, s->nr_inrolari * sizeof(inrolare));
    DIE(s->inrolari == NULL, "realloc: nu s-a putut redimensiona vectorul inrolarilor.\n");
    s->inrolari[s->nr_inrolari - 1] = inrolare;
}


void calculeaza_medii_generale(secretariat *s) {
    int idx_inrolare = 0;

    for (int i = 0; i < s->nr_studenti; i++) {
        double suma_notelor = 0.0f;
        int nr_materii = 0;

        while (idx_inrolare < s->nr_inrolari
            && s->inrolari[idx_inrolare].id_student == s->studenti[i].id) {
            suma_notelor +=
                (double) s->inrolari[idx_inrolare].note[0]
                + (double) s->inrolari[idx_inrolare].note[1]
                + (double) s->inrolari[idx_inrolare].note[2];
            nr_materii++;
            idx_inrolare++;
        }

        if (nr_materii == 0)
            s->studenti[i].medie_generala = 0.0f;
        else
            s->studenti[i].medie_generala = (float)(suma_notelor / (double)nr_materii);
    }
}

secretariat *citeste_secretariat(const char *nume_fisier) {
    // TODO(student): 1.1
    secretariat *s = malloc(sizeof(secretariat));
    s->nr_studenti = 0;
    s->nr_materii = 0;
    s->nr_inrolari = 0;
    s->studenti = NULL;
    s->materii = NULL;
    s->inrolari = NULL;

    FILE *fin = fopen(nume_fisier, "r");
    char linie[BUFFER_LENGTH];

    Table table = NONE;


    while (!feof(fin)) {
        fgets(linie, BUFFER_LENGTH, fin);
        if (linie[strlen(linie) - 1] == '\n')
            linie[strlen(linie) - 1] = '\0';
        trim(linie);

        if (!strlen(linie)) {
            // Empty linie
            continue;
        }

        if (!strcmp(linie, "[STUDENTI]")) {
            // Incepe citirea din tabela "STUDENTI"
            table = TABELA_STUDENTI;
        } else if (!strcmp(linie, "[MATERII]")) {
            // Incepe citirea din tabela "MATERII"
            table = TABELA_MATERII;
        } else if (!strcmp(linie, "[INROLARI]")) {
            // Incepe citirea din tabela "INROLARI"
            table = TABELA_INROLARI;
        } else {
            /* Daca se foloseste match in loc de if-else, clang-tidy va afisa erori */

            if (table == TABELA_STUDENTI) {
                student student =
                    parseaza_intrare_student(linie);
                /* Pentru debugging: print_student(student); */
                adauga_student(
                    s,
                    student.id, student.nume, student.an_studiu,
                    student.statut, student.medie_generala);
            } else if (table == TABELA_MATERII) {
                materie materie = parseaza_intrare_materie(linie);
                /* Pentru debugging: print_materie(materie); */
                adauga_materie(s, materie);
            } else if (table == TABELA_INROLARI) {
                inrolare inrolare = parseaza_intrare_inrolare(linie);
                /* Pentru debugging: print_inrolare(inrolare); */
                adauga_inrolare(s, inrolare);
            }
        }
    }

    fclose(fin);
    calculeaza_medii_generale(s);

    return s;
}



void elibereaza_secretariat(secretariat **s) {
    // TODO(student): 1.3
    free((*s)->studenti);
    for (int i = 0; i < (*s)->nr_materii; i++) {
        free(((*s)->materii[i].nume));
        free(((*s)->materii[i].nume_titular));
    }
    free((*s)->materii);
    free((*s)->inrolari);
    free(*s);
    *s = NULL;
}
