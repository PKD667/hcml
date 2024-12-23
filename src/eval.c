#include "../include/expr.h"
#include "../include/hcml.h"
#include "../include/cutils.h"


#include <string.h>
#include <stdlib.h>

// var parent / child separator : '#'

char* get_variable_value(struct html_tag* vars, char* name) {

    char* v_name = strdup(name);

    dbg(3, "Getting value of '%s'", name);

    // if var has a # it means it's a nested variable
    char* child_name = NULL;
    char* dot = strchr(v_name, '#');
    if (dot != NULL) {
        // get the parent variable
        *dot = '\0';
        // get the child variable
        child_name = dot + 1;
    }

    dbg(3, "Getting variable %s in vars %p", v_name, vars);
    struct html_tag* var = get_var(vars, v_name);
    dbg(3, "Got variable %s", v_name);
    if (var == NULL) {
        return NULL;
    }

    if (child_name != NULL) {
        return get_variable_value(var, child_name);
    }

    dbg(3, "Getting variable %s with value %s", name, var->content);
    return var->content;
}

// var parent / child separator : '#'

int update_variable_value(struct html_tag* vars, char* name, char* value) {

    dbg(3, "Updating variable %s to %s", name, value);

    char* v_name = strdup(name);

    // if var has a dot, it means it's a nested variable
    char* child_name = NULL;
    char* dot = strchr(v_name, '#');
    if (dot != NULL) {
        // get the parent variable
        *dot = '\0';
        // get the child variable
        child_name = dot + 1;
    }

    struct html_tag* var = get_var(vars, v_name);
    if (var == NULL) {
        return 1;
    }

    if (child_name != NULL) {
        return update_variable_value(var, child_name, value);
    }

    char* old_content = var->content;
    var->content = strdup(value);

    free(old_content);
    free(v_name);

    return 0;
}           

// check if a string is a numeric value (int or float)
int numval(const char* cs) {

    if (cs == NULL || *cs == '\0') {
        return 0;
    }
    int fp = 0; // float point
    char* c = strdup(cs); // create a copy of cs
    char* temp = c; // keep a pointer to free the memory later

    while (*c != '\0') {
        if (!isdigit(*c)) {
            if (*c == '.') { fp++; /*Found a DOT `.` */ }
            else { free(temp); return 0; /*`char* cs` is not a number*/ }
        }
        c++;
    }
    free(temp); // free the copied string

    if (fp == 0) { return 1; /*`char* cs` is an integer*/ }
    else if (fp == 1) { return 2; /*`char* cs` is a float*/ }

    return 0; // `char* cs` is not a number
}

long unsigned int hash(const char *str) {
    long unsigned int hash = 0xcbf29ce484222325ULL; // FNV offset basis
    while (*str) {
        hash ^= (unsigned char)*str++;
        hash *= 0x100000001b3ULL; // FNV prime
    }
    return hash & 0xFFFFF;
}

int hash_strings(char** expr) {
    // count all occurences of ''' in the expression
    int count = 0;
    for (int i = 0; (*expr)[i]; i++) {
        if ((*expr)[i] == '\'') {
            count++;
        }
    }

    // if there are no occurences of ''' in the expression, return
    if (count == 0) {
        return 0;
    }

    dbg(3, "Found %d strings to hash", count / 2);

    #define MAX_HASH_SIZE 20

    // create a new expression with the correct size
    char* new_expr = calloc(strlen(*expr) + (MAX_HASH_SIZE) * ((count + 1) / 2) + 1 , sizeof(char));

    for (int i = 0; (*expr)[i]; i++) {
        if ((*expr)[i] == '\'') {
            
            dbg(3, "Found string at %d", i);

            int l = 0;
            while ((*expr)[i+l+1] != '\'' && (*expr)[i+l+1] != '\0') {
                l++;
            }

            dbg(3, "Found string of length %d", l);

            char* str_to_hash = calloc(l+1, sizeof(char));
            dbg(3,"Copying '%.*s' to str_to_hash", l, (*expr) + i + 1);
            strncpy(str_to_hash, (*expr) + i + 1, l);

            dbg(3, "Hashing '%s'", str_to_hash);

            long unsigned int h = hash(str_to_hash);

            char hash_str[20];
            sprintf(hash_str, "%lu", h);

            dbg(3, "Hashed to %s", hash_str);

            strcat(new_expr, hash_str);

            i = i + l + 1;
        } else {
            strncat(new_expr, &((*expr)[i]), 1);
        }
    }

    *expr = new_expr;

    dbg(3, "New expression: %s", new_expr);

    return 0;
}



int eval(char* expr, struct html_tag* vars) {
    // First verify we have a valid expression
    if (!expr || !*expr) {
        dbg(1, "Empty expression");
        return -1;
    }

    dbg(3,"Hashing strings in expression %s", expr);

    char* old_expr = expr;

    // hash the strings
    if (hash_strings(&expr) != 0) {
        dbg(1, "Failed to hash strings");
        return -1;
    }
    dbg(3,"Hashed expression: %s", expr);
    

    struct expr_var_list expr_vars = {0};

    struct expr *e = expr_create(expr, strlen(expr), &expr_vars, NULL);
    if (!e) {
        dbg(1, "Failed to create expression");
        return -1; // Failed to create expression
    }

    // set the variables
    int var_count = 0;
    struct expr_var *v;
    for (v = expr_vars.head; v != NULL; v = v->next) {
        var_count++;
    }

    // Fill array
    int i = 0;
    for (v = expr_vars.head; v != NULL; v = v->next) {
        char* name = v->name;

        dbg(3, "Parsing element %s", name);  

        // get the value of the variable
        char* value = get_variable_value(vars, name); // variable reference
        if (value == NULL) {
            dbg(1, "Variable %s not found", name);
            expr_destroy(e, &expr_vars);
            return -1; // Variable not found
        }

        dbg(3, "Value of %s is %s", name, value);
        // if the variable is a number, we dont really need to hash it 
        if (numval(value)) {
            dbg(3, "Variable %s is a number", name);
            expr_var(&expr_vars, name, strlen(name))->value = atof(value);
            continue;
        }

        // hash the variable
        long unsigned int h = hash(value);
        // convert the hash to a string
        char hash_str[20];
        sprintf(hash_str, "%lu", h);

        dbg(3, "Hashing variable %s with value %s to %s", name, value, hash_str);

        // set the variable value
        expr_var(&expr_vars, name, strlen(name))->value = atof(hash_str);
        i++;
    }

    dbg(3, "Parsed %d elements", i);

    float result = expr_eval(e);
    if (isnan(result) || isinf(result)) {
        dbg(1, "Expression evaluation failed");
        expr_destroy(e, &expr_vars);
         // Clean up expr_vars
        struct expr_var *curr = expr_vars.head;
        while (curr) {
            struct expr_var *next = curr->next;
            free(curr);
            curr = next;
        }

        return -2; // Expr evaluation failed
    }

    dbg(3, "Expression evaluated to %f", result);


    expr_destroy(e, &expr_vars);

    if (old_expr != expr) {
        free(expr);
    }

    return (int)result;
}
