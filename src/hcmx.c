#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

#include "../include/hcml.h"
#include "../include/cutils.h"
#include "../include/hcmx.h"




struct html_tag* run_hcmx(struct http_request request, struct html_tag* hcmx_html, struct fn_entry* functions) {

    // add a <?set> tag with the client request
    struct html_tag* set_tag = hcml_set_create("request",NULL);
    hcml_set_append(set_tag,"body", request.body);
    hcml_set_append(set_tag,"path", request.path);
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
    hcml_set_append(set_tag,"method", method);



    // insert the tag at the beginning
    insert_tag(hcmx_html,set_tag,0);


    // compile the hcml
    int compile_res = hcml_compile(hcmx_html);
    if (compile_res != 0) {
        msg(ERROR,"Error: could not compile hcml");
        return NULL;
    }

    // run functions
    struct html_tag** fn_tags = get_tags_by_name(hcmx_html,"?do");
    
    for (int i = 0; fn_tags[i]; i++) {
        struct html_tag* fn_tag = fn_tags[i];
        char* fn_name = get_attribute_value(fn_tag,"fn");
        if (fn_name == NULL) {
            msg(ERROR,"Error: ?do tag must have a name attribute");
            return NULL;
        }

        // find the function
        handler_fn fn = NULL;
        for (int j = 0; functions[j].name; j++) {
            if (strcmp(functions[j].name,fn_name) == 0) {
                fn = functions[j].fn;
                break;
            }
        }

        if (fn == NULL) {
            msg(ERROR,"Error: function %s not found",fn_name);
            return NULL;
        }

        // run the function
        struct html_tag* res = fn(fn_tag->childs);
        if (res == NULL) {
            msg(ERROR,"Error: function %s returned NULL",fn_name);
            return NULL;
        }

        fn_tag->parent->childs[get_index(fn_tag)] = res;
    }

    // find the <?return> tag
    struct html_tag** return_tag = get_tags_by_name(hcmx_html,"?return");
    if (return_tag == NULL) {
        msg(WARNING,"Error: no ?return tag found using empty string");
        // its not allowed to not have a return tag
        return NULL;
    }

    if (return_tag[1] != NULL) {
        msg(ERROR,"Error: multiple ?return tags found");
        msg(INFO,"Returning the first one");
    }

    return return_tag[0];
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
