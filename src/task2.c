#include "task2.h"
#include "trim.h"

/* Intoarce 0 daca nu sunt probleme si -1 in cazul in care exista campuri invalida */
int imparte_campuri_SELECT(char *toate_campurile, int *nr_campuri, char ***campuri)
{
    char *token = strtok(toate_campurile, ",");

    while (token) {
        (*nr_campuri)++;
        (*campuri) = (char **) realloc(*campuri, *nr_campuri * sizeof(char*));
        (*campuri)[*nr_campuri - 1] = malloc(150 * sizeof(char));
        strcpy((*campuri)[*nr_campuri - 1], token);
        trim((*campuri)[*nr_campuri - 1]);
        token = strtok(NULL, ",");
    }

    return 0;
}


/*
 * Daca tabela nu exista in baza de date, atunci intoarce -1 si scrie un text de eroare
 * altfel intoarce 0
*/
int is_valid_table(char *nume_tabela)
{
    if (strcmp(nume_tabela, "studenti")
        && strcmp(nume_tabela, "materii")
        && strcmp(nume_tabela, "inrolari")) {
        fprintf(stderr, "[EROARE] Tabela \"%s\" nu exista in baza de date a secretariatului!\n",
            nume_tabela);
        return -1;
    }

    return 0;
}

int is_valid_field(char *nume_tabela, char *camp)
{
    for (size_t i = 0; i < strlen(camp); i++) {
        if (!isspace(camp[i]))
            continue;
        fprintf(stderr, "[EROARE] Camp invalid \"%s\"\n", camp);
        printf("[INFO] Campurile NU pot fi separate prin spatii!\n");
        return -1;
    }

    int err_flag = 0;
    if (!strcmp(nume_tabela, "studenti")
        && strcmp(camp, "id") && strcmp(camp, "nume")  && strcmp(camp, "an_studiu")
        && strcmp(camp, "status") && strcmp(camp, "medie_generala")) {
        err_flag = -1;
    }

    if (!strcmp(nume_tabela, "materii")
        && strcmp(camp, "id") && strcmp(camp, "nume") && strcmp(camp, "nume_titular")) {
        err_flag = -1;
    }

    if (!strcmp(nume_tabela, "inrolari")
        && strcmp(camp, "id_student") && strcmp(camp, "id_materie") && strcmp(camp, "note")) {
        err_flag = -1;
    }


    if (err_flag) {
        fprintf(stderr, "[EROARE] Tabela \"%s\" nu contine campul \"%s\"\n", nume_tabela, camp);
        return -1;
    }

    return 0;
}




/* Template:
SELECT <campuri> FROM <tabel> WHERE <camp> <operator> <valoare>;
SELECT <campuri> FROM <tabel>;
*/
void SELECT(secretariat *secretariat, char *interogare)
{
    char *ptr_SELECT = strstr(interogare, "SELECT ");
    char *ptr_FROM   = strstr(interogare, " FROM ");
    char *ptr_WHERE  = strstr(interogare, " WHERE ");

    if (!ptr_SELECT) {
        fprintf(stderr, "[EROARE] Interogarea nu contine clauza 'SELECT'\n");
        return;
    }

    if (!ptr_FROM) {
        fprintf(stderr, "[EROARE] Interogarea nu contine clauza 'FROM'\n");
        return;
    }

    int idx_SELECT = ptr_SELECT - interogare;
    int idx_FROM = ptr_FROM - interogare;

    char *nume_tabela = (char*) malloc(100 * sizeof(char));
    char *toate_campurile = (char*) malloc(100 * sizeof(char));
    
    int buffer_len = 0;
    int ret_val = 0;

    buffer_len = idx_FROM - idx_SELECT - strlen("SELECT ");
    strncpy(
        toate_campurile,
        interogare +  idx_SELECT + strlen("SELECT "),
        buffer_len);
    toate_campurile[buffer_len] = '\0';
    trim(toate_campurile);

    // Are valoarea '1' in caz de "SELECT *" si '0' atunci cand "SELECT <campuri>"
    int is_select_all =
        (strlen(toate_campurile) == 1 && toate_campurile[0] == '*');

    int nr_campuri = 0;
    char **campuri = NULL;

    if (!is_select_all) {
        // NU se foloseste globbing-ul "*"
        if (imparte_campuri_SELECT(toate_campurile, &nr_campuri, &campuri))
            return;  // Campuri invalide
    }

    if (ptr_WHERE) {
        /* SELECT ... FROM <tabel> WHERE ...; */
        int idx_WHERE = ptr_WHERE - interogare;
        buffer_len = idx_WHERE - idx_FROM - strlen(" FROM ");
        strncpy(
            nume_tabela,
            interogare + idx_FROM + strlen(" FROM "),
            buffer_len);
        nume_tabela[buffer_len] = '\0';
    } else {
        /* SELECT ... FROM <tabel>; */
        buffer_len = strlen(interogare) - strlen(";") - idx_FROM - strlen(" FROM ");
        strncpy(
            nume_tabela,
            interogare + idx_FROM + strlen(" FROM "),
            buffer_len);
        nume_tabela[buffer_len] = '\0';
    }


    trim(nume_tabela);

    if (is_valid_table(nume_tabela)) {
        return;
    }



    if (is_select_all) {
        // Se foloseste globbing-ul "*"

        if (!strcmp(nume_tabela, "studenti")) {
            for (int i = 0; i < secretariat->nr_studenti; i++) {
                student student = secretariat->studenti[i];
                printf("%d %s %d %c %.1f\n",
                    student.id, student.nume, student.an_studiu,
                    student.statut, student.medie_generala);
            }
        } else if (!strcmp(nume_tabela, "materii")) {
            for (int i = 0; i < secretariat->nr_materii; i++) {
                materie materie = secretariat->materii[i];
                printf("%d %s %s\n",
                    materie.id, materie.nume, materie.nume_titular);
            }
        } else if (!strcmp(nume_tabela, "inrolari")) {
            for (int i = 0; i < secretariat->nr_inrolari; i++) {
                inrolare inrolare = secretariat->inrolari[i];
                printf("%d %d %.1f %.1f %.1f\n",
                    inrolare.id_student, inrolare.id_materie,
                    inrolare.note[0], inrolare.note[1], inrolare.note[2]);
            }
        }
    } else {
        // Se interogheaza dupa nume de coloana:
        
        ret_val = 0;
        for (int i = 0; i < nr_campuri; i++) {
            if (is_valid_field(nume_tabela, campuri[i]))
                ret_val = -1;  // Campuri invalide
        }

        if (ret_val)
            return;   // Campuri invalide

        if (!strcmp(nume_tabela, "studenti")) {
            for (int i = 0; i < secretariat->nr_studenti; i++) {
                student student = secretariat->studenti[i];

                for (int j = 0; j < nr_campuri; j++) {
                    if (!strcmp(campuri[j], "id"))
                        printf("%d", student.id);
                    if (!strcmp(campuri[j], "nume"))
                        printf("%s", student.nume);
                    if (!strcmp(campuri[j], "an_studiu"))
                        printf("%d", student.an_studiu);
                    if (!strcmp(campuri[j], "statut"))
                        printf("%c", student.statut);
                    if (!strcmp(campuri[j], "medie_generala"))
                        printf("%.1f", student.medie_generala);

                    if (j < nr_campuri - 1)
                        printf(" ");
                }
                printf("\n");
            }
        } else if (!strcmp(nume_tabela, "materii")) {
            for (int i = 0; i < secretariat->nr_materii; i++) {
                materie materie = secretariat->materii[i];

                for (int j = 0; j < nr_campuri; j++) {
                    if (!strcmp(campuri[j], "id"))
                        printf("%d", materie.id);
                    if (!strcmp(campuri[j], "nume"))
                        printf("%s", materie.nume);
                    if (!strcmp(campuri[j], "nume_titular"))
                        printf("%s", materie.nume_titular);
                    
                    if (j < nr_campuri - 1)
                        printf(" ");
                }
                printf("\n");

            }
        } else if (!strcmp(nume_tabela, "inrolari")) {
            for (int i = 0; i < secretariat->nr_inrolari; i++) {
                inrolare inrolare = secretariat->inrolari[i];

                for (int j = 0; j < nr_campuri; j++) {
                    if (!strcmp(campuri[j], "id_student"))
                        printf("%d", inrolare.id_student);
                    if (!strcmp(campuri[j], "id_materie"))
                        printf("%d", inrolare.id_materie);
                    if (!strcmp(campuri[j], "note"))
                        printf("%1.f %1.f %1.f",
                            inrolare.note[0], inrolare.note[1], inrolare.note[2]);

                    if (j < nr_campuri - 1)
                        printf(" ");
                }
                printf("\n");
            }
        }
    }

}

void proceseaza_interogare(secretariat *secretariat, char *interogare)
{
    trim(interogare);

    if (!strlen(interogare)) {
        fprintf(stderr, "[EROARE] Interogare vida!\n");
        return;
    }

    if (interogare[strlen(interogare) - 1] != ';') {
        fprintf(stderr, "[EROARE] Lipseste ';' la final de interogare!\n");
        return;
    }

    if (strstr(interogare, "SELECT ")) {
        SELECT(secretariat, interogare);
    } else if (strstr(interogare, "UPDATE ")) {

    } else if (strstr(interogare, "DELETE ")) {

    } else {
        fprintf(stderr, "[EROARE] Interogare invalida!\n");
    }
}

int main(int argc, char *argv[]) {
    // TODO(student): Task 2

    if (argc != 2) {
        fprintf(stderr, "[EROARE] Programul se asteapta sa fie apelat in felul urmator:\n");
        fprintf(stderr, "         ./tema1 <file.db>\n");
        return EXIT_FAILURE;
    }

    secretariat *secretariat = citeste_secretariat(argv[1]);

    int nr_interogari = 0;

    scanf("%d", &nr_interogari);
    getc(stdin);   // Citeste new-line '\n'

    char *linie = (char *) malloc(250 * sizeof(char));

    for (int i = 0; i < nr_interogari; i++) {
        fgets(linie, 250, stdin);

        // Inlocuieste new-line ('\n') cu NULL terminator ('\0'):
        linie[strlen(linie) - 1] = '\0';

        proceseaza_interogare(secretariat, linie);
    }

    elibereaza_secretariat(&secretariat);
    return 0;
}
