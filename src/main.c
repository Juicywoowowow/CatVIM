#include "common.h"
#include "cemvm.h"
#include "file.h"
#include "repl.h"
#include "error.h"

static void run_file(const char* path) {
    char* source = read_file(path);
    if (source == NULL) {
        exit(74);
    }

    VM vm;
    init_vm(&vm);
    
    reset_error();
    InterpretResult result = interpret(&vm, source);
    
    free_vm(&vm);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
    error_init();

    if (argc == 1) {
        run_repl();
    } else if (argc == 2) {
        run_file(argv[1]);
    } else {
        fprintf(stderr, "Usage: camel [path]\n");
        exit(64);
    }

    return 0;
}
