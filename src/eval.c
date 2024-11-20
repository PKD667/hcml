#include "../include/expr.h"

#include "math.h"

// evaluate a mathematical expression
int math_eval(char* expr) {
    
    struct expr *e = expr_create(expr, strlen(expr), NULL, NULL);
    if (e == NULL) {
        return 1;
    }

    int result = (int)expr_eval(e);

    return result;
}