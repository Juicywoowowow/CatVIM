#ifndef camel_error_h
#define camel_error_h

#include "common.h"

typedef struct {
    bool hadError;
    bool panicMode;
} ErrorState;

void error_init(void);
void error_at(int line, const char* message);
void error(const char* message);
bool had_error(void);
void reset_error(void);

#endif
