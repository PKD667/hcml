#pragma once

#include "stdlib.h"

#include "context.h"


// HCMX extension
// Hyper C Markup eXtension




// process HTMX tags and the extensiosn
struct htmx_context* process_htmx(struct html_tag* root);

// load and (get HCMX code)
int get_hcmx(char* path,char** html_str,struct htmx_context** ctx);

struct html_tag* run_hcmx(struct http_request request,
                         struct html_tag* hcmx_html, 
                         struct fn_entry* functions
);

// get the method as a string
char* http_method_str(enum http_method method);
