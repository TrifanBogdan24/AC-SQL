#include "task2.h"
#include "trim.h"


typedef struct {
    char *camp;
    char *op_comp;
    char *valoare;
} conditie;


/*
 Parseaza campurile din clauza SELECT si intoarce un array de string-uri
 
 Exemplu:
 if (toate_campurile == "nume") return ["nume"]
 if (toate_campurile == "nume, medie_generala") return ["nume", "medie_generala"]

 ATENTIE! Functia nu poate parsa caracterul "*" de globbing
*/
char **parseaza_campuri_SELECT(char *toate_campurile, int *nr_campuri)
{
    char **campuri = NULL;
    char *token = strtok(toate_campurile, ",");

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
 if (clauza_where == "id = 0") return ["id = 0"]
 if (clauza_where == "id = 0 AND nume = USO") return ["id = 0", "nume = USO"]

 ATENTIE! Functia nu se ocupa de eliminarea caracterului ';' de la finalul interogarii.
          Acel caracter trebuie sters inaine de a apela functia!
*/
char **get_conditii_strings_clauza_WHERE(char *clauza_where, int *nr_conditii)
{
    char **conditii = NULL;
    *nr_conditii = 0;
    
    const char *delim = "AND";
    size_t len_delim = strlen(delim);
    char *ptr = clauza_where;
    char *found = strstr(ptr, delim);  // prima căutare

    while (found) {
        *found = '\0';

        (*nr_conditii)++;
        conditii = realloc(conditii, *nr_conditii * sizeof(char*));
        int idx = (*nr_conditii) - 1;
        conditii[idx] = malloc(250 * sizeof(char));
        strcpy(conditii[idx], ptr);

        ptr = found + len_delim;
        found = strstr(ptr, delim);
    }

    // Ultima bucata (dupa ultimul "AND" sau intregul sir daca nu exista "AND")
    if (*ptr != '\0') {
        (*nr_conditii)++;
        conditii = realloc(conditii, *nr_conditii * sizeof(char*));
        int idx = (*nr_conditii) - 1;
        conditii[idx] = malloc(250 * sizeof(char));
        strcpy(conditii[idx], ptr);
    }
    
    return conditii;
}

/*
 Parseaza clauza WHERE si returneaza un array de conditii, plasate in struct-uri

 if (clauza_where == "id = 0") return [
    conditie(camp="id", op_comp="=", valoare="0")
    ]
 if (clauza_where == "id = 0 AND nume != USO") return [
    conditie(camp="id", op_comp="=", valoare="0"),
    conditie(camp="nume", op_comp="!=", valoare="USO"),
    ]

 ATENTIE! Functia nu se ocupa de eliminarea caracterului ';' de la finalul interogarii.
          Acel caracter trebuie sters inaine de a apela functia!
*/
conditie *parseaza_clauza_WHERE(char *clauza_where, int *nr_conditii)
{
    (*nr_conditii) = 0;
    char **str_conditii = get_conditii_strings_clauza_WHERE(clauza_where, nr_conditii);

    conditie *conditii = (conditie *) malloc((*nr_conditii) * sizeof(conditie));

    for (int i = 0; i < *nr_conditii; i++) {
        // <camp> <operator> <valoare>
        conditie *conditie = &conditii[i];
        conditie->camp = (char *) malloc(150 * sizeof(char));
        conditie->op_comp = (char *) malloc(150 * sizeof(char));
        conditie->valoare = (char *) malloc(150 * sizeof(char));

        char *token = strtok(str_conditii[i], " ");
        strcpy(conditie->camp, token);
        trim(conditie->camp);

        token = strtok(NULL, " ");
        strcpy(conditie->op_comp, token);
        trim(conditie->op_comp);

        token = strtok(NULL, " ");
        strcpy(conditie->valoare, token);
        trim(conditie->valoare);
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
        if (!strcmp(cond.op_comp, "<"))
            return strcmp(student.nume, nume) < 0;
        if (!strcmp(cond.op_comp, ">"))
            return strcmp(student.nume, nume) > 0;

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
        if (!strcmp(cond.op_comp, "<"))
            return strcmp(materie.nume, nume) < 0;
        if (!strcmp(cond.op_comp, ">"))
            return strcmp(materie.nume, nume) > 0;

        return 0;   // Conditie WHERE invalida
    } else if (!strcmp(cond.camp, "nume_titular")) {
        char *nume_titular = cond.valoare;
        
        if (!strcmp(cond.op_comp, "="))
            return strcmp(materie.nume_titular, nume_titular) == 0;
        if (!strcmp(cond.op_comp, "!="))
            return strcmp(materie.nume_titular, nume_titular) != 0;
        if (!strcmp(cond.op_comp, "<"))
            return strcmp(materie.nume_titular, nume_titular) < 0;
        if (!strcmp(cond.op_comp, ">"))
            return strcmp(materie.nume_titular, nume_titular) > 0;

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
int match_student_on_all_conditii(student student, int nr_conds, conditie *conds)
{
    for (int i = 0; i < nr_conds; i++)
        if (!match_student_on_conditie(student, conds[i]))
            return 0;
    return 1;
}


/*
 * Returneaza 1 (TRUE) daca MATERIA respecta TOATE conditiile,
 * Altfel, intoarce 0 (FALSE)
*/
int match_materie_on_all_conditii(materie materie, int nr_conds, conditie *conds)
{
    for (int i = 0; i < nr_conds; i++)
        if (!match_materie_on_conditie(materie, conds[i]))
            return 0;
    return 1;
}


/*
 * Returneaza 1 (TRUE) daca INROLARE respecta TOATE conditiile,
 * Altfel, intoarce 0 (FALSE)
*/
int match_inrolare_on_all_conditii(inrolare inrolare, int nr_conds, conditie *conds)
{
    for (int i = 0; i < nr_conds; i++)
        if (!match_inrolare_on_conditie(inrolare, conds[i]))
            return 0;
    return 1;
}


void SELECT_FROM_studenti(secretariat *secretariat,
    int nr_campuri, char **campuri, 
    int nr_conds, conditie *conds)
{
    for (int i = 0; i < secretariat->nr_studenti; i++) {
        student student = secretariat->studenti[i];

        if (nr_conds > 0
                && !match_student_on_all_conditii(student, nr_conds, conds))
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
    int nr_conds, conditie *conds)
{
    for (int i = 0; i < secretariat->nr_materii; i++) {
        materie materie = secretariat->materii[i];

        if (nr_conds > 0
                && !match_materie_on_all_conditii(materie, nr_conds, conds))
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
    int nr_conds, conditie *conds)
{
    for (int i = 0; i < secretariat->nr_inrolari; i++) {
        inrolare inrolare = secretariat->inrolari[i];

        if (nr_conds > 0
            && !match_inrolare_on_all_conditii(inrolare, nr_conds, conds))
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
    char **campuri = (char **) malloc(nr_campuri * sizeof(char));
    for (int i = 0; i < nr_campuri; i++)
        campuri[i] = (char *) malloc(30 * sizeof(char));
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


/* Template:
SELECT <campuri> FROM <tabel> WHERE <camp> <comp> <valoare>;
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
        interogare + idx_SELECT + strlen("SELECT "),
        buffer_len);
    toate_campurile[buffer_len] = '\0';
    trim(toate_campurile);

    // Are valoarea '1' in caz de "SELECT *" si '0' atunci cand "SELECT <campuri>"
    int is_select_all =
        (strcmp(toate_campurile, "*") == 0);

    int nr_campuri = 0;
    char **campuri = NULL;

    if (!is_select_all) {
        // NU se foloseste globbing-ul "*"
        campuri = parseaza_campuri_SELECT(toate_campurile, &nr_campuri);
    } else {
        // Se foloseste globbing-ul "*"
        campuri = build_campuri(nume_tabela, &nr_campuri);
    }

    // Conditii din clauza WHERE:
    int nr_conds = 0;
    conditie *conds = NULL;

    if (ptr_WHERE) {
        /* SELECT ... FROM <tabel> WHERE ...; */

        int idx_WHERE = ptr_WHERE - interogare;
        buffer_len = idx_WHERE - idx_FROM - strlen(" FROM ");
        strncpy(
            nume_tabela,
            interogare + idx_FROM + strlen(" FROM "),
            buffer_len);
        nume_tabela[buffer_len] = '\0';

        /* WHERE <camp> <comp> <valoare>; */
        /* WHERE <conditie1> AND <conditie2> AND ... AND <conditieN>; */
        char *clauza_WHERE = (char *) malloc(250 * sizeof(char));
        strncpy(
            clauza_WHERE,
            interogare + idx_WHERE + strlen(" WHERE "),
            250
        );
        conds = parseaza_clauza_WHERE(clauza_WHERE, &nr_conds);
    } else {
        /* SELECT ... FROM <tabel>; */
        buffer_len = strlen(interogare) - idx_FROM - strlen(" FROM ");
        strncpy(
            nume_tabela,
            interogare + idx_FROM + strlen(" FROM "),
            buffer_len);
        nume_tabela[buffer_len] = '\0';
    }


    trim(nume_tabela);

    if (!is_valid_table(nume_tabela)) {
        return;
    }


    ret_val = 0;
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
            nr_conds, conds);
    } else if (!strcmp(nume_tabela, "materii")) {
        SELECT_FROM_materii(
            secretariat,
            nr_campuri, campuri, 
            nr_conds, conds);
    } else if (!strcmp(nume_tabela, "inrolari")) {
        SELECT_FROM_inrolari(
            secretariat,
            nr_campuri, campuri, 
            nr_conds, conds);
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

    // Sterge caracterul punct-si-virgula ';' din textul interogarii
    interogare[strlen(interogare) - 1] = '\0';


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
