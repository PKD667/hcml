

#include "test.h"

/*
    EVAL TESTS

    The evaluation functions are a subset of the HCML functions
    They are used to evaluate expressions and variables
    (`if` conditions, `op` blocks, etc)
*/

int test_eval() {
    int failed = 0;

    // Test 1: Basic expression
    {
        char* input = "1 + 1";
        int res = eval(input, NULL);
        if (res != 2) {
            msg(ERROR,"Basic expression test failed");
            failed++;
        } else {
            msg(INFO,"Basic expression test PASSED");
        }
    }

    // Test 2: Variable evaluation
    {
        char* input = "1 + var";
        struct html_tag* vars = calloc(1, sizeof(struct html_tag));
        vars->name = strdup("vars");
        
        vars->childs = calloc(1, sizeof(struct html_tag*));
        vars->childs[0] = calloc(1, sizeof(struct html_tag));
        vars->childs[0]->name = strdup("var");
        vars->childs[0]->content = strdup("2");

        vars->childs_count = 1;

        int res = eval(input, vars);
        free(vars->name);
        free(vars->content);
        if (res != 3) {
            msg(ERROR,"Variable evaluation test failed");
            failed++;
        } else {
            msg(INFO,"Variable evaluation test PASSED");
        }
    }

    // Test 3: string comparision
    {      
        // should evaluate:
        // (hello == hello or dog == cat ) -> true
        char* input = "string1 == string2 || string3 == string4";

        struct html_tag* vars = calloc(1, sizeof(struct html_tag));
        vars->name = strdup("vars");
        vars->childs = calloc(4, sizeof(struct html_tag*));

        vars->childs[0] = calloc(1, sizeof(struct html_tag));
        vars->childs[0]->name = strdup("string1");
        vars->childs[0]->content = strdup("hello");

        vars->childs[1] = calloc(1, sizeof(struct html_tag));
        vars->childs[1]->name = strdup("string2");
        vars->childs[1]->content = strdup("hello");

        vars->childs[2] = calloc(1, sizeof(struct html_tag));
        vars->childs[2]->name = strdup("string3");
        vars->childs[2]->content = strdup("dog");

        vars->childs[3] = calloc(1, sizeof(struct html_tag));
        vars->childs[3]->name = strdup("string4");
        vars->childs[3]->content = strdup("cat");

        vars->childs_count = 4;

        int res = eval(input, vars);

        if (res != 1) {
            msg(ERROR,"String comparision test failed");
            failed++;
        } else {
            msg(INFO,"String comparision test PASSED");
        }
    }

    // Test 4: nested expressions
    {
        // should evaluate:
        // (1 + 1) * 2 -> 4
        char* input = "(num + 4) * 2";

        struct html_tag* vars = calloc(1, sizeof(struct html_tag));
        vars->name = strdup("vars");

        vars->childs = calloc(2, sizeof(struct html_tag*));
        vars->childs[0] = calloc(1, sizeof(struct html_tag));
        vars->childs[0]->name = strdup("num");
        vars->childs[0]->content = strdup("4");

        vars->childs_count = 1;

        int res = eval(input, vars);

        if (res != 16) {
            msg(ERROR,"Nested expressions test failed");
            failed++;
        } else {
            msg(INFO,"Nested expressions test PASSED");
        }
    }

    // Test 5: nested var acess (var#child)
    {
        // should evaluate:
        // var.child -> 2
        char* input = "var#child == 'bing'";

        struct html_tag* vars = calloc(1, sizeof(struct html_tag));
        vars->name = strdup("vars");

        vars->childs = calloc(1, sizeof(struct html_tag*));
        vars->childs[0] = calloc(1, sizeof(struct html_tag));
        vars->childs[0]->name = strdup("var");
        vars->childs[0]->content = strdup("bong");

        vars->childs[0]->childs = calloc(1, sizeof(struct html_tag*));
        vars->childs[0]->childs[0] = calloc(1, sizeof(struct html_tag));
        vars->childs[0]->childs[0]->name = strdup("child");
        vars->childs[0]->childs[0]->content = strdup("bing");
        vars->childs[0]->childs_count = 1;

        vars->childs_count = 1;

        int res = eval(input, vars);

        // res should evaluate to true
        if (res != 1) {
            msg(ERROR,"Nested var access test failed");
            failed++;
        } else {
            msg(INFO,"Nested var access test PASSED");
        }
    } 

    return failed;
}