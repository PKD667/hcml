#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

#include "../include/hcml.h"
#include "../include/cutils.h"
#include "../include/hcmx.h"


int get_hcmx(char* path,char** html_str,struct htmx_context* ctx) {

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

    // process the htmx tags
    ctx = process_htmx(html);

    // compile the hcml
    int compile_res = hcml_compile(html);
    if (compile_res != 0) {
        msg(ERROR,"Error: could not compile hcml");
        return 1;
    }

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

    struct html_tag* vars;
    vars = calloc(1,sizeof(struct html_tag));
    vars->name = "vars";
    size_t vars_childs_alloc = 16;
    vars->childs_count = 0;
    vars->childs = malloc(vars_childs_alloc*sizeof(struct html_tag*));

    // set the request variables
    vars->childs[vars->childs_count] = calloc(1,sizeof(struct html_tag));
    vars->childs[vars->childs_count]->name = "path";
    vars->childs[vars->childs_count]->content = strdup(request.path);
    vars->childs_count++;

    vars->childs[vars->childs_count] = calloc(1,sizeof(struct html_tag));
    vars->childs[vars->childs_count]->name = "method"; 
    char* method = NULL;
    switch (request.method) {
        case GET:
            method = "GET";
            break;
        case POST:
            method = "POST";
            break;
        case PUT:
            method = "PUT";
            break;
        case DELETE:
            method = "DELETE";
            break;
    }
    vars->childs[vars->childs_count]->content = strdup(method);
    vars->childs_count++;

    // add the body
    vars->childs[vars->childs_count] = calloc(1,sizeof(struct html_tag));
    vars->childs[vars->childs_count]->name = "body";
    vars->childs[vars->childs_count]->content = strdup(request.body);
    vars->childs_count++;

    dbg(3,"Evaluating hcml tags");
    int eval_res = hcml_eval(hcmx_html,&vars,&vars_childs_alloc);
    if (eval_res != 0) {
        msg(ERROR,"Error: could not evaluate hcml");
        return NULL;
    }

    return hcmx_html;
}



char* handle_request(struct http_request request, struct htmx_context* ctx) {
    msg(INFO,"Request: %s",request);
    
    struct htmx_handler* handler = NULL;
    for (int i = 0; i < ctx->count; i++) {
        if (strcmp(ctx->handlers[i]->path,request.path) == 0 && ctx->handlers[i]->method == request.method) {
            handler = ctx->handlers[i];
            break;
        }
    }

    if (handler == NULL) {
        msg(ERROR,"Error: no handler found for %s %s",request.method,request.path);
        return NULL;
    }

    struct html_tag* result = run_hcmx(request,handler->hcmx_handler,ctx->functions);

    if (result == NULL) {
        msg(ERROR,"Error: could not run hcmx");
        return NULL;
    }

    char* code;
    int code_size = create_html(result,&code);

    if (code_size < 0) {
        msg(ERROR,"Error: could not create html code");
        return NULL;
    }

    return code;

}
