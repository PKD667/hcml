
#include "test.h"

// UTILS FOR TEST

struct html_tag* hcml_test_fn(struct html_tag* tag) {

    if (tag == NULL) {
        return NULL;
    }

    if (strcmp(tag->content,"arg") != 0) {
        printf("Error: invalid content\n");
        return NULL;
    }

    // create a new tag
    struct html_tag* new_tag = calloc(1,sizeof(struct html_tag));
    new_tag->name = strdup("p");
    new_tag->content = strdup("Hello");
    return new_tag;
}


/*
 SPECIAL HCML TESTS
*/

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

    // Test 5: call
    {
        char* input = "<html> \
                        <?set id=\"fn\" type=\"div\"> \
                            <p>bing</p> \
                        </?set> \
                        <?call id=\"fn\">arg</?call> \
                       </html>";
        struct html_tag* root = html_parser(input);

        struct html_tag* vars = calloc(1, sizeof(struct html_tag));
        vars->name = strdup("vars");

        printf("Allocating vars\n");

        size_t vars_childs_alloc = 16;
        vars->childs_count = 1;
        vars->childs = calloc(vars_childs_alloc,sizeof(struct html_tag*));

        printf("Allocating functions\n");

        // init default functions
        struct html_tag* fns = calloc(1, sizeof(struct html_tag));
        fns->name = strdup("functions");
        fns->childs_count = 1;

        printf("Allocating test fn\n");

        fns->childs = calloc(2, sizeof(struct html_tag*));
        fns->childs[0] = calloc(1, sizeof(struct html_tag));
        fns->childs[0]->name = strdup("fn");
        fns->childs[0]->content = (void*)hcml_test_fn;

        // other stuff
        vars->childs[0] = fns;

        // evaluate the hcml tags
        printf("Evaluating hcml tags\n");
        int eval_res = hcml_eval(root, &vars, &vars_childs_alloc);
        printf("Eval res: %d\n",eval_res);
        if (eval_res != 0 ||
            strcmp(root->childs[0]->content, "Hello") != 0
        ) {
            msg(ERROR,"HCML call test failed");
            printf("Content: %s\n", root->childs[0]->content);
            failed++;
        } else {
            msg(INFO,"HCML call test PASSED");
        }

        char* compiled;
        int size = create_html(root, &compiled);
        printf("Compiled: %s\n", compiled);
    }

    return failed;
}