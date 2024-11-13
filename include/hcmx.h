#pragma once

#include "stdlib.h"

typedef struct html_tag* (*handler_fn)(struct html_tag** args);

// Registration system
struct fn_entry {
    char* name;
    handler_fn fn;
};


// HCMX extension
// Hyper C Markup eXtension

enum http_method {
    GET,
    POST,
    PUT,
    DELETE
};

struct http_request {
    enum http_method method;
    char* path;
    char* body;
};

// Handler for HTMX tags
struct htmx_handler {

    char* path;
    enum http_method method;

    struct html_tag* hcmx_handler;
};

// Global context for an HCMX document
struct htmx_context {

    struct fn_entry* functions;

    struct htmx_handler** handlers;
    size_t count;
    size_t capacity;
};


// process HTMX tags and the extensiosn
struct htmx_context* process_htmx(struct html_tag* root);

// load and (get HCMX code)
int get_hcml(char* path,char** html_str,struct htmx_context* ctx);

struct html_tag* run_hcmx(struct http_request request,
                         struct html_tag* hcmx_html, 
                         struct fn_entry* functions
);