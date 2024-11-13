#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/hcml.h"
#include "include/cutils.h"

int test_parser() {
    int failed = 0;

    // Test 1: Basic tag
    {
        char* input = "<html></html>";
        struct html_tag* result = html_parser(input);
        dbg(2,"Result: %p",result);
        dbg(2,"Result: %s",result->name);
        if (!result || strcmp(result->name, "html") != 0) {
            msg(ERROR,"Basic tag test failed");
            failed++;
        }
        dbg(INFO,"Basic tag test PASSED");

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
        }
        dbg(INFO,"Nested tags test PASSED");

        // free result
        destroy_html(result);
    }

    // Test 3: Attributes
    {
        char* input = "<div class=\"test\" id=\"myid\"></div>";
        struct html_tag* result = html_parser(input);
        if (!result || result->attributes_count != 2 ||
            strcmp(result->attributes[0]->name, "class") != 0) {
            msg(ERROR,"Attributes test failed");
            failed++;
        }
        dbg(INFO,"Attributes test PASSED");

        // free result
        destroy_html(result);
    }

    // Test 4: Malformed
    {
        char* input = "<div><p>Hello</div></p>";
        struct html_tag* result = html_parser(input);
        if (result != NULL) {
            msg(ERROR,"Malformed test failed");
            failed++;
        }
        msg(INFO,"Malformed test PASSED");
        // free result
        destroy_html(result);
    }

    printf("%s: %d tests failed\n", failed ? "FAIL" : "SUCCESS", failed);
    return failed;
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
        }
        msg(INFO,"Basic get_tag_by_id PASSED");

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
        }
        msg(INFO,"get_tags_by_name multiple tags PASSED");

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
        }
        msg(INFO,"Non-existent ID test PASSED");

        destroy_html(root);
    }

    // Test 4: Deep nesting search
    {
        char* input = "<div><section><article><p id=\"deep\">Found me!</p></article></section></div>";
        struct html_tag* root = html_parser(input);

        struct html_tag* found = get_tag_by_id(root, "deep");
        if (!found || strcmp(found->content, "Found me!") != 0) {
            msg(ERROR,"Deep nesting search failed");
            failed++;
        }
        msg(INFO,"Deep nesting search PASSED");

        destroy_html(root);
    }

    printf("Getter tests: %s (%d failed)\n", failed ? "FAIL" : "SUCCESS", failed);
    return failed;
}



int main() {

    DEBUG = 4;
    
    test_parser();
    test_getters();

    return 0;

}