// Hyper C markup language
// parser and utils for the hcml language

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>


#include "../include/hcml.h"
#include "../include/cutils.h"
#include "../include/hcmx.h"
#include "../include/context.h"

struct html_tag* get_var(struct html_tag* vars, char* name) {
    dbg(3, "Searching for %s in %d vars", name, vars->childs_count);
    for (int i = 0; i < vars->childs_count; i++) {
        dbg(3, "Checking %s == %s", vars->childs[i]->name, name);
        if (strcmp(vars->childs[i]->name, name) == 0) {
            return vars->childs[i];
        }
    }
    return NULL;
}


// HCML variable are represented using the standard HTML tag structure. 
// Theoretically, the cariable are html objects with children
// When doing operation the variable is replaced by the content of the main parent tag

void print_vars(struct html_tag* vars) {
    for (int i = 0; i < vars->childs_count; i++) {
        printf("Var %s -> %d\n", vars->childs[i]->name, vars->childs[i]->childs_count);
    }
}
// Parse HCML <?set> tag
/*
    This function will process a set tag and append its HTML inner structure to the vars
*/
int hcml_parse_set(struct html_tag* set, struct html_tag** vars, size_t* vars_childs_alloc) {
    dbg(3, "Processing set tag");
    // we are setting a variable
    // get the variable name
    char* var_name = get_attribute_value(set, "id");
    dbg(3, "Var name: %s", var_name);
    if (var_name == NULL) {
        msg(ERROR, "Error: no id attribute in ?set tag");
        return 2;  // HCML error
    }

    char* type = get_attribute_value(set, "type");
    if (type == NULL) {
        type = DEFAULT_TAG_TYPE;
    }

    dbg(3, "Type: %s", type);
    set->name = strdup(var_name);
    set->attributes = malloc(sizeof(struct tag_attribute*));
    set->attributes[0] = malloc(sizeof(struct tag_attribute));
    set->attributes[0]->name = strdup("type");
    set->attributes[0]->value = strdup(type);
    set->attributes_count = 1;

    dbg(3, "Adding variable %s", var_name);
    if ((*vars)->childs_count >= (*vars_childs_alloc) - 1) {
        (*vars_childs_alloc) *= 2;
        (*vars)->childs = realloc((*vars)->childs, (*vars_childs_alloc) * sizeof(struct html_tag*));
    }
    (*vars)->childs[(*vars)->childs_count] = set;
    (*vars)->childs_count++;
    dbg(3, "Variable added");
    print_vars(*vars);

    // delete the set tag
    pop_tag(set);

    return 0;
}

// Parse HCML <?get> tag
/*
    This function will process a get tag and replace it with the variable value
*/
int hcml_parse_get(struct html_tag* get, struct html_tag** vars, size_t* vars_childs_alloc) {
    dbg(3, "Getting variable");
    print_vars(*vars);
    // we are getting a variable
    // get the variable name
    char* var_name = get_attribute_value(get, "id");
    if (var_name == NULL) {
        msg(ERROR, "Error: no id attribute in ?get tag");
        return 1;
    }

    // get the variable value
    struct html_tag* var = get_var(*vars, var_name);
    if (var == NULL) {
        msg(ERROR, "Error: variable %s not found", var_name);
        return 1;
    }
    dbg(3, "Variable %s found", var_name);

    // get the type
    char* type = get_attribute_value(var, "type");
    if (type == NULL) {
        type = DEFAULT_TAG_TYPE;
    }

    var->attributes[0]->name = strdup("id");
    var->attributes[0]->value = strdup(var_name);

    var->name = strdup(type);

    if (get->childs_count == 0) {
        // replace html by var
        get->parent->childs[get_index(get)] = var;

    } else if (get->childs_count == 1) {
        // We'll start by implementing one layer of depth
        // TODO: Implement multiple layers of depth
        struct html_tag* child = get->childs[0];

        char* field_name = child->name;
        if (!field_name) {
            msg(ERROR, "Child tag is weird");
            return 1;
        }

        struct html_tag* field = get_var(var, field_name);
        if (!field) {
            msg(ERROR, "Field %s not found in variable %s", field_name, var_name);
            return 1;
        }

        // Replace the field tag with the actual value
        get->parent->childs[get_index(get)] = field;

    } else {
        msg(ERROR, "Multiple childs");
        return 1;
    }

    return 0;
}

// Parse HCML <?load> tag
int hcml_parse_load(struct html_tag* load, struct html_tag** vars, size_t* vars_childs_alloc) {
    // get the file path
    char* file_path = get_attribute_value(load, "src");
    if (file_path == NULL) {
        msg(ERROR, "Error: no src attribute in ?load tag");
        return 1;
    }

    char* file_content;
    int file_size = rdfile(file_path, &file_content);
    if (file_size < 0) {
        msg(ERROR, "Error: could not read file %s (%d)", file_path, file_size);
        return 1;
    }

    // parse the file content
    struct html_tag* file_html = html_parser(file_content);
    if (file_html == NULL) {
        msg(ERROR, "Error: could not parse file %s", file_path);
        free(file_content);
        return 1;
    }

    free(file_content);

    // add the file content to the parent
    load->parent->childs[get_index(load)] = file_html;

    // destroy the load tag
    destroy_html(load);

    return 0;
}

int hcml_parse_if(struct html_tag* if_tag, struct html_tag** vars, size_t* vars_childs_alloc) {
    // get the condition
    char* condition = get_attribute_value(if_tag, "cond");
    if (condition == NULL) {
        msg(ERROR, "Error: no cond attribute in ?if tag");
        return 1;
    }

    // evaluares the condition
    int result = eval(condition, *vars);

    if (!result) {
        // remove the if tag
        dbg(3, "Condition %s is false", condition);

        if (if_tag->parent->childs_count > 1 && 
            strcmp(if_tag->parent->childs[get_index(if_tag) + 1]->name,"?else") == 0) {
            
            dbg(3, "Else tag found");

            // the else tag is executed
            struct html_tag* else_tag = if_tag->parent->childs[get_index(if_tag) + 1];
            // search for type
            char* type = get_attribute_value(else_tag, "type");
            if (type == NULL) {
                type = DEFAULT_TAG_TYPE;
            }
            free(else_tag->name);
            else_tag->name = strdup(type);

            dbg(3, "Else tag good");
        }

        pop_tag(if_tag);
        destroy_html(if_tag);

        return 0;

    } 
    dbg(3, "Condition %s is true", condition);

    // check for a type
    dbg(3, "Checking for type");
    char* type = get_attribute_value(if_tag, "type");
    if (type == NULL) {
        type = DEFAULT_TAG_TYPE;
    }

    // replace the if tag with a div
    free(if_tag->name);
    if_tag->name = strdup(type);

    // remove the condition attribute
    for (int i = 0; i < if_tag->attributes_count; i++) {
        if (strcmp(if_tag->attributes[i]->name, "cond") == 0) {
            free(if_tag->attributes[i]->name);
            free(if_tag->attributes[i]->value);
            free(if_tag->attributes[i]);
            if_tag->attributes_count--;
            for (int j = i; j < if_tag->attributes_count; j++) {
                if_tag->attributes[j] = if_tag->attributes[j + 1];
            }
            break;
        }
    }

    dbg(3,"Checking for else tag");

    if (if_tag->parent->childs_count > get_index(if_tag) && 
            strcmp(if_tag->parent->childs[get_index(if_tag) + 1]->name,"?else") == 0) {
            
        dbg(3, "Removing else tag");
        // REMOVE THE ELSE TAG
        struct html_tag* else_tag = if_tag->parent->childs[get_index(if_tag) + 1];
        dbg(3,"Else tag is %s", else_tag->name);
        remove_tag(else_tag);
        dbg(3,"Else tag removed");

    }


    return 0;
}

int hcml_parse_eval(struct html_tag* eval, struct html_tag** vars, size_t* vars_childs_alloc) {
    dbg(3, "Processing eval tag");
    // Evaluate the inner content
    hcml_eval(eval, vars, vars_childs_alloc);

    // For simplicity, we'll just replace the eval tag with its content rendered as a string
    char* content = html_to_string(eval);

    // Create a new text node with the evaluated content
    struct html_tag* text_node = calloc(1, sizeof(struct html_tag));
    text_node->name = NULL;  // Text node
    text_node->content = strdup(content);

    // Replace the eval tag with the text node in the parent's child list
    eval->parent->childs[get_index(eval)] = text_node;

    // Free the eval tag
    destroy_html(eval);
    free(content);

    return 0;
}

struct html_tag* get_fn(struct html_tag* fns, char* name) {
    for (int i = 0; i < fns->childs_count; i++) {
        if (strcmp(fns->childs[i]->name, name) == 0) {
            return fns->childs[i];
        }
    }
    return NULL;
}


int hcml_parse_call(struct html_tag* call, struct html_tag** vars, size_t* vars_childs_alloc) {
    dbg(3, "Processing call tag");
    // Get the function name
    char* fn_name = get_attribute_value(call, "id");
    if (fn_name == NULL) {
        msg(ERROR, "Error: no id attribute in ?call tag");
        return 1;
    }

    // Get the function from vars
    struct html_tag* fns = get_var(*vars, "functions");
    if (fns == NULL) {
        msg(ERROR, "Error: no functions defined");
        return 1;
    }

    struct html_tag* fn = get_fn(fns, fn_name);
    if (fn == NULL) {
        msg(ERROR, "Error: function %s not found", fn_name);
        return 1;
    }   

    // the fn content is the pointer to the function
    hcml_fn fn_ptr = (hcml_fn)fn->content;

    // call the function
    if (fn_ptr == NULL) {
        msg(ERROR, "Error: function %s is NULL", fn_name);
        return 1;
    }

    struct html_tag* res = fn_ptr(call);

    dbg(3, "Got result %s from function %s", res->name, fn_name);
    dbg(3,"Result content: %s", res->content);

    // replace the call tag with the result
    call->parent->childs[get_index(call)] = res;

    destroy_html(call);

    dbg(3, "Call tag processed");
    return 0;
}

void* hcml_funcs[][2] = {
    {"?set", hcml_parse_set},
    {"?get", hcml_parse_get},
    {"?load", hcml_parse_load},
    {"?if", hcml_parse_if},
    {"?eval", hcml_parse_eval},
    {"?call", hcml_parse_call}
};



int hcml_eval(struct html_tag* html, struct html_tag** vars, size_t* vars_childs_alloc) {
    if (html == NULL) {
        dbg(3, "NULL tag");
        return 1;  // invalid tag error
    }

    if (html->name && html->name[0] == '?') {
        dbg(3, "Processing hcml tag %s", html->name);
        // loop over the functions
        int found = 0;
        for (int i = 0; i < sizeof(hcml_funcs) / sizeof(hcml_funcs[0]); i++) {
            if (strcmp(html->name, hcml_funcs[i][0]) == 0) {
                // call the function
                int (*fn)(struct html_tag*, struct html_tag**, size_t*) = hcml_funcs[i][1];
                dbg(3, "Calling processing function for %s tag", html->name);
                int res = fn(html, vars, vars_childs_alloc);
                dbg(3, "Processing function returned %d", res);
                if (res != 0) {
                    return res;
                }
                found = 1;
                dbg(3, "HCML tag done");
                break;
            }
        }
        if (!found) {
            msg(ERROR, "Error: Unknown hcml tag %s", html->name);
            return 1;
        }
    } else {

        // our goal is to evaluate ALL the tags in childs, but there are a few problems
        // 1. Sometimes, a tag can get removed, meaning an index isn't enough
        // 2. Sometimes, a tag can get replaced.
        // 3. Sometimes, a tag can get added.

        // the new approach will consist in keeping track of the evaluated tags in an array
        // make sure all the childq are in the array

        int index = 0;
        struct html_tag* current_tag;
        while (index < html->childs_count) {

            dbg(3, "Evaluating child %d", index);


            current_tag = html->childs[index];

            // evaluate the tag
            hcml_eval(current_tag, vars, vars_childs_alloc);

            // check for changes in the structure
            if (html->childs[index] != current_tag) {
                // the tag has been replaced 
                // we'll keep the index the same
                continue;
            }

            index++;
        }
        

    }

    dbg(3, "HCML tag evaluated");
    return 0;
}





int hcml_compile(struct html_tag* html) {
    printf("Compiling code from %s\n", html->name ? html->name : "root");

    // if root that is hcml ERROR
    if (html->parent == NULL && html->name && html->name[0] == '?') {
        msg(ERROR, "Error: root tag cannot be a hcml tag");
        return 1;
    }

    struct html_tag* vars;
    vars = calloc(1, sizeof(struct html_tag));
    vars->name = strdup("vars");

    size_t vars_childs_alloc = 16;
    vars->childs_count = 1;
    vars->childs = malloc(vars_childs_alloc * sizeof(struct html_tag*));

    // init default functions
    struct html_tag* fns = get_stdlib();
    // other stuff
    vars->childs[0] = fns;


    // evaluate the hcml tags
    dbg(3, "Evaluating hcml tags");
    int eval_res = hcml_eval(html, &vars, &vars_childs_alloc);
    if (eval_res != 0) {
        msg(ERROR, "Error: could not evaluate hcml");
        return 1;
    }

    // don't free the vars
    dbg(3, "HCML compiling done");
    return 0;
}

// HTMX

struct htmx_context* process_htmx(struct html_tag* root) {
    struct html_tag** on_tags = get_tags_by_name(root, "?hx-on");

    struct htmx_context* ctx = calloc(1, sizeof(struct htmx_context));
    ctx->handlers_capacity = 16;
    ctx->handlers = calloc(ctx->handlers_capacity, sizeof(struct htmx_handler*));

    for (int i = 0; on_tags[i]; i++) {
        struct html_tag* on_tag = on_tags[i];
        struct htmx_handler* handler = calloc(1, sizeof(struct htmx_handler));

        struct html_tag* htmx_tag = on_tags[i]->parent;
        if (!htmx_tag) {
            msg(ERROR, "hx-on tag must have a parent");
            free(handler);
            continue;
        }

        char* val;

        enum http_method method;
        if ((val = get_attribute_value(htmx_tag, "hx-get"))) {
            method = GET;
        } else if ((val = get_attribute_value(htmx_tag, "hx-post"))) {
            method = POST;
        } else if ((val = get_attribute_value(htmx_tag, "hx-put"))) {
            method = PUT;
        } else if ((val = get_attribute_value(htmx_tag, "hx-delete"))) {
            method = DELETE;
        } else {
            msg(ERROR, "Missing method attribute");
            free(handler);
            continue;
        }
        dbg(3, "Method: %s", http_method_str(method));

        // Get path
        handler->path = strdup(val);
        if (!handler->path) {
            msg(ERROR, "Missing hx-path attribute");
            free(handler);
            continue;
        }
        dbg(3, "Path: %s", handler->path);

        // Get code
        handler->hcmx_handler = on_tag;

        char* type;
        type = get_attribute_value(htmx_tag, "type");
        if (type == NULL) {
            type = DEFAULT_TAG_TYPE;
        }
        on_tag->name = strdup(type);

        // Add handler to context
        if (ctx->handlers_count >= ctx->handlers_capacity) {
            ctx->handlers_capacity *= 2;
            ctx->handlers = realloc(ctx->handlers, ctx->handlers_capacity * sizeof(struct htmx_handler*));
        }
        ctx->handlers[ctx->handlers_count] = handler;
        ctx->handlers_count++;

        // Remove the hx-on tag
        pop_tag(on_tag);
    }

    return ctx;
}