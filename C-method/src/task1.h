#pragma once

#include "../include/structuri.h"

// Functie folosita la crearea bazei de date si la clauza UPDATE
void calculeaza_medii_generale(secretariat *s);

secretariat *citeste_secretariat(const char *nume_fisier);

void adauga_student(secretariat *s, int id, char *nume, int an_studiu, char statut, float medie_generala);

void elibereaza_secretariat(secretariat **s);
