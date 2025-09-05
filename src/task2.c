#include "task2.h"
#include "trim.h"


typedef struct {
    char *camp;
    char *op_comp;  // Operatorul de comperatie
    char *valoare;
} conditie;






/*
 * Daca tabela exista in baza de date, atunci intoarce 1 (TRUE)
 * Altfel, scrie un text de eroare si intoarce 0 (FALSE)
*/
int is_valid_table(const char *nume_tabela)
{
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
int is_valid_field(char *nume_tabela, char *camp)
{
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
        && strcmp(camp, "status") && strcmp(camp, "medie_generala")) {
        err_flag = 0;
    }

    if (!strcmp(nume_tabela, "materii")
        && strcmp(camp, "id") && strcmp(camp, "nume") && strcmp(camp, "nume_titular")) {
        err_flag = 0;
    }

    if (!strcmp(nume_tabela, "inrolari")
        && strcmp(camp, "id_student") && strcmp(camp, "id_materie") && strcmp(camp, "note")) {
        err_flag = 0;
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
char **split_conditii_into_strings(char *str_conditii, int *nr_conditii)
{
    char **conditii_strings = NULL;
    *nr_conditii = 0;
    
    const char *delim = "AND";
    size_t len_delim = strlen(delim);
    char *ptr = str_conditii;
    char *found = strstr(ptr, delim);  // prima căutare

    while (found) {
        *found = '\0';

        (*nr_conditii)++;
        conditii_strings = realloc(conditii_strings, *nr_conditii * sizeof(char*));
        int idx = (*nr_conditii) - 1;
        conditii_strings[idx] = malloc(250 * sizeof(char));
        strcpy(conditii_strings[idx], ptr);

        ptr = found + len_delim;
        found = strstr(ptr, delim);
    }

    // Ultima bucata (dupa ultimul "AND" sau intregul sir daca nu exista "AND")
    if (*ptr != '\0') {
        (*nr_conditii)++;
        conditii_strings = realloc(conditii_strings, *nr_conditii * sizeof(char*));
        int idx = (*nr_conditii) - 1;
        conditii_strings[idx] = malloc(250 * sizeof(char));
        strcpy(conditii_strings[idx], ptr);
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
conditie *parseaza_conditiile_WHERE(char *str_conditii, int *nr_conditii)
{
    (*nr_conditii) = 0;
    char **conditii_strings = split_conditii_into_strings(str_conditii, nr_conditii);

    conditie *conditii = (conditie *) malloc((*nr_conditii) * sizeof(conditie));

    for (int i = 0; i < *nr_conditii; i++) {
        // <camp> <operator> <valoare>
        conditie *conditie = &conditii[i];
        conditie->camp = (char *) malloc(150 * sizeof(char));
        conditie->op_comp = (char *) malloc(150 * sizeof(char));
        conditie->valoare = (char *) malloc(150 * sizeof(char));

        char *token = strtok(conditii_strings[i], " ");
        strcpy(conditie->camp, token);
        trim(conditie->camp);

        token = strtok(NULL, " ");
        strcpy(conditie->op_comp, token);
        trim(conditie->op_comp);

        token = strtok(NULL, "");
        strcpy(conditie->valoare, token);
        trim(conditie->valoare);

        // Elimina ghilimelele/apostrofurile de la inceput/sfarsit
        size_t len = strlen(conditie->valoare);
        if (len >= 2 &&
            ((conditie->valoare[0] == '\'' && conditie->valoare[len - 1] == '\'')
            ||(conditie->valoare[0] == '\"' && conditie->valoare[len - 1] == '\"'))) {
            strcpy(conditie->valoare, conditie->valoare + 1);
            len--;
            conditie->valoare[len - 1] = '\0';
        }
    }

    return conditii;
}

/*
 * Returneaza 1 (TRUE) daca STUDENTUL respecta conditia,
 * Altfel, intoarce 0 (FALSE)
*/
int match_student_on_conditie(student student, conditie cond)
{
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

        return 0;   // Conditie WHERE invalida
    
    } else if (!strcmp(cond.camp, "statut")) {
        char statut = cond.valoare[0];

        if (!strcmp(cond.op_comp, "="))
            return student.statut == statut;
        if (!strcmp(cond.op_comp, "!="))
            return student.statut != statut;

        return 0;   // Conditie WHERE invalida
    } else if (!strcmp(cond.camp, "medie_generala")) {
        float medie_generala = atof(cond.valoare);

        if (!strcmp(cond.op_comp, "="))
            return student.medie_generala == medie_generala;
        if (!strcmp(cond.op_comp, "!="))
            return student.medie_generala != medie_generala;
        if (!strcmp(cond.op_comp, "<"))
            return student.medie_generala < medie_generala;
        if (!strcmp(cond.op_comp, ">"))
            return student.medie_generala > medie_generala;

        return 0;   // Conditie WHERE invalida
    }

    return 0;   // Conditie WHERE invalida
}


/*
 * Returneaza 1 (TRUE) daca MATERIA respecta conditia,
 * Altfel, intoarce 0 (FALSE)
*/
int match_materie_on_conditie(materie materie, conditie cond)
{
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
int match_inrolare_on_conditie(inrolare inrolare, conditie cond)
{
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

        return 0;   // Conditie WHERE invalida
    } else if (!strcmp(cond.camp, "note")) {
        char *token = strtok(cond.valoare, " ");
        float nota_laborator = atof(token);

        token = strtok(NULL, " ");
        float nota_partial = atof(token);

        token = strtok(NULL, " ");
        float nota_final = atof(token);

        if (!strcmp(cond.op_comp, "="))
            return (nota_laborator == inrolare.note[0]
            && nota_partial == inrolare.note[1]
            && nota_final == inrolare.note[2]);
        else if (!strcmp(cond.op_comp, "="))
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
int match_student_on_all_conditii(student student, int nr_conditii, conditie *conditii)
{
    for (int i = 0; i < nr_conditii; i++)
        if (!match_student_on_conditie(student, conditii[i]))
            return 0;
    return 1;
}


/*
 * Returneaza 1 (TRUE) daca MATERIA respecta TOATE conditiile,
 * Altfel, intoarce 0 (FALSE)
*/
int match_materie_on_all_conditii(materie materie, int nr_conditii, conditie *conditii)
{
    for (int i = 0; i < nr_conditii; i++)
        if (!match_materie_on_conditie(materie, conditii[i]))
            return 0;
    return 1;
}


/*
 * Returneaza 1 (TRUE) daca INROLARE respecta TOATE conditiile,
 * Altfel, intoarce 0 (FALSE)
*/
int match_inrolare_on_all_conditii(inrolare inrolare, int nr_conditii, conditie *conditii)
{
    for (int i = 0; i < nr_conditii; i++)
        if (!match_inrolare_on_conditie(inrolare, conditii[i]))
            return 0;
    return 1;
}


void SELECT_FROM_studenti(secretariat *secretariat,
    int nr_campuri, char **campuri, 
    int nr_conditii, conditie *conditii)
{
    for (int i = 0; i < secretariat->nr_studenti; i++) {
        student student = secretariat->studenti[i];

        if (nr_conditii > 0
                && !match_student_on_all_conditii(student, nr_conditii, conditii))
            continue;
        
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
}

void SELECT_FROM_materii(secretariat *secretariat,
    int nr_campuri, char **campuri, 
    int nr_conditii, conditie *conditii)
{
    for (int i = 0; i < secretariat->nr_materii; i++) {
        materie materie = secretariat->materii[i];

        if (nr_conditii > 0
                && !match_materie_on_all_conditii(materie, nr_conditii, conditii))
            continue;

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
}


void SELECT_FROM_inrolari(secretariat *secretariat,
    int nr_campuri, char **campuri, 
    int nr_conditii, conditie *conditii)
{
    for (int i = 0; i < secretariat->nr_inrolari; i++) {
        inrolare inrolare = secretariat->inrolari[i];

        if (nr_conditii > 0
            && !match_inrolare_on_all_conditii(inrolare, nr_conditii, conditii))
            continue;

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


char **allocate_campuri(int nr_campuri)
{
    char **campuri = (char **) malloc(nr_campuri * sizeof(char *));
    for (int i = 0; i < nr_campuri; i++)
        campuri[i] = (char *) malloc(50 * sizeof(char));
    return campuri;
}

char **build_campuri_STUDENTI(int *nr_campuri)
{
    (*nr_campuri) = 5;

    char **campuri = allocate_campuri(*nr_campuri);
    strcpy(campuri[0], "id");
    strcpy(campuri[1], "nume");
    strcpy(campuri[2], "an_studiu");
    strcpy(campuri[3], "statut");
    strcpy(campuri[4], "medie_generala");
    return campuri;
}

char **build_campuri_METERIE(int *nr_campuri)
{
    (*nr_campuri) = 3;

    char **campuri = allocate_campuri(*nr_campuri);
    strcpy(campuri[0], "id");
    strcpy(campuri[1], "nume");
    strcpy(campuri[2], "nume_titular");
    return campuri;
}


char **build_campuri_INROLARE(int *nr_campuri)
{
    (*nr_campuri) = 3;

    char **campuri = allocate_campuri(*nr_campuri);
    strcpy(campuri[0], "id_student");
    strcpy(campuri[1], "id_materie");
    strcpy(campuri[2], "note");
    return campuri;
}


char **build_campuri(char *nume_tabela, int *nr_campuri)
{
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
char **parseaza_campurile_SELECT(char *str_conditii, int *nr_campuri)
{
    char **campuri = NULL;
    char *token = strtok(str_conditii, ",");

    (*nr_campuri) = 0;

    while (token) {
        printf("%s\n", token);
        (*nr_campuri)++;
        int idx = (*nr_campuri) - 1;
        campuri = realloc(campuri, *nr_campuri * sizeof(char*));
        campuri[idx] = malloc(150 * sizeof(char));
        strcpy(campuri[idx], token);
        trim(campuri[idx]);
        token = strtok(NULL, ",");
        printf("%s\n", campuri[idx]);
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
void SELECT(secretariat *secretariat, char *interogare)
{
    char *ptr = strstr(interogare, "SELECT");
    if (!ptr) return;
    ptr += strlen("SELECT");

    // Extrage <campuri>
    while (isspace((unsigned char)*ptr)) ptr++;
    char *fromPos = strstr(ptr, "FROM");
    if (!fromPos) return;

    char *str_campuri = (char*) malloc(256 * sizeof(char));
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

    char *nume_tabela = (char*) malloc(128 * sizeof(char));
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
    char *str_conditii = (char *) malloc(256 * sizeof(char));
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

        for (int i = 0; i < nr_conditii; i++) {
            printf("\"%s\" \"%s\" \"%s\"\n",
                conditii[i].camp, conditii[i].op_comp, conditii[i].valoare);
        }
    }



    if (is_select_all) {
        // Se foloseste globbing-ul "*"
        printf("aici\n");
        campuri = build_campuri(nume_tabela, &nr_campuri);
        printf("aici NU\n");

    } else {
        campuri = parseaza_campurile_SELECT(str_campuri, &nr_campuri);
    }

    for (int i = 0; i < nr_conditii; i++) {
        printf("Camp[%d] = \"%s\"\n", i, campuri[i]);
    }

    printf("bine 3\n");

    if (!is_valid_table(nume_tabela)) {
        return;
    }


    int ret_val = 0;
    for (int i = 0; i < nr_campuri; i++) {
        printf("camp[%d] = \"%s\"\n", i, campuri[i]);
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


    free(campuri);
    free(conditii);
    free(str_campuri);
    free(str_conditii);
    free(nume_tabela);
}

/* Template:
UPDATE <tabel> SET <camp> = <valoare> WHERE <conditie>;
UPDATE <tabel> SET <camp> = <valoare> WHERE <cond1> AND <cond2>;
*/
void UPDATE(char *interogare)
{
    char *ptr = strstr(interogare, "UPDATE");
    if (!ptr)
        return;

    ptr += strlen("UPDATE");

    // Exrage <tabel>
    while (isspace((unsigned char)*ptr)) ptr++;
    char *endTable = strstr(ptr, "SET");
    if (!endTable) return;

    char *tabel = (char *) malloc(128 * sizeof(char));
    strncpy(tabel, ptr, endTable - ptr);
    tabel[endTable - ptr] = '\0';
    trim(tabel);

    // Extrage partea "<camp> = <valaore>"
    ptr = endTable + strlen("SET");
    while (isspace((unsigned char)*ptr)) ptr++;
    char *endWhere = strstr(ptr, "WHERE");
    if (!endWhere) return;

    char *campValoare = (char *) malloc(128 * sizeof(char));
    strncpy(campValoare, ptr, endWhere - ptr);
    campValoare[endWhere - ptr] = '\0';
    trim(campValoare);

    // Imparte <camp> de <valoare> folosind separatorul '='
    char *eq = strchr(campValoare, '=');
    if (!eq) return;

    char *camp = (char *) malloc(64 * sizeof(char));
    char *valoare = (char *) malloc(64 * sizeof(char));
    strncpy(camp, campValoare, eq - campValoare);
    camp[eq - campValoare] = '\0';
    strcpy(valoare, eq + 1);

    trim(camp);
    trim(valoare);

    // Extrage conditie/conditii (TOT ce este dupa WHERE)
    ptr = endWhere + strlen("WHERE");
    while (isspace((unsigned char)*ptr)) ptr++;
    char *conditii = (char*) malloc(256 * sizeof(char));
    strncpy(conditii, ptr, strlen(ptr));
    conditii[strlen(ptr)] = '\0';
    trim(conditii);

    // Sterge ';' de la final
    size_t len = strlen(conditii);
    if (len > 0 && conditii[len - 1] == ';')
        conditii[len - 1] = '\0';

    printf("Tabel: \"%s\"\n", tabel);
    printf("Camp: \"%s\"\n", camp);
    printf("Valoare: \"%s\"\n", valoare);
    printf("Conditii: \"%s\"\n", conditii);

    free(tabel);
    free(camp);
    free(valoare);
    free(conditii);
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
        // TODO
    } else if (strstr(interogare, "DELETE ")) {
        // TODO
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
