#include <math.h>
#include "task1.h"
#include "task2.h"
#include "task3.h"
#include "trim.h"
#include "die.h"

#define FIVE          5.0f
#define TEN           10.0f
#define ONE_HUNDRED   100.0f
#define BUFFER_LENGTH 256
#define NR_CAMPURI_TABELA_STUDENT   5
#define NR_CAMPURI_TABELA_MATERIE   3
#define NR_CAMPURI_TABELA_INROLARE  3

typedef struct {
    char *camp;
    char *op_comp;  // Operatorul de comperatie
    char *valoare;
} conditie;

typedef enum {
    ID_STUDENT,
    ID_MATERIE
} id_type;


/* Elibereaza memoria alocata vectorului */
void elibereaza_conditiile(int nr_conditii, conditie **conditii) {
    if (!conditii || !(*conditii)) return;

    for (int i = 0; i < nr_conditii; i++) {
        if ((*conditii)[i].camp)
            free((*conditii)[i].camp);
        if ((*conditii)[i].op_comp)
            free((*conditii)[i].op_comp);
        if ((*conditii)[i].valoare)
            free((*conditii)[i].valoare);
    }

    free(*conditii);
}

/* Elibereaza memoria alocata matricii (array de string-uri) */
void elibereaza_strings(int nr_strings, char ***strings) {
    if (!strings || !(*strings)) return;

    for (int i = 0; i < nr_strings; i++)
        if ((*strings)[i]) free((*strings)[i]);

    free(*strings);
}


/*
 * Elimina ghilimelele/apostrofurile de la inceputul si sfarsitul sirului
*/
void remove_trailing_quotation_marks(char *str) {
    if (!str) return;

    size_t len = strlen(str);
    if (len >= 2 &&
        ((str[0] == '\'' && str[len - 1] == '\'')
        ||(str[0] == '\"' && str[len - 1] == '\"'))) {
        memmove(str, str + 1, strlen(str));
        len--;
        str[len - 1] = '\0';
    }
}


/*
 * Daca tabela exista in baza de date, atunci intoarce 1 (TRUE)
 * Altfel, scrie un text de eroare si intoarce 0 (FALSE)
*/
int is_valid_table(const char *nume_tabela) {
    if (strcmp(nume_tabela, "studenti")
        && strcmp(nume_tabela, "materii")
        && strcmp(nume_tabela, "inrolari")) {
        fprintf(stderr, "[EROARE] Tabela \"%s\" nu exista in baza de date a secretariatului!\n",
            nume_tabela);
        return 0;
    }

    return 1;
}

/*
 * Daca tabela contine o coloana cu numele campului, atunci intoarce 1 (TRUE)
 * Altfel, return 0 (FALSE)
*/
int is_valid_field(char *nume_tabela, char *camp) {
    for (size_t i = 0; i < strlen(camp); i++) {
        if (!isspace(camp[i]))
            continue;
        fprintf(stderr, "[EROARE] Camp invalid \"%s\"\n", camp);
        printf("[INFO] Campurile NU pot fi separate prin spatii!\n");
        return 0;
    }

    int err_flag = 0;
    if (!strcmp(nume_tabela, "studenti")
        && strcmp(camp, "id") && strcmp(camp, "nume")  && strcmp(camp, "an_studiu")
        && strcmp(camp, "statut") && strcmp(camp, "medie_generala")) {
        err_flag = 1;
    }

    if (!strcmp(nume_tabela, "materii")
        && strcmp(camp, "id") && strcmp(camp, "nume") && strcmp(camp, "nume_titular")) {
        err_flag = 1;
    }

    if (!strcmp(nume_tabela, "inrolari")
        && strcmp(camp, "id_student") && strcmp(camp, "id_materie") && strcmp(camp, "note")) {
        err_flag = 1;
    }


    if (err_flag) {
        fprintf(stderr, "[EROARE] Tabela \"%s\" nu contine campul \"%s\"\n", nume_tabela, camp);
        return 0;
    }

    return 1;
}




/*
 Imparte conditiile in mai multe siruri.

 Exemplu:
 if (str_conditii == "id = 0") return ["id = 0"]
 if (str_conditii == "id = 0 AND nume = USO") return ["id = 0", "nume = USO"]

 ATENTIE! Functia nu se ocupa de eliminarea caracterului ';' de la finalul interogarii.
          Acel caracter trebuie sters inaine de a apela functia!
*/
char **split_conditii_into_strings(char *str_conditii, int *nr_conditii) {
    *nr_conditii = 0;
    char **conditii_strings = NULL;

    const char *delim = "AND";
    size_t len_delim = strlen(delim);
    char *ptr = str_conditii;
    char *found = strstr(ptr, delim);  // prima cautare

    while (found) {
        *found = '\0';

        (*nr_conditii)++;
        conditii_strings = realloc(conditii_strings, *nr_conditii * sizeof(char*));
        DIE(conditii_strings == NULL, "realloc: nu s-a putut redimensiona array-ul de string-uri pentru conditii\n");

        int idx = (*nr_conditii) - 1;
        conditii_strings[idx] = malloc(BUFFER_LENGTH * sizeof(char));
        snprintf(conditii_strings[idx], BUFFER_LENGTH, "%s", ptr);

        ptr = found + len_delim;
        found = strstr(ptr, delim);
    }

    // Ultima bucata (dupa ultimul "AND" sau intregul sir daca nu exista "AND")
    if (*ptr != '\0') {
        (*nr_conditii)++;
        conditii_strings = realloc(conditii_strings, *nr_conditii * sizeof(char*));
        DIE(conditii_strings == NULL, "realloc: nu s-a putut redimensiona array-ul de string-uri pentru conditii\n");
        int idx = (*nr_conditii) - 1;
        conditii_strings[idx] = malloc(BUFFER_LENGTH * sizeof(char));
        snprintf(conditii_strings[idx], BUFFER_LENGTH, "%s", ptr);
    }

    return conditii_strings;
}

/*
 Parseaza clauza WHERE si returneaza un array de conditii, plasate in struct-uri

 if (str_conditii == "id = 0") return [
    conditie(camp="id", op_comp="=", valoare="0")
    ]
 if (str_conditii == "id = 0 AND nume != USO") return [
    conditie(camp="id", op_comp="=", valoare="0"),
    conditie(camp="nume", op_comp="!=", valoare="USO"),
    ]

 ATENTIE! Functia nu se ocupa de eliminarea caracterului ';' de la finalul interogarii.
          Acel caracter trebuie sters inaine de a apela functia!
*/
conditie *parseaza_conditiile_WHERE(char *str_conditii, int *nr_conditii) {
    (*nr_conditii) = 0;
    char **conditii_strings = split_conditii_into_strings(str_conditii, nr_conditii);

    conditie *conditii = (conditie *) malloc((*nr_conditii) * sizeof(conditie));

    for (int i = 0; i < *nr_conditii; i++) {
        // <camp> <operator> <valoare>
        conditie *conditie = &conditii[i];
        conditie->camp = (char *) malloc(BUFFER_LENGTH * sizeof(char));
        conditie->op_comp = (char *) malloc(BUFFER_LENGTH * sizeof(char));
        conditie->valoare = (char *) malloc(BUFFER_LENGTH * sizeof(char));

        char *token = strtok(conditii_strings[i], " ");
        snprintf(conditie->camp, BUFFER_LENGTH, "%s", token);
        trim(conditie->camp);

        token = strtok(NULL, " ");
        snprintf(conditie->op_comp, BUFFER_LENGTH, "%s", token);
        trim(conditie->op_comp);

        token = strtok(NULL, "");
        snprintf(conditie->valoare, BUFFER_LENGTH, "%s", token);
        trim(conditie->valoare);
        remove_trailing_quotation_marks(conditie->valoare);
    }

    elibereaza_strings(*nr_conditii, &conditii_strings);
    return conditii;
}

/*
 * Returneaza 1 (TRUE) daca STUDENTUL respecta conditia,
 * Altfel, intoarce 0 (FALSE)
*/
int match_student_on_conditie(student student, conditie cond) {
    if (!strcmp(cond.camp, "id")) {
        int id = atoi(cond.valoare);

        if (!strcmp(cond.op_comp, "="))
            return student.id == id;
        if (!strcmp(cond.op_comp, "!="))
            return student.id != id;
        if (!strcmp(cond.op_comp, "<"))
            return student.id < id;
        if (!strcmp(cond.op_comp, ">"))
            return student.id > id;
        if (!strcmp(cond.op_comp, "<="))
            return student.id <= id;
        if (!strcmp(cond.op_comp, ">="))
            return student.id >= id;

        return 0;   // Conditie WHERE invalida
    } else if (!strcmp(cond.camp, "nume")) {
        char *nume = cond.valoare;

        if (!strcmp(cond.op_comp, "="))
            return strcmp(student.nume, nume) == 0;
        if (!strcmp(cond.op_comp, "!="))
            return strcmp(student.nume, nume) != 0;

        return 0;   // Conditie WHERE invalida
    } else if (!strcmp(cond.camp, "an_studiu")) {
        int an = atoi(cond.valoare);

        if (!strcmp(cond.op_comp, "="))
            return student.an_studiu == an;
        if (!strcmp(cond.op_comp, "!="))
            return student.an_studiu != an;
        if (!strcmp(cond.op_comp, "<"))
            return student.an_studiu < an;
        if (!strcmp(cond.op_comp, ">"))
            return student.an_studiu > an;
        if (!strcmp(cond.op_comp, "<="))
            return student.an_studiu <= an;
        if (!strcmp(cond.op_comp, ">="))
            return student.an_studiu >= an;

        return 0;   // Conditie WHERE invalida
    } else if (!strcmp(cond.camp, "statut")) {
        char statut = cond.valoare[0];

        if (!strcmp(cond.op_comp, "="))
            return student.statut == statut;
        if (!strcmp(cond.op_comp, "!="))
            return student.statut != statut;

        return 0;   // Conditie WHERE invalida
    } else if (!strcmp(cond.camp, "medie_generala")) {
        float medie_generala = (float) atof(cond.valoare);

        if (!strcmp(cond.op_comp, "="))
            return student.medie_generala == medie_generala;
        if (!strcmp(cond.op_comp, "!="))
            return student.medie_generala != medie_generala;
        if (!strcmp(cond.op_comp, "<"))
            return student.medie_generala < medie_generala;
        if (!strcmp(cond.op_comp, ">"))
            return student.medie_generala > medie_generala;
        if (!strcmp(cond.op_comp, "<="))
            return student.medie_generala <= medie_generala;
        if (!strcmp(cond.op_comp, ">="))
            return student.medie_generala >= medie_generala;

        return 0;   // Conditie WHERE invalida
    }

    return 0;   // Conditie WHERE invalida
}


/*
 * Returneaza 1 (TRUE) daca MATERIA respecta conditia,
 * Altfel, intoarce 0 (FALSE)
*/
int match_materie_on_conditie(materie materie, conditie cond) {
    if (!strcmp(cond.camp, "id")) {
        int id = atoi(cond.valoare);

        if (!strcmp(cond.op_comp, "="))
            return materie.id == id;
        if (!strcmp(cond.op_comp, "!="))
            return materie.id != id;
        if (!strcmp(cond.op_comp, "<"))
            return materie.id < id;
        if (!strcmp(cond.op_comp, ">"))
            return materie.id > id;
        if (!strcmp(cond.op_comp, "<="))
            return materie.id <= id;
        if (!strcmp(cond.op_comp, ">="))
            return materie.id >= id;

        return 0;   // Conditie WHERE invalida
    } else if (!strcmp(cond.camp, "nume")) {
        char *nume = cond.valoare;

        if (!strcmp(cond.op_comp, "="))
            return strcmp(materie.nume, nume) == 0;
        if (!strcmp(cond.op_comp, "!="))
            return strcmp(materie.nume, nume) != 0;

        return 0;   // Conditie WHERE invalida
    } else if (!strcmp(cond.camp, "nume_titular")) {
        char *nume_titular = cond.valoare;

        if (!strcmp(cond.op_comp, "="))
            return strcmp(materie.nume_titular, nume_titular) == 0;
        if (!strcmp(cond.op_comp, "!="))
            return strcmp(materie.nume_titular, nume_titular) != 0;

        return 0;   // Conditie WHERE invalida
    }


    return 0;   // Conditie WHERE invalida
}

/*
 * Returneaza 1 (TRUE) daca INROLAREA respecta conditia,
 * Altfel, intoarce 0 (FALSE)
*/
int match_inrolare_on_conditie(inrolare inrolare, conditie cond) {
    if (!strcmp(cond.camp, "id_student")) {
        int id_student = atoi(cond.valoare);

        if (!strcmp(cond.op_comp, "="))
            return inrolare.id_student == id_student;
        if (!strcmp(cond.op_comp, "!="))
            return inrolare.id_student != id_student;
        if (!strcmp(cond.op_comp, "<"))
            return inrolare.id_student < id_student;
        if (!strcmp(cond.op_comp, ">"))
            return inrolare.id_student > id_student;
        if (!strcmp(cond.op_comp, "<="))
            return inrolare.id_student <= id_student;
        if (!strcmp(cond.op_comp, ">="))
            return inrolare.id_student >= id_student;

        return 0;   // Conditie WHERE invalida
    } else if (!strcmp(cond.camp, "id_materie")) {
        int id_materie = atoi(cond.valoare);

        if (!strcmp(cond.op_comp, "="))
            return inrolare.id_materie == id_materie;
        if (!strcmp(cond.op_comp, "!="))
            return inrolare.id_materie != id_materie;
        if (!strcmp(cond.op_comp, "<"))
            return inrolare.id_materie < id_materie;
        if (!strcmp(cond.op_comp, ">"))
            return inrolare.id_materie > id_materie;
        if (!strcmp(cond.op_comp, "<="))
            return inrolare.id_materie <= id_materie;
        if (!strcmp(cond.op_comp, ">="))
            return inrolare.id_materie >= id_materie;

        return 0;   // Conditie WHERE invalida
    } else if (!strcmp(cond.camp, "note")) {
        char *token = strtok(cond.valoare, " ");
        float nota_laborator = (float) atof(token);

        token = strtok(NULL, " ");
        float nota_partial = (float) atof(token);

        token = strtok(NULL, " ");
        float nota_final = (float) atof(token);

        if (!strcmp(cond.op_comp, "="))
            return (nota_laborator == inrolare.note[0]
            && nota_partial == inrolare.note[1]
            && nota_final == inrolare.note[2]);
        if (!strcmp(cond.op_comp, "!="))
            return (nota_laborator != inrolare.note[0]
            || nota_partial != inrolare.note[1]
            || nota_final != inrolare.note[2]);

        return 0;   // Conditie WHERE invalida
    }

    return 0;   // Conditie WHERE invalida
}

/*
 * Returneaza 1 (TRUE) daca STUDENTUL respecta TOATE conditiile,
 * Altfel, intoarce 0 (FALSE)
*/
int match_student_on_all_conditii(student student, int nr_conditii, conditie *conditii) {
    for (int i = 0; i < nr_conditii; i++)
        if (!match_student_on_conditie(student, conditii[i]))
            return 0;
    return 1;
}


/*
 * Returneaza 1 (TRUE) daca MATERIA respecta TOATE conditiile,
 * Altfel, intoarce 0 (FALSE)
*/
int match_materie_on_all_conditii(materie materie, int nr_conditii, conditie *conditii) {
    for (int i = 0; i < nr_conditii; i++)
        if (!match_materie_on_conditie(materie, conditii[i]))
            return 0;
    return 1;
}


/*
 * Returneaza 1 (TRUE) daca INROLARE respecta TOATE conditiile,
 * Altfel, intoarce 0 (FALSE)
*/
int match_inrolare_on_all_conditii(inrolare inrolare, int nr_conditii, conditie *conditii) {
    for (int i = 0; i < nr_conditii; i++)
        if (!match_inrolare_on_conditie(inrolare, conditii[i]))
            return 0;
    return 1;
}



void SELECT_FROM_studenti(secretariat *secretariat,
    int nr_campuri, char **campuri,
    int nr_conditii, conditie *conditii) {
    for (int i = 0; i < secretariat->nr_studenti; i++) {
        student student = secretariat->studenti[i];

        if (nr_conditii > 0
            && !match_student_on_all_conditii(student, nr_conditii, conditii))
            continue;

        for (int j = 0; j < nr_campuri; j++) {
            if (!strcmp(campuri[j], "id")) {
                printf("%d", student.id);
            } else if (!strcmp(campuri[j], "nume")) {
                printf("%s", student.nume);
            } else if (!strcmp(campuri[j], "an_studiu")) {
                printf("%d", student.an_studiu);
            } else if (!strcmp(campuri[j], "statut")) {
                printf("%c", student.statut);
            } else if (!strcmp(campuri[j], "medie_generala")) {
                // Rotunjire la 2 zecimale:
                float temp = student.medie_generala * ONE_HUNDRED;
                int intPart = (int)temp;
                float thirdDecimal = (temp - (float)intPart) * TEN;

                if (thirdDecimal >= FIVE) {
                    // Rotunjire la sus la 2 zecimale
                    intPart += 1;
                }

                float result = (float)intPart / ONE_HUNDRED;
                printf("%.2f", result);
            }

            if (j < nr_campuri - 1)
                printf(" ");
        }
        printf("\n");
    }
}

void SELECT_FROM_materii(secretariat *secretariat,
    int nr_campuri, char **campuri,
    int nr_conditii, conditie *conditii) {
    for (int i = 0; i < secretariat->nr_materii; i++) {
        materie materie = secretariat->materii[i];

        if (nr_conditii > 0
            && !match_materie_on_all_conditii(materie, nr_conditii, conditii))
            continue;

        for (int j = 0; j < nr_campuri; j++) {
            if (!strcmp(campuri[j], "id"))
                printf("%d", materie.id);
            else if (!strcmp(campuri[j], "nume"))
                printf("%s", materie.nume);
            else if (!strcmp(campuri[j], "nume_titular"))
                printf("%s", materie.nume_titular);

            if (j < nr_campuri - 1)
                printf(" ");
        }
        printf("\n");
    }
}


void SELECT_FROM_inrolari(secretariat *secretariat,
    int nr_campuri, char **campuri,
    int nr_conditii, conditie *conditii) {
    for (int i = 0; i < secretariat->nr_inrolari; i++) {
        inrolare inrolare = secretariat->inrolari[i];

        if (nr_conditii > 0
            && !match_inrolare_on_all_conditii(inrolare, nr_conditii, conditii))
            continue;

        for (int j = 0; j < nr_campuri; j++) {
            if (!strcmp(campuri[j], "id_student"))
                printf("%d", inrolare.id_student);
            else if (!strcmp(campuri[j], "id_materie"))
                printf("%d", inrolare.id_materie);
            else if (!strcmp(campuri[j], "note"))
                printf("%.2f %.2f %.2f",
                    inrolare.note[0], inrolare.note[1], inrolare.note[2]);

            if (j < nr_campuri - 1)
                printf(" ");
        }
        printf("\n");
    }
}


char **allocate_campuri(int nr_campuri) {
    char **campuri = (char **) malloc(nr_campuri * sizeof(char *));
    for (int i = 0; i < nr_campuri; i++)
        campuri[i] = (char *) malloc(BUFFER_LENGTH * sizeof(char));
    return campuri;
}

char **build_campuri_STUDENTI(int *nr_campuri) {
    (*nr_campuri) = NR_CAMPURI_TABELA_STUDENT;

    char **campuri = allocate_campuri(*nr_campuri);
    snprintf(campuri[0], BUFFER_LENGTH, "%s", "id");
    snprintf(campuri[1], BUFFER_LENGTH, "%s", "nume");
    snprintf(campuri[2], BUFFER_LENGTH, "%s", "an_studiu");
    snprintf(campuri[3], BUFFER_LENGTH, "%s", "statut");
    snprintf(campuri[4], BUFFER_LENGTH, "%s", "medie_generala");
    return campuri;
}

char **build_campuri_METERIE(int *nr_campuri) {
    (*nr_campuri) = NR_CAMPURI_TABELA_MATERIE;

    char **campuri = allocate_campuri(*nr_campuri);
    snprintf(campuri[0], BUFFER_LENGTH, "%s", "id");
    snprintf(campuri[1], BUFFER_LENGTH, "%s", "nume");
    snprintf(campuri[2], BUFFER_LENGTH, "%s", "nume_titular");
    return campuri;
}


char **build_campuri_INROLARE(int *nr_campuri) {
    (*nr_campuri) = NR_CAMPURI_TABELA_INROLARE;

    char **campuri = allocate_campuri(*nr_campuri);
    snprintf(campuri[0], BUFFER_LENGTH, "id_student");
    snprintf(campuri[1], BUFFER_LENGTH, "id_materie");
    snprintf(campuri[2], BUFFER_LENGTH, "note");
    return campuri;
}


char **build_campuri(char *nume_tabela, int *nr_campuri) {
    if (!strcmp(nume_tabela, "studenti")) {
        return build_campuri_STUDENTI(nr_campuri);
    } else if (!strcmp(nume_tabela, "materii")) {
        return build_campuri_METERIE(nr_campuri);
    } else if (!strcmp(nume_tabela, "inrolari")) {
        return build_campuri_INROLARE(nr_campuri);
    }

    fprintf(stderr, "[EROARE] Nume invalid de tabela \"%s\"", nume_tabela);
    return NULL;
}


/*
 Parseaza campurile din clauza SELECT si intoarce un array de string-uri
 
 Exemplu:
 if (str_conditii == "nume") return ["nume"]
 if (str_conditii == "nume, medie_generala") return ["nume", "medie_generala"]

 ATENTIE! Functia nu poate parsa caracterul "*" de globbing
*/
char **parseaza_campurile_SELECT(char *str_conditii, int *nr_campuri) {
    *nr_campuri = 0;
    char **campuri = NULL;

    char *token = strtok(str_conditii, ",");

    while (token) {
        (*nr_campuri)++;
        int idx = *nr_campuri - 1;

        campuri = realloc(campuri, (*nr_campuri) * sizeof(char *));
        DIE(campuri == NULL, "realloc: nu s-a putu redimensiona vectorul campurilor selectate\n");

        campuri[idx] = (char *) malloc((strlen(token) + 1) * sizeof(char));
        snprintf(campuri[idx], BUFFER_LENGTH, "%s", token);
        trim(campuri[idx]);

        token = strtok(NULL, ",");
    }

    return campuri;
}


/* Template:
SELECT * FROM <tabel>;
SELECT <campuri> FROM <tabel>;
SELECT <campuri> FROM <tabel> WHERE <camp> <operator> <valoare>;
SELECT <campuri> FROM <tabel> WHERE <cond1> AND <cond2>;

Campuri - unul sau mai multe nume de coloane, despartite prin virgula ','
*/
void SELECT(secretariat *secretariat, char *interogare) {
    char *ptr = strstr(interogare, "SELECT");
    if (!ptr) return;
    ptr += strlen("SELECT");

    // Extrage <campuri>
    while (isspace((unsigned char)*ptr)) ptr++;
    char *fromPos = strstr(ptr, "FROM");
    if (!fromPos) return;

    char *str_campuri = (char*) malloc(BUFFER_LENGTH * sizeof(char));
    strncpy(str_campuri, ptr, fromPos - ptr);
    str_campuri[fromPos - ptr] = '\0';
    trim(str_campuri);

    int nr_campuri = 0;
    char **campuri = NULL;

    // Flag pentru "*"
    int is_select_all =
        (strcmp(str_campuri, "*") == 0);

    // Extrage <nume_tabela>
    ptr = fromPos + strlen("FROM");
    while (isspace((unsigned char)*ptr)) ptr++;
    char *wherePos = strstr(ptr, "WHERE");

    char *nume_tabela = (char*) malloc(BUFFER_LENGTH * sizeof(char));
    if (wherePos) {
        strncpy(nume_tabela, ptr, wherePos - ptr);
        nume_tabela[wherePos - ptr] = '\0';
    } else {
        // pana la ';'
        char *end = strchr(ptr, ';');
        if (!end) end = ptr + strlen(ptr);
        strncpy(nume_tabela, ptr, end - ptr);
        nume_tabela[end - ptr] = '\0';
    }
    trim(nume_tabela);

    // Extrage <conditii> (daca exista clauza WHERE)
    char *str_conditii = (char *) malloc(BUFFER_LENGTH * sizeof(char));
    int nr_conditii = 0;
    conditie *conditii = NULL;

    if (wherePos) {
        ptr = wherePos + strlen("WHERE");
        while (isspace((unsigned char)*ptr)) ptr++;
        char *end = strchr(ptr, ';');
        if (!end) end = ptr + strlen(ptr);
        strncpy(str_conditii, ptr, end - ptr);
        str_conditii[end - ptr] = '\0';
        trim(str_conditii);
        conditii = parseaza_conditiile_WHERE(str_conditii, &nr_conditii);
    }



    if (is_select_all) {
        // Se foloseste globbing-ul "*"
        campuri = build_campuri(nume_tabela, &nr_campuri);
    } else {
        campuri = parseaza_campurile_SELECT(str_campuri, &nr_campuri);
    }

    if (!is_valid_table(nume_tabela)) {
        return;
    }


    int ret_val = 0;
    for (int i = 0; i < nr_campuri; i++) {
        if (!is_valid_field(nume_tabela, campuri[i]))
            ret_val = -1;  // Campuri invalide
    }

    if (ret_val)
        return;   // Campuri invalide


    if (!strcmp(nume_tabela, "studenti")) {
        SELECT_FROM_studenti(
            secretariat,
            nr_campuri, campuri,
            nr_conditii, conditii);
    } else if (!strcmp(nume_tabela, "materii")) {
        SELECT_FROM_materii(
            secretariat,
            nr_campuri, campuri,
            nr_conditii, conditii);
    } else if (!strcmp(nume_tabela, "inrolari")) {
        SELECT_FROM_inrolari(
            secretariat,
            nr_campuri, campuri,
            nr_conditii, conditii);
    }


    free(str_campuri);
    free(str_conditii);
    free(nume_tabela);

    elibereaza_strings(nr_campuri, &campuri);
    elibereaza_conditiile(nr_conditii, &conditii);
}


void UPDATE_studenti(secretariat *secretariat,
    char *camp, char *valoare,
    int nr_conditii, conditie *conditii) {
    for (int i = 0; i < secretariat->nr_studenti; i++) {
        student *student = &secretariat->studenti[i];

        if (nr_conditii > 0
            && !match_student_on_all_conditii(*student, nr_conditii, conditii))
            continue;

        if (!strcmp(camp, "id")) {
            student->id = atoi(valoare);
        } else if (!strcmp(camp, "nume")) {
            snprintf(student->nume, BUFFER_LENGTH, "%s", valoare);
        } else if (!strcmp(camp, "statut")) {
            if (!strlen(valoare)) return;   // Eroare
            student->statut = valoare[0];
        } else if (!strcmp(camp, "medie_generala")) {
            student->medie_generala = (float) atof(valoare);
        }
    }
}

void UPDATE_materii(secretariat *secretariat,
    char *camp, char *valoare,
    int nr_conditii, conditie *conditii) {
    for (int i = 0; i < secretariat->nr_materii; i++) {
        materie *materie = &secretariat->materii[i];

        if (nr_conditii > 0
            && !match_materie_on_all_conditii(*materie, nr_conditii, conditii))
            continue;

        if (!strcmp(camp, "id")) {
            materie->id = atoi(valoare);
        } else if (!strcmp(camp, "nume")) {
            snprintf(materie->nume, MAX_STUDENT_NAME, "%s", valoare);
        } else if (!strcmp(camp, "nume_titular")) {
            snprintf(materie->nume_titular, BUFFER_LENGTH, "%s", valoare);
        }
    }
}

void UPDATE_inrolari(secretariat *secretariat,
    char *camp, char *valoare,
    int nr_conditii, conditie *conditii) {
    float nota_laborator = 0.0f;
    float nota_examen_partial = 0.0f;
    float nota_examen_final = 0.0f;


    if (!strcmp(camp, "note")) {
        char *token = strtok(valoare, " ");
        nota_laborator = (float) atof(token);

        token = strtok(NULL, " ");
        nota_examen_partial = (float) atof(token);

        token = strtok(NULL, " ");
        nota_examen_final = (float) atof(token);
    }

    for (int i = 0; i < secretariat->nr_inrolari; i++) {
        inrolare *inrolare = &secretariat->inrolari[i];

        if (nr_conditii > 0
            && !match_inrolare_on_all_conditii(*inrolare, nr_conditii, conditii))
            continue;

        if (!strcmp(camp, "id_student")) {
            inrolare->id_student = atoi(valoare);
        } else if (!strcmp(camp, "id_materie")) {
            inrolare->id_materie = atoi(valoare);
        } else if (!strcmp(camp, "note")) {
            inrolare->note[0] = nota_laborator;
            inrolare->note[1] = nota_examen_partial;
            inrolare->note[2] = nota_examen_final;


            // Updateaza mediile:
            calculeaza_medii_generale(secretariat);
        }
    }
}

/* Template:
UPDATE <tabel> SET <camp> = <valoare> WHERE <conditie>;
UPDATE <tabel> SET <camp> = <valoare> WHERE <cond1> AND <cond2>;
*/
void UPDATE(secretariat *secretariat, char *interogare) {
    char *ptr = strstr(interogare, "UPDATE");
    if (!ptr)
        return;

    ptr += strlen("UPDATE");

    // Exrage <tabel>
    while (isspace((unsigned char)*ptr)) ptr++;
    char *endTable = strstr(ptr, "SET");
    if (!endTable) return;

    char *nume_tabela = (char *) malloc(BUFFER_LENGTH * sizeof(char));
    strncpy(nume_tabela, ptr, endTable - ptr);
    nume_tabela[endTable - ptr] = '\0';
    trim(nume_tabela);

    // Extrage partea "<camp> = <valaore>"
    ptr = endTable + strlen("SET");
    while (isspace((unsigned char)*ptr)) ptr++;
    char *endWhere = strstr(ptr, "WHERE");
    if (!endWhere) return;

    char *campValoare = (char *) malloc(BUFFER_LENGTH * sizeof(char));
    strncpy(campValoare, ptr, endWhere - ptr);
    campValoare[endWhere - ptr] = '\0';
    trim(campValoare);

    // Imparte <camp> de <valoare> folosind separatorul '='
    char *eq = strchr(campValoare, '=');
    if (!eq) return;

    char *camp = (char *) malloc(BUFFER_LENGTH * sizeof(char));
    char *valoare = (char *) malloc(BUFFER_LENGTH * sizeof(char));
    strncpy(camp, campValoare, eq - campValoare);
    camp[eq - campValoare] = '\0';
    snprintf(valoare, BUFFER_LENGTH, "%s", eq + 1);

    trim(camp);
    trim(valoare);
    remove_trailing_quotation_marks(valoare);


    // Extrage conditie/conditii (TOT ce este dupa WHERE)
    ptr = endWhere + strlen("WHERE");
    while (isspace((unsigned char)*ptr)) ptr++;
    char *str_conditii = (char*) malloc(BUFFER_LENGTH * sizeof(char));
    strncpy(str_conditii, ptr, strlen(ptr));
    str_conditii[strlen(ptr)] = '\0';
    trim(str_conditii);

    // Sterge ';' de la final
    size_t len = strlen(str_conditii);
    if (len > 0 && str_conditii[len - 1] == ';')
        str_conditii[len - 1] = '\0';

    int nr_conditii = 0;
    conditie *conditii = parseaza_conditiile_WHERE(str_conditii, &nr_conditii);

    if (!strcmp(nume_tabela, "studenti")) {
        UPDATE_studenti(
            secretariat,
            camp, valoare,
            nr_conditii, conditii);
    } else if (!strcmp(nume_tabela, "materii")) {
        UPDATE_materii(
            secretariat,
            camp, valoare,
            nr_conditii, conditii);
    } else if (!strcmp(nume_tabela, "inrolari")) {
        UPDATE_inrolari(
            secretariat,
            camp, valoare,
            nr_conditii, conditii);
    }

    free(nume_tabela);
    free(campValoare);
    free(camp);
    free(valoare);
    free(str_conditii);

    elibereaza_conditiile(nr_conditii, &conditii);
}




void DELETE_FROM_inrolari(
    secretariat *secretariat,
    int nr_conditii, conditie *conditii) {
    int idx = 0;


    while (idx < secretariat->nr_inrolari) {
        inrolare inrolare = secretariat->inrolari[idx];
        if (!match_inrolare_on_all_conditii(inrolare, nr_conditii, conditii)) {
            idx++;
            continue;
        }


        // Altfel, sterg inrolarea cu indicele 'idx' din vector:
        for (int i = idx; i < secretariat->nr_inrolari - 1; i++)
            secretariat->inrolari[i] = secretariat->inrolari[i + 1];

        secretariat->nr_inrolari -= 1;
        if (secretariat->nr_inrolari == 0) {
            // Vectorul nu mai contine niciun element, ii eliberez memoria:
            free(secretariat->inrolari);
            secretariat->inrolari = NULL;
        } else {
            // Redimensionez vectorul:
            secretariat->inrolari = realloc(secretariat->inrolari, secretariat->nr_inrolari * sizeof(inrolare));
            DIE(secretariat->nr_inrolari > 0 && secretariat->inrolari == NULL,
                "realloc: nu s-a putut redimensiona vectorul inrolarilor.\n");
        }
    }

    // Updateaza mediile:
    calculeaza_medii_generale(secretariat);
}


void DELETE_FROM_inrolari_by_id(secretariat *secretariat, id_type type, int id) {
    conditie conditie;

    conditie.camp = (char *) malloc(BUFFER_LENGTH * sizeof(char));
    conditie.op_comp = (char *) malloc(BUFFER_LENGTH * sizeof(char));
    conditie.valoare = (char *) malloc(BUFFER_LENGTH * sizeof(char));

    /* DELETE FROM inrolari WHERE id_student/materie = id  */
    if (type == ID_STUDENT)
        snprintf(conditie.camp, BUFFER_LENGTH, "%s", "id_student");
    else if (type == ID_MATERIE)
        snprintf(conditie.camp, BUFFER_LENGTH, "%s", "id_materie");

    snprintf(conditie.op_comp, BUFFER_LENGTH, "%s", "=");
    snprintf(conditie.valoare, BUFFER_LENGTH, "%d", id);
    DELETE_FROM_inrolari(secretariat, 1, &conditie);

    free(conditie.camp);
    free(conditie.op_comp);
    free(conditie.valoare);
}

void DELETE_FROM_studenti(
    secretariat *secretariat,
    int nr_conditii, conditie *conditii) {
    int idx = 0;

    while (idx < secretariat->nr_studenti) {
        student student = secretariat->studenti[idx];
        if (!match_student_on_all_conditii(student, nr_conditii, conditii)) {
            idx++;
            continue;
        }

        /* DELETE FROM inrolari WHERE id_student = student[idx].id  */
        DELETE_FROM_inrolari_by_id(secretariat, ID_STUDENT, secretariat->studenti[idx].id);


        // Sterg studentul cu indicele 'idx' din vector:
        for (int i = idx; i < secretariat->nr_studenti - 1; i++)
            secretariat->studenti[i] = secretariat->studenti[i + 1];

        secretariat->nr_studenti -= 1;
        if (secretariat->nr_studenti == 0) {
            // Vectorul nu mai contine niciun element, ii eliberez memoria:
            free(secretariat->studenti);
            secretariat->studenti = NULL;
        } else {
            // Redimensionez vectorul:
            secretariat->studenti = realloc(secretariat->studenti, secretariat->nr_studenti * sizeof(student));
            DIE(secretariat->nr_studenti > 0 && secretariat->studenti == NULL,
                "realloc: nu s-a putut redimensiona vectorul studentilor.\n");
        }
    }
}


void DELETE_FROM_materii(
    secretariat *secretariat,
    int nr_conditii, conditie *conditii) {
    int idx = 0;

    while (idx < secretariat->nr_materii) {
        materie materie = secretariat->materii[idx];
        if (!match_materie_on_all_conditii(materie, nr_conditii, conditii)) {
            idx++;
            continue;
        }

        /* DELETE FROM inrolari WHERE id_materie = materii[idx].id  */
        DELETE_FROM_inrolari_by_id(secretariat, ID_MATERIE, secretariat->materii[idx].id);

        // Sterg materia cu indicele 'idx' din vector:
        free(secretariat->materii[idx].nume);
        free(secretariat->materii[idx].nume_titular);
        // Sterg materia cu indicele 'idx', shiftand la stanga vectorul in memorie:
        memmove(&secretariat->materii[idx],
            &secretariat->materii[idx + 1],
            (secretariat->nr_materii - idx - 1) * sizeof(materie));



        secretariat->nr_materii--;
        if (secretariat->nr_materii == 0) {
            // Vectorul nu mai contine niciun element, ii eliberez memoria:
            free(secretariat->materii);
            secretariat->materii = NULL;
        } else {
            // Redimensionez vectorul:
            secretariat->materii = realloc(secretariat->materii, secretariat->nr_materii * sizeof(materie));
            DIE(secretariat->nr_materii > 0 && secretariat->materii == NULL,
                "realloc: nu s-a putut redimensiona vectorul materiilor.\n");
        }
    }
}





/* Template
DELETE FROM <tabel> WHERE <conditie>;
DELETE FROM <tabel> WHERE <conditie1> AND <conditie2>;
*/
void DELETE(secretariat *secretariat, char *interogare) {
    char *ptr = strstr(interogare, "DELETE FROM");
    if (!ptr) return;
    ptr += strlen("DELETE FROM");

    // Extrage numele tabelului
    while (isspace((unsigned char)*ptr)) ptr++;
    char *wherePos = strstr(ptr, "WHERE");

    char *nume_tabela = (char*) malloc(BUFFER_LENGTH * sizeof(char));
    if (wherePos) {
        strncpy(nume_tabela, ptr, wherePos - ptr);
        nume_tabela[wherePos - ptr] = '\0';
    } else {
        // pana la ';'
        char *end = strchr(ptr, ';');
        if (!end) end = ptr + strlen(ptr);
        strncpy(nume_tabela, ptr, end - ptr);
        nume_tabela[end - ptr] = '\0';
    }
    trim(nume_tabela);

    // Extrage conditiile (daca exista)
    char *str_conditii = (char*) malloc(BUFFER_LENGTH * sizeof(char));
    if (wherePos) {
        ptr = wherePos + strlen("WHERE");
        while (isspace((unsigned char)*ptr)) ptr++;
        char *end = strchr(ptr, ';');
        if (!end) end = ptr + strlen(ptr);
        strncpy(str_conditii, ptr, end - ptr);
        str_conditii[end - ptr] = '\0';
        trim(str_conditii);
    }

    int nr_conditii = 0;
    conditie *conditii = parseaza_conditiile_WHERE(str_conditii, &nr_conditii);

    if (!strcmp(nume_tabela, "studenti")) {
        DELETE_FROM_studenti(
            secretariat,
            nr_conditii, conditii);
    } else if (!strcmp(nume_tabela, "materii")) {
        DELETE_FROM_materii(
            secretariat,
            nr_conditii, conditii);
    } else if (!strcmp(nume_tabela, "inrolari")) {
        DELETE_FROM_inrolari(
            secretariat,
            nr_conditii, conditii);
    }

    free(nume_tabela);
    free(str_conditii);
    elibereaza_conditiile(nr_conditii, &conditii);
}



void proceseaza_interogare(secretariat *secretariat, char *interogare) {
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
        UPDATE(secretariat, interogare);
    } else if (strstr(interogare, "DELETE ")) {
        DELETE(secretariat, interogare);
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
    /* snippet criptare:
    char *iv = "ab";
    char *key = "pclp1";
    cripteaza_studenti(secretariat, (void*)key, strlen(key), (void*)iv, strlen(iv), "fisier.acs");
    return 0;
    */

    int nr_interogari = 0;

    scanf("%d", &nr_interogari);
    getc(stdin);   // Citeste new-line '\n'

    char *linie = (char *) malloc(BUFFER_LENGTH * sizeof(char));

    for (int i = 0; i < nr_interogari; i++) {
        fgets(linie, BUFFER_LENGTH, stdin);

        // Inlocuieste new-line ('\n') cu NULL terminator ('\0'):
        if (linie[strlen(linie) - 1] == '\n')
            linie[strlen(linie) - 1] = '\0';

        proceseaza_interogare(secretariat, linie);
    }

    elibereaza_secretariat(&secretariat);

    free(linie);
    return 0;
}
