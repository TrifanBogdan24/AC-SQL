#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task1.h"

const char *SEP = ",;";
const char *SPACE_CHARS = " \r\t";

void DIE(int condition, char *error_message)
{
    if (!condition)
        return;
    fprintf(stderr, "%s", error_message);
    exit(EXIT_FAILURE);
}

void trim(char* str)
{
    while (str && strchr(SPACE_CHARS, str[0]))
        str++;
    while (str && strchr(SPACE_CHARS, str[strlen(str) - 1]))
        str[strlen(str) - 1] = '\0';
}

void print_student(student student)
{
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

void print_materie(materie materie)
{
    printf("[MATERIE]\n");
    printf("Nume materie: \"%s\"\n", materie.nume);
    printf("ID: %d\n", materie.id);
    printf("Nume titular de curs: %s\n", materie.nume_titular);
    printf("\n");
}

void print_inrolare(inrolare inrolare)
{
    printf("[INROLARE]\n");
    printf("ID student: %d\n", inrolare.id_student);
    printf("ID materie: %d\n", inrolare.id_materie);
    printf("Note:\n");
    printf("\t- Laborator: %.2f\n", inrolare.note[0]);
    printf("\t- (Examen) Partial: %.2f\n", inrolare.note[1]);
    printf("\t- (Examen) Final: %.2f\n", inrolare.note[2]);
    printf("\n");
}

student parseaza_intrare_student(char *linie)
{
    student student;
    student.medie_generala = 0.0;

    char *token = strtok(linie, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    trim(token);
    student.id = atoi(token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    trim(token);
    strcpy(student.nume, token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    trim((token));
    student.an_studiu = atoi(token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    trim(token);
    student.statut = token[0];

    return student;
}


materie parseaza_intrare_materie(char *linie)
{
    materie materie;

    char *token = strtok(linie, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    trim(token);
    materie.id = atoi(token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    materie.nume = (char *) malloc(150 * sizeof(char));
    trim(token);
    strcpy(materie.nume, token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    materie.nume_titular = (char *) malloc(150 * sizeof(char));
    trim(token);
    strcpy(materie.nume_titular, token);

    return materie;
}



inrolare parseaza_intrare_inrolare(char *linie)
{
    inrolare inrolare;

    char *token = strtok(linie, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    trim(token);
    inrolare.id_student = atoi(token);

    token = strtok(NULL, SEP);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    trim(token);
    inrolare.id_materie = atoi(token);

    token = strtok(NULL, SPACE_CHARS);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    inrolare.note[0] = atof(token);    // Laborator

    token = strtok(NULL, SPACE_CHARS);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    inrolare.note[1] = atof(token);    // (Examen) Partial

    token = strtok(NULL, SPACE_CHARS);
    DIE(token == NULL, "Fisierul .db nu respecta formatul dorit!");
    inrolare.note[2] = atof(token);    // (Examen) Final

    return inrolare;
}


void adauga_student(secretariat *s, int id, char *nume, int an_studiu, char statut, float medie_generala) {
    // TODO(student): 1.2
    s->nr_studenti += 1;

    s->studenti = realloc(s->studenti, s->nr_studenti * sizeof(student));

    student student;
    student.id = id;
    strcpy(student.nume, nume);
    student.an_studiu = an_studiu;
    student.statut = statut;
    student.medie_generala = medie_generala;

    s->studenti[s->nr_studenti - 1] = student;
}

void adauga_materie(secretariat *s, materie materie)
{
    s->nr_materii += 1;
    s->materii = realloc(s->materii, s->nr_materii * sizeof(materie));
    s->materii[s->nr_materii - 1] = materie;
}

void adauga_inrolare(secretariat *s, inrolare inrolare)
{
    s->nr_inrolari += 1;
    s->inrolari = realloc(s->inrolari, s->nr_inrolari * sizeof(inrolare));
    s->inrolari[s->nr_inrolari - 1] = inrolare;
}


typedef enum {
    TABELA_STUDENTI,
    TABELA_MATERII,
    TABELA_INROLARI,
    NONE
} Table;

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
    char linie[100];

    Table table = NONE;


    while (!feof(fin))
    {
        fgets(linie, 100, fin);
        linie[strlen(linie) - 1] = '\0';

        if (!strlen(linie)) {
            // Empty linie
            continue;
        }

        if (!strcmp(linie, "[STUDENTI]\0")) {
            // Incepe citirea din tabela "STUDENTI"
            table = TABELA_STUDENTI;
        } else if (!strcmp(linie, "[MATERII]\0")) {
            // Incepe citirea din tabela "MATERII"
            table = TABELA_MATERII;
        } else if (!strcmp(linie, "[INROLARI]\0")) {
            // Incepe citirea din tabela "INROLARI"
            table = TABELA_INROLARI;
        } else {
            switch (table)
            {
            case TABELA_STUDENTI:
                student student = parseaza_intrare_student(linie);
                /* Pentru debugging: print_student(student); */
                adauga_student(
                    s,
                    student.id, student.nume, student.an_studiu,
                    student.statut, student.medie_generala);
                break;
            
            case TABELA_MATERII:
                materie materie = parseaza_intrare_materie(linie);
                /* Pentru debugging: print_materie(materie); */
                adauga_materie(s, materie);
                break;
            
            case TABELA_INROLARI:
                inrolare inrolare = parseaza_intrare_inrolare(linie);
                /* Pentru debugging: print_inrolare(inrolare); */
                adauga_inrolare(s, inrolare);
                break;
            
            default:
                break;
            }
        }
    }

    fclose(fin);
    return s;
}



void elibereaza_secretariat(secretariat **s)
{
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
