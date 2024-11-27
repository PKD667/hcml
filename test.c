#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "include/hcml.h"
#include "include/cutils.h"
#include "include/hcmx.h"

/*
 BASE HTML PARSER TEST SUITE
*/

int test_parser() {
    int failed = 0;

    {
        // Test 1: Basic tag
        char* input = "<html></html>\n";
        printf("TESTING PARSER\n");
        printf("Input: %s",input);
        struct html_tag* result = html_parser(input);
        printf("Result: %p\n",result);
        printf("Result: %s\n",result->name);
        if (!result || strcmp(result->name, "html") != 0) {
            msg(ERROR,"Basic tag test failed");
            failed++;
        } else {
            msg(INFO,"Basic tag test PASSED");
        }

        // free result
        destroy_html(result);
    }

    // Test 2: Nested tags
    {
        char* input = "<div><p>Hello</p></div>";
        struct html_tag* result = html_parser(input);
        if (!result || result->childs_count != 1 ||
            strcmp(result->childs[0]->content, "Hello") != 0) {
            msg(ERROR,"Nested tags test failed");
            failed++;
        } else {
            msg(INFO,"Nested tags test PASSED");
        }

        // free result
        destroy_html(result);
    }

    // Test 3: Attributes
    {
        char* input = "<div class=\"test\" id=\"myid\" ></div>";
        struct html_tag* result = html_parser(input);
        if (!result || result->attributes_count != 2 ||
            strcmp(result->attributes[0]->name, "class") != 0) {
            msg(ERROR,"Attributes test failed");

            printf("Result: %p\n",result);
            printf("Attr name: '%s'\n",result->attributes[0]->name);
            printf("Attr value: '%s'\n",result->attributes[0]->value);

            printf("Attr name: '%s'\n",result->attributes[1]->name);
            printf("Attr value: '%s'\n",result->attributes[1]->value);


            failed++;
        } else {
            msg(INFO,"Attributes test PASSED");
        }

        // free result
        destroy_html(result);
    }

    // Test 4: Malformed
    {
        char* input = "<div><p>Hello</div></p>";
        struct html_tag* result = html_parser(input);
        if (result != NULL) {
            msg(ERROR,"Malformed test failed");
            char* code;
            create_html(result,&code);
            printf("Result: %p\n",result);
            printf("Result: %s\n",result->name);
            printf("Result: %s\n",code);
            failed++;
        } else {
            msg(INFO,"Malformed test PASSED");
        }
        
        // free result
        destroy_html(result);
    }

    // test for tag with content
    {
        char* input = "<div>hello</div>";
        struct html_tag* result = html_parser(input);
        if (!result || strcmp(result->content, "hello") != 0) {
            msg(ERROR,"Tag with content test FAILED");
            failed++;
        } else {
            msg(INFO,"Tag with content test PASSED");
        }

        // free result
        destroy_html(result);
    }

    // Test 5: Self-closing tag
    {
        char* input = "<br/>";
        struct html_tag* result = html_parser(input);
        if (!result || strcmp(result->name, "br") != 0 || result->childs_count != 0) {
            msg(ERROR,"Self-closing tag test FAILED");
            failed++;
        } else {
            msg(INFO,"Self-closing tag test PASSED");
        }

        // free result
        destroy_html(result);
    }

    // Test 6: Comment handling
    {
        char* input = "<!-- This is a comment -->";
        struct html_tag* result = html_parser(input);
        if (result != NULL) {
            msg(ERROR,"Comment test FAILED");
            failed++;
            // free result
            destroy_html(result);
        } else {
            msg(INFO,"Comment test PASSED");
        }
    }

    // Test 7: Doctype declaration
    {
        char* input = "<!DOCTYPE html>";
        struct html_tag* result = html_parser(input);
        if (result != NULL) {
            msg(ERROR,"Doctype test FAILED");
            failed++;
            // free result
            destroy_html(result);
        } else {
            msg(INFO,"Doctype test PASSED");
        }
    }

    // Test 8: Unclosed tag
    {
        char* input = "<div><p>Hello";
        struct html_tag* result = html_parser(input);
        if (result != NULL) {
            msg(ERROR,"Unclosed tag test FAILED");
            failed++;
            // free result
            destroy_html(result);
        } else {
            msg(INFO,"Unclosed tag test PASSED");
        }
    }

    // Test 9: Wrongly nested tags
    {
        char* input = "<b><i>Text</b></i>";
        struct html_tag* result = html_parser(input);
        if (result != NULL) {
            msg(ERROR,"Wrongly nested tags test FAILED");
            failed++;
            // free result
            destroy_html(result);
        } else {
            msg(INFO,"Wrongly nested tags test PASSED");
        }
    }

    printf("%s: %d tests failed\n", failed ? "FAIL" : "SUCCESS", failed);
    return failed;
}



int test_file(char* infile, char* outfile) {

    
    char *file_content;
    long file_size = rdfile(infile,&file_content);

    if (file_size < 0) {
        msg(ERROR,"Error: could not read file %s (%d)","test_file",file_size);
        return 1;
    }

    struct html_tag* root = html_parser(file_content);

    assert(root != NULL);
    assert(strcmp(root->name, "html") == 0);

    // create_html(root, &file_content);

    char* new_html = NULL;
    int size = create_html(root, &new_html);

    printf("New html: %s\n", new_html);

    // re parse

    struct html_tag* new_root = html_parser(new_html);

    assert(new_root != NULL);

    assert(strcmp(new_root->name, "html") == 0);

    // write to file
    int wr = wrnfile(outfile,new_html,size);
    if (wr < 0) {
        msg(ERROR,"Error: could not write file %s (%d)","test2.html",wr);
        return 1;
    }

    destroy_html(root);
    destroy_html(new_root);

    return 0;
}



int test_getters() {
    int failed = 0;

    // Test 1: Basic get by ID
    {
        char* input = "<div id=\"test1\"><p id=\"test2\">Hello</p></div>";
        struct html_tag* root = html_parser(input);

        struct html_tag* found = get_tag_by_id(root, "test2");
        if (!found || strcmp(found->content, "Hello") != 0) {
            msg(ERROR,"Basic get_tag_by_id failed");
            failed++;
        } else {
            msg(INFO,"Basic get_tag_by_id PASSED");
        }

        destroy_html(root);
    }

    // Test 2: Multiple tags with same name
    {
        char* input = "<div><p>First</p><p>Second</p></div>";
        struct html_tag* root = html_parser(input);

        struct html_tag** found = get_tags_by_name(root, "p");
        if (!found || !found[0] || !found[1] || found[2] != NULL ||
            strcmp(found[0]->content, "First") != 0) {
            msg(ERROR,"get_tags_by_name multiple tags failed");
            failed++;
        } else {
            msg(INFO,"get_tags_by_name multiple tags PASSED");
        }

        free(found);  // Don't forget this!
        destroy_html(root);
    }

    // Test 3: Non-existent ID
    {
        char* input = "<div><p>Test</p></div>";
        struct html_tag* root = html_parser(input);

        struct html_tag* found = get_tag_by_id(root, "nonexistent");
        if (found != NULL) {
            msg(ERROR,"Non-existent ID test failed");
            failed++;
        } else {
            msg(INFO,"Non-existent ID test PASSED");
        }

        destroy_html(root);
    }

    // Test 4: Deep nesting search
    {
        char* input = "<div><section><article><p id=\"deep\">Found me!</p></article></section></div>";
        struct html_tag* root = html_parser(input);

        struct html_tag* found = get_tag_by_id(root, "deep");
        if (!found || strcmp(found->content, "Found me!") != 0) {
            msg(ERROR,"Deep nesting search failed");
            printf("Found: %p\n", found);
            printf("Found: %s\n", found->content);
            failed++;
        } else {
            msg(INFO,"Deep nesting search PASSED");
        }
        

        destroy_html(root);
    }

    printf("Getter tests: %s (%d failed)\n", failed ? "FAIL" : "SUCCESS", failed);
    return failed;
}

/*
 SPECIAL HCML TESTS
*/


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

// HCML tests

int test_hcml() {
    int failed = 0;

    // Test 1: HCML parsing 
    {
        char* input = "<html><?set id=\"test\">HELLO</?set></html>";
        struct html_tag* root = html_parser(input);

        int res = hcml_compile(root);
        if (res != 0) {
            msg(ERROR,"HCML parsing test failed");
            failed++;
        } else {
            msg(INFO,"HCML parsing test PASSED");
        }
    }

    // Test 2: set and get
    {
        char* input = "<html><?set id=\"test\" type=\"div\">Hello</?set><?get id=\"test\"></?get></html>";
        struct html_tag* root = html_parser(input);
        printf("Root: %p\n",root);
        printf("Childs: %d\n",root->childs_count);
        int res = hcml_compile(root);

        if (res != 0) {
            msg(ERROR,"HCML set and get test failed");
            failed++;
        } else {
            msg(INFO,"HCML set and get test PASSED");
        }

        char* compiled;
        int size = create_html(root, &compiled);
        printf("Compiled: %s\n", compiled);
    }

    // Test 3: nested set and get
    {
        char* input = "<html><?set id=\"var\"><in> In variable </in></?set><?get id=\"var\"><in/></?get></html>";
        struct html_tag* root = html_parser(input);
        int res = hcml_compile(root);

        if (res != 0 || 
            strcmp(root->childs[0]->name, "in") != 0 ||
            strcmp(root->childs[0]->content, "In variable ") != 0
        ) {
            msg(ERROR,"HCML nested set and get test failed");
            failed++;
        } else {
            msg(INFO,"HCML nested set and get test PASSED");
        }

        char* compiled;
        int size = create_html(root, &compiled);
        printf("Compiled: %s\n", compiled);
    }

    // Test 4: if
    {
        char* input = "<html> \
                        <?set id=\"var\">l</?set> \
                        <?if cond=\"var == 'l'\" type=\"div\" > \
                            <p>bing</p> \
                        </?if> \
                        <?else> \
                            <p>bong</p> \
                        </?else> \
                       </html>";
        struct html_tag* root = html_parser(input);
        int res = hcml_compile(root);

        if (res != 0 || 
            strcmp(root->childs[0]->childs[0]->content, "bing") != 0
        ) {
            msg(ERROR,"HCML if test failed");
            printf("Content: %s\n", root->childs[0]->childs[0]->content);
            failed++;
        } else {
            msg(INFO,"HCML if test PASSED");
        }

        char* compiled;
        int size = create_html(root, &compiled);
        printf("Compiled: %s\n", compiled);
    }

    return failed;
}




int main() {

    DEBUG = 4;

    
    int parser_f = test_parser();
    if (parser_f) {
        msg(ERROR,"%d parser tests failed", parser_f);
    }
    
    int getter_f = test_getters();
    if (getter_f) {
        msg(ERROR,"%d getter tests failed", getter_f);
    }

    /*
        ** HCML tests **
    */

    // Components Tests :

    // -> EVAL 
    int eval_f = test_eval();
    if (eval_f) {
        msg(ERROR,"%d EVAL tests failed", eval_f);
    }

    // Complete HCML tests

    int hcml_f = test_hcml();
    if (hcml_f) {
        msg(ERROR,"%d HCML tests failed", hcml_f);
    }

    return 0;

}