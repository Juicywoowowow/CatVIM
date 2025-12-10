#include "repl.h"
#include "cemvm.h"
#include "error.h"

void run_repl(void) {
    char line[1024];
    VM vm;
    init_vm(&vm);

    printf("Camel REPL v%s\n", CAMEL_VERSION);
    printf("Type 'exit' to quit\n\n");

    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        if (strcmp(line, "exit\n") == 0) {
            break;
        }

        reset_error();
        interpret(&vm, line);
    }

    free_vm(&vm);
}
