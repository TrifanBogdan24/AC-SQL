#include "task2.h"

int main(int argc, char *argv[]) {
    // TODO(student): Task 2
    printf("Num of arguments: %d\n", argc);
    printf("Salut!\n");
    for (int i = 0; i < argc; i++) {
        printf("\t- Arg %d: %s ", i, argv[i]);
    }
    printf("\n\n");
    return 0;
}
