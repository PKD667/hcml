/*
 BASE HTML PARSER TEST SUITE
*/

#include "test.h"

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

        char* code;
        create_html(result,&code);
        printf("Code: %s\n",code);

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

