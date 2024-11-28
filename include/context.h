#pragma once

#include "stdio.h"
#include "stdlib.h"




enum http_method {
    GET,
    POST,
    PUT,
    DELETE
};

struct http_header {
    char* name;
    char* value;
};

struct http_request {
    enum http_method method;
    char* path;

    size_t headers_count;
    struct http_header* headers;

    char* body;
};

struct http_response {
    int status;

    size_t headers_count;
    struct http_header* headers;
    
    char* content;
};

// Handler for HTMX tags
struct htmx_handler {

    char* path;
    enum http_method method;

    struct html_tag* hcmx_handler;
};

typedef struct html_tag* (*handler_fn)(struct html_tag** args);

// Registration system
struct fn_entry {
    char* name;
    handler_fn fn;
};

// Global context for an HCMX document
struct htmx_context {

    struct fn_entry* functions;
    size_t functions_count;
    size_t functions_capacity;

    struct htmx_handler** handlers;
    size_t handlers_count;
    size_t handlers_capacity;
};

// add a handler to the context
int htmx_ctx_add_handler(struct htmx_context* ctx, struct htmx_handler* handler);

// add a function to the context
int htmx_ctx_add_function(struct htmx_context* ctx, struct fn_entry fn);

// get a handler from the context
struct htmx_handler* htmx_ctx_get_handler(struct htmx_context* ctx, struct http_request* request);

// get a function from the context
struct fn_entry htmx_ctx_get_function(struct htmx_context* ctx, char* name);

// concatenate two contexts
int htmx_ctx_merge(struct htmx_context* dest, struct htmx_context* other);

// create a new context
struct htmx_context* htmx_ctx_create();

// destroy a context
int htmx_ctx_destroy(struct htmx_context* ctx);


// server stuff

struct server_context {
    int port;
    char* web_root;

    // endpoints
    struct htmx_context* htmx_ctx;
};

// add a file to the server context
int srv_ctx_add_file(struct server_context* ctx, struct web_file file);

// create a new server context
struct server_context* srv_ctx_create(int port, char* web_root);

// get a file from the server context
struct web_file* srv_ctx_get_file(struct server_context* ctx, char* path);

// destroy a server context
int srv_ctx_destroy(struct server_context* ctx);

//visualize the server context
void srv_ctx_visualize(struct server_context* ctx);




