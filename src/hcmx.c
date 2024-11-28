#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "unistd.h"
#include "stdio.h"
#include "dirent.h"

#include "../include/hcml.h"
#include "../include/cutils.h"
#include "../include/hcmx.h"
#include "../include/context.h"


char* http_method_str(enum http_method method) {
    switch (method) {
        case GET:
            return "GET";
        case POST:
            return "POST";
        case PUT:
            return "PUT";
        case DELETE:
            return "DELETE";
        default:
            return NULL;
    }
}

struct html_tag* htmx_loader() {
    // add a special HTMX loader
    struct html_tag* htmx_loader = calloc(1,sizeof(struct html_tag));
    htmx_loader->name = strdup("script");
    htmx_loader->attributes_count = 1;
    htmx_loader->attributes = calloc(1,sizeof(struct tag_attribute*));
    htmx_loader->attributes[0] = calloc(1,sizeof(struct tag_attribute));
    htmx_loader->attributes[0]->name = strdup("src");
    htmx_loader->attributes[0]->value = strdup("https://unpkg.com/htmx.org@2.0.3");
    htmx_loader->self_closing = 0;

    htmx_loader->childs_count = 0;
    htmx_loader->childs = NULL;

    return htmx_loader;
}


int get_hcmx(char* path,char** html_str,struct htmx_context** ctx) {

    // read the file
    char* file_content;
    int file_size = rdfile(path,&file_content);
    if (file_size < 0) {
        msg(ERROR,"Error: could not read file %s (%d)",path,file_size);
        return 1;
    }

    // parse the file content
    struct html_tag* html = html_parser(file_content);
    if (html == NULL) {
        msg(ERROR,"Error: could not parse file %s",path);
        return 1;
    }

    dbg(3,"Parsed component %s",html->name);

    // add the htmx loader
    struct html_tag* loader = htmx_loader();
    loader->parent = html;
    html->childs_count++;
    html->childs = realloc(html->childs,html->childs_count*sizeof(struct html_tag*));
    html->childs[html->childs_count-1] = loader;

    // process the htmx tags
    *ctx = process_htmx(html);

    // compile the hcml

    // get the cwd
    char cwd[1024];
    getcwd(cwd,1024);

    char hcmx_parent[1024];
    strcpy(hcmx_parent,path);
    char* last_slash = strrchr(hcmx_parent,'/');
    if (last_slash == NULL) {
        msg(ERROR,"Error: invalid path");
        return 1;
    }
    *last_slash = '\0';

    chdir(hcmx_parent);
    int compile_res = hcml_compile(html);
    if (compile_res != 0) {
        msg(ERROR,"Error: could not compile hcml");
        return 1;
    }
    chdir(cwd);

    // create the html code
    int code_size = create_html(html,html_str);
    if (code_size < 0) {
        msg(ERROR,"Error: could not create html code");
        return 1;
    }

    // free the html
    destroy_html(html);

    return 0;

}

struct html_tag* run_hcmx(struct http_request request, struct html_tag* hcmx_html, struct fn_entry* functions) {

    dbg(3,"Running HCMX");

    struct html_tag* vars;
    vars = calloc(1,sizeof(struct html_tag));
    vars->name = "vars";
    size_t vars_childs_alloc = 16;
    vars->childs_count = 0;
    vars->childs = malloc(vars_childs_alloc*sizeof(struct html_tag*));

    // set the request variables
    vars->childs[vars->childs_count] = calloc(1,sizeof(struct html_tag));
    vars->childs[vars->childs_count]->name = strdup("path");
    vars->childs[vars->childs_count]->content = strdup(request.path);
    vars->childs_count++;

    // set the method
    char* method = http_method_str(request.method);
    if (method == NULL) {
        msg(ERROR,"Error: invalid method");
        return NULL;
    }

    vars->childs[vars->childs_count] = calloc(1,sizeof(struct html_tag));
    vars->childs[vars->childs_count]->name = strdup("method");
    vars->childs[vars->childs_count]->content = strdup(method);
    vars->childs_count++;

    // add the body
    if (request.body != NULL) {
        vars->childs[vars->childs_count] = calloc(1,sizeof(struct html_tag));
        vars->childs[vars->childs_count]->name = strdup("body");
        vars->childs[vars->childs_count]->content = strdup(request.body);
        vars->childs_count++;
    }

    dbg(3,"Evaluating hcml tags");
    dbg(3,"Root: %s",hcmx_html->name);
    int eval_res = hcml_eval(hcmx_html,&vars,&vars_childs_alloc);
    if (eval_res != 0) {
        msg(ERROR,"Error: could not evaluate hcml");
        return NULL;
    }

    dbg(3,"HCMX done");

    return hcmx_html;
}


