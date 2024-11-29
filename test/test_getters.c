
#include "test.h"

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

