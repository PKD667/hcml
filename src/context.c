#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../include/context.h"
#include "../include/cutils.h"
#include "../include/hcml.h"
#include "../include/hcmx.h"



// HTMX context

int htmx_ctx_add_handler(struct htmx_context* ctx, struct htmx_handler* handler) {

    dbg(3,"Adding handler for %s",handler->path);
    char* code;
    create_html(handler->hcmx_handler,&code);
    dbg(3,"Handler code: %s",code);

    if (ctx->handlers_count + 1 >= ctx->handlers_capacity) {
        ctx->handlers_capacity *= 2;
        ctx->handlers = realloc(ctx->handlers,ctx->handlers_capacity*sizeof(struct htmx_handler*));
    }

    ctx->handlers[ctx->handlers_count] = handler;
    ctx->handlers_count++;

    return 0;
}

int htmx_ctx_add_function(struct htmx_context* ctx, struct fn_entry fn) {

    dbg(3,"Adding function %s",fn.name);

    if (ctx->functions_count + 1 >= ctx->functions_capacity) {
        ctx->functions_capacity *= 2;
        ctx->functions = realloc(ctx->functions,ctx->functions_capacity*sizeof(struct fn_entry*));
    }

    ctx->functions[ctx->functions_count] = fn;
    ctx->functions_count++;

    return 0;
}

struct htmx_context* htmx_ctx_create() {
    struct htmx_context* ctx = calloc(1,sizeof(struct htmx_context));
    ctx->handlers_capacity = 16;
    ctx->handlers = calloc(ctx->handlers_capacity,sizeof(struct htmx_handler*));

    ctx->functions_capacity = 16;
    ctx->functions = calloc(ctx->functions_capacity,sizeof(struct fn_entry*));


    return ctx;
}

struct htmx_handler* htmx_ctx_get_handler(struct htmx_context* ctx, struct http_request* request) {

    bool found_path = false;

    for (int i = 0; i < ctx->handlers_count; i++) {
        struct htmx_handler* handler = ctx->handlers[i];
        dbg(3,"Checking handler %s %s",handler->path,http_method_str(handler->method));
        if (strcmp(handler->path,request->path) == 0) {
            if (handler->method == request->method) {
                dbg(3,"Found handler for %s",request->path);
                return handler;
            }

            found_path = true;
        }
    }

    msg(ERROR,"Error: no handler found for %s",request->path);
    msg(ERROR,"Found handler for path: %s",found_path ? "yes" : "no");
    return NULL;
}

struct fn_entry htmx_ctx_get_function(struct htmx_context* ctx, char* name) {
    for (int i = 0; i < ctx->functions_count; i++) {
        if (strcmp(ctx->functions[i].name,name) == 0) {
            return ctx->functions[i];
        }
    }
    return (struct fn_entry){NULL,NULL};
}

int htmx_ctx_pop_handler(struct htmx_context* ctx, struct htmx_handler* handler) {
    for (int i = 0; i < ctx->handlers_count; i++) {
        if (ctx->handlers[i] == handler) {
            for (int j = i; j < ctx->handlers_count - 1; j++) {
                ctx->handlers[j] = ctx->handlers[j + 1];
            }
            ctx->handlers_count--;
            return 0;
        }
    }
    return 1;
}

int htmx_ctx_merge(struct htmx_context* dest, struct htmx_context* other) {
    
    // add handlers to dest 
    // if handler already exists update it
    for (int i = 0; i < other->handlers_count; i++) {
        struct htmx_handler* handler = other->handlers[i];

        struct http_request req = {handler->method,handler->path};

        struct htmx_handler* dest_handler = htmx_ctx_get_handler(dest,&req);
        if (dest_handler != NULL) {
            htmx_ctx_pop_handler(dest,dest_handler);
            free(dest_handler->path);
            free(dest_handler);
        }
        htmx_ctx_add_handler(dest,handler);
    }

    return 0;
}


int htmx_ctx_destroy(struct htmx_context* ctx) {
    for (int i = 0; i < ctx->handlers_count; i++) {
        free(ctx->handlers[i]);
    }
    free(ctx->handlers);
    free(ctx);

    return 0;
}



struct server_context* srv_ctx_create(int port, char* web_root) {
    struct server_context* ctx = calloc(1, sizeof(struct server_context));
    ctx->port = port;
    ctx->web_root = strdup(web_root);

    ctx->htmx_ctx = htmx_ctx_create();

    return ctx;
}

int srv_ctx_destroy(struct server_context* ctx) {

    htmx_ctx_destroy(ctx->htmx_ctx);

    free(ctx->web_root);
    free(ctx);

    return 0;
}

void srv_ctx_visualize(struct server_context* ctx) {
    printf("Server context:\n");
    printf("Port: %d\n", ctx->port);
    printf("Web root: %s\n", ctx->web_root);
    printf("HTMX context:\n");
    printf("  Handlers - %lu:\n", ctx->htmx_ctx->handlers_count);
    for (size_t i = 0; i < ctx->htmx_ctx->handlers_count; i++) {
        printf("    %s %s\n", http_method_str(ctx->htmx_ctx->handlers[i]->method), ctx->htmx_ctx->handlers[i]->path);
    }
    printf("  Functions - %lu:\n", ctx->htmx_ctx->functions_count);
    for (size_t i = 0; i < ctx->htmx_ctx->functions_count; i++) {
        printf("    %s\n", ctx->htmx_ctx->functions[i].name);
    }

    return;
}