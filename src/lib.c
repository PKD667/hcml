// HCML FUNCS STDLIB

#include "../include/hcml.h"
#include "../include/cutils.h"

#include <time.h>

// DEFAULT FUNCTIONS
struct html_tag* hcml_fn_time(struct html_tag* tag) {
    struct html_tag* res = calloc(1, sizeof(struct html_tag));
    res->name = strdup("time");
    res->content = calloc(64, sizeof(char));
    time_t t = time(NULL);
    
    struct tm tm = *localtime(&t);
    sprintf(res->content, "%d-%02d-%02d %02d:%02d:%02d", 
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec
    );

    printf( "Time: %s\n", res->content);

    return res;
}



void* hcml_funcs_stdlib[][2] = {
    {"time", hcml_fn_time}
};


struct html_tag* get_stdlib() {
    struct html_tag* functions = calloc(1, sizeof(struct html_tag));
    functions->name = strdup("functions");
    functions->childs_count = sizeof(hcml_funcs_stdlib) / sizeof(hcml_funcs_stdlib[0]);
    functions->childs = calloc(functions->childs_count, sizeof(struct html_tag*));

    for (int i = 0; i < functions->childs_count; i++) {
        functions->childs[i] = calloc(1, sizeof(struct html_tag));
        functions->childs[i]->name = strdup(hcml_funcs_stdlib[i][0]);
        functions->childs[i]->content = (void*)hcml_funcs_stdlib[i][1];
    }

    return functions;
}