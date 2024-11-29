#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "test.h"





void* test_funcs[][2] = {
    {"parser", test_parser},
    {"getters", test_getters},
    {"eval", test_eval},
    {"hcml", test_hcml}
};


// RUN FILE

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

    assert(root != NULL);

    if (hcml_compile(root) != 0) {
        msg(ERROR,"Error: could not compile new root");
        return 1;
    }

    printf("Root: %p\n",root);

    char* new_html = NULL;
    int size = create_html(root, &new_html);

    printf("New html: %s\n", new_html);

    assert(strcmp(root->name, "html") == 0);

    // write to file
    int wr = wrnfile(outfile,new_html,size);
    if (wr < 0) {
        msg(ERROR,"Error: could not write file %s (%d)","test2.html",wr);
        return 1;
    }

    destroy_html(root);

    return 0;
}




int main(int argc, char** argv) {

    DEBUG = 4;

    if (argc < 2) {
        msg(ERROR,"Usage: %s [test|run] <unit>",argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "test") == 0) {
        
        if (argc < 3) {
            msg(ERROR,"Usage: %s test <unit>",argv[0]);
            return 1;
        }

        if (strcmp(argv[2], "all") == 0) {
            int failed = 0;
            for (int i = 0; i < sizeof(test_funcs) / sizeof(test_funcs[0]); i++) {
                int (*fn)() = test_funcs[i][1];
                failed += fn();
            }

            if (failed) {
                msg(FATAL,"%d tests failed",failed);
            }

            return failed;
        } else {
            for (int i = 0; i < sizeof(test_funcs) / sizeof(test_funcs[0]); i++) {
                if (strcmp(test_funcs[i][0], argv[2]) == 0) {
                    int (*fn)() = test_funcs[i][1];
                    int failed = fn();
                    if (failed) {
                        msg(FATAL,"%d tests faild for %s",failed,argv[2]);
                    }

                    return failed;
                }
            }

            msg(ERROR,"Unknown test unit %s",argv[2]);
            return 1;
        }
    } else if (strcmp(argv[1], "run") == 0) {

        if (argc < 3) {
            msg(ERROR,"Usage: %s run <infile> [outfile]",argv[0]);
            return 1;
        }

        if (argc == 3) {
            return test_file(argv[2], "out.html");
        }

        return test_file(argv[2], argv[3]);
    } else {
        msg(ERROR,"Unknown command %s",argv[1]);
        return 1;

    }
    
    
    return 0;

}