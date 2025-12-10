#include "error.h"

static ErrorState errorState = {false, false};

void error_init(void) {
    errorState.hadError = false;
    errorState.panicMode = false;
}

void error_at(int line, const char* message) {
    if (errorState.panicMode) return;
    errorState.panicMode = true;
    
    fprintf(stderr, "[line %d] Error: %s\n", line, message);
    errorState.hadError = true;
}

void error(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
    errorState.hadError = true;
}

bool had_error(void) {
    return errorState.hadError;
}

void reset_error(void) {
    errorState.hadError = false;
    errorState.panicMode = false;
}
