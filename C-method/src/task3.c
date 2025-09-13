#include "task3.h"
#include <stdio.h>
#include <stdlib.h>

#define NR_BLOCKS 4

void print_hex_block(const unsigned char *block, size_t block_len) {
    for (size_t i = 0; i < block_len; i++) {
        printf("0x%02x", block[i]);

        if (i < block_len - 1)
            printf(" ");
    }
}

void print_blocks(unsigned char **blocks, size_t block_len) {
    for (int i = 0; i < NR_BLOCKS; i++) {
        printf("Blocul %d: ", i);
        print_hex_block(blocks[i], block_len);
        printf("\n");
    }
    printf("\n");
}

void XOR(
    unsigned char *block, size_t block_len,
    const unsigned char *key, size_t key_len) {
    size_t idx = 0;
    for (size_t i = 0; i < block_len; i++) {
        block[i] ^= key[idx];
        idx = (idx + 1) % key_len;
    }
}


void P_BOX(unsigned char *block, size_t block_len) {
    unsigned char *temp = calloc(block_len, sizeof(unsigned char));
    // Aplica permutarea:
    for (size_t i = 0; i < block_len; i++) {
        size_t j = (i * (block_len - 1) + 2) % block_len;
        temp[j] = block[i];
    }
    memcpy(block, temp, block_len);
    free(temp);
}

void elibereaza_blocuri(unsigned char ***blocks) {
    if (!(*blocks)) return;
    for (int i = 0; i < NR_BLOCKS; i++)
        free((*blocks)[i]);
    free(*blocks);
}


void cripteaza_studenti(secretariat *secretariat, void *key, size_t key_len,
                        void *iv, size_t iv_len, char *cale_output) {
    // TODO(student): Cripteaza vectorul secretariat->studenti si scrie rezultatul in fisierul cale_output


    unsigned char *iv_str = (unsigned char *) iv;
    unsigned char *key_str = (unsigned char *) key;
    /* Pentru debugging: printf("IV: %s\n", iv_str); */
    /* Pentru debugging: printf("Key: %s\n", key_str); */


    /* Transforma vectorul  de studenti intr-un array de bytes */
    size_t nr_bytes_sruct_student =
        sizeof(int)
        + MAX_STUDENT_NAME * sizeof(char)
        + sizeof(int) + sizeof(char) + sizeof(float);

    size_t length = secretariat->nr_studenti * nr_bytes_sruct_student;

    unsigned char *bytes_studenti = calloc(length, 1);
    unsigned char *p = bytes_studenti;

    for (int i = 0; i < secretariat->nr_studenti; i++) {
        student st = secretariat->studenti[i];
        memcpy(p, &st.id, sizeof(int)); p += sizeof(int);
        memcpy(p, st.nume, MAX_STUDENT_NAME); p += MAX_STUDENT_NAME;
        memcpy(p, &st.an_studiu, sizeof(int)); p += sizeof(int);
        memcpy(p, &st.statut, sizeof(char)); p += sizeof(char);
        memcpy(p, &st.medie_generala, sizeof(float)); p += sizeof(float);
    }

    /*  Padding + imparte in 4 blocuri */
    size_t padding = 0;
    while ((length + padding) % 4 != 0)
        padding++;

    size_t total_length = length + padding;
    size_t block_length = total_length / 4;


    unsigned char **blocks = malloc(NR_BLOCKS * sizeof(unsigned char *));


    for (int i = 0; i < NR_BLOCKS; i++) {
        // Aloca memorie si initializa cu 0 (0x00):
        blocks[i] = calloc(block_length, sizeof(unsigned char));

        if (i < NR_BLOCKS - 1) {
            memcpy(blocks[i], bytes_studenti + i * block_length, block_length);
        } else {
            // Copiaza doar lungimea blocului ramas:
            memcpy(blocks[i], bytes_studenti + i * block_length, length - 3 * block_length);
        }
    }

    free(bytes_studenti);


    /* Criptarea primului bloc: XOR(blocks[0], IV) */
    XOR(blocks[0], block_length, iv_str, iv_len);

    /* Aplicare S-BOX: XOR(blocks[0], key) */
    XOR(blocks[0], block_length, key_str, key_len);

    /* Aplicare P-BOX */
    P_BOX(blocks[0], block_length);


    for (int i = 1; i < NR_BLOCKS; i++) {
        /* XOR intre blocul curent si cel precedent: XOR(blocks[i], blocks[i - 1]_enc)  */
        XOR(blocks[i], block_length, blocks[i - 1], block_length);

        /* Aplicare S-BOX: XOR(blocks[i], key) */
        XOR(blocks[i], block_length, key_str, key_len);

        /* Aplicare P-BOX */
        P_BOX(blocks[i], block_length);
    }

    FILE *fout = fopen(cale_output, "wb");

    if (!fout) {
        elibereaza_blocuri(&blocks);
        fprintf(stderr, "[EROARE] Nu s-a putut deschide fisierul \"%s\"\n", cale_output);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NR_BLOCKS; i++) {
        for (size_t j = 0; j < block_length; j++)
            fwrite(&blocks[i][j], sizeof(unsigned char), 1, fout);
    }

    fclose(fout);
    elibereaza_blocuri(&blocks);
}
