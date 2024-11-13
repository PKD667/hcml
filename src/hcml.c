// Hyper C markup language
// parser and utils for the hcml language

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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




struct html_tag* get_var(struct html_tag* vars,char* name) {
    for (int i = 0; i < vars->childs_count; i++) {
        if (strcmp(vars->childs[i]->name,name) == 0) {
            return vars->childs[i];
        }
    }
    return NULL;
}

int hcml_eval(struct html_tag* html,struct html_tag** vars) {

    if (html == NULL || html->name == NULL) {
        return 1; // invalid tag error
    } 

    if (html->name[0] == '?') {
        // we are in a hcml tag
        // we need to evaluate it
        if (strcmp(html->name,"?set") == 0) {
            // we are setting a variable
            // get the variable name
            char* var_name = get_attribute_value(html,"name");
            if (var_name == NULL) {
                msg(ERROR,"Error: no name attribute in ?set tag");
                return 2; // HCML error
            }

            // get the variable value
            char* var_value = html->content;
            if (var_value == NULL) {
                msg(ERROR,"Error: no content in ?set tag");
                return 1; // invalid tag error
            }

            (*vars)->childs_count++;
            (*vars)->childs = realloc((*vars)->childs,(*vars)->childs_count*sizeof(struct html_tag*));
            (*vars)->childs[(*vars)->childs_count-1]  = html;
        } 
        // Unfortunately the ?get part is awfully slow
        // We could fix that easily by using the CUtils hashmap
        // I'm lazy but I might do it later
        else if (strcmp(html->name,"?get") == 0) {

            

            // we are getting a variable
            // get the variable name
            char* var_name = get_attribute_value(html,"name");
            if (var_name == NULL) {
                msg(ERROR,"Error: no name attribute in ?get tag");
                return 1;
            }

            // get the variable value
            struct html_tag* var = get_var(*vars,var_name);
            if (var->content == NULL) {
                msg(ERROR,"Error: variable %s not found",var_name);
                return 1;
            }   
            
            if (html->childs_count == 0) {
                // replace html by var
                html->parent->childs[get_index(html)] = var;

            } else {
                // Field-specific get
                char* var_name = get_attribute_value(html, "name");
                struct html_tag* var = get_var(*vars, var_name);
                if (!var) {
                    msg(ERROR, "Variable %s not found", var_name);
                    return 1;
                }

                // We'll start by implementing one layer of depth
                for (int i = 0; i < html->childs_count; i++) {
                    struct html_tag* child = html->childs[i];

                    char* field_name = child->name;
                    if (!field_name) {
                        msg(ERROR, "Field tag must have a name attribute");
                        return 1;
                    }

                    struct html_tag* field = get_var(var, field_name);
                    if (!field) {
                        msg(ERROR, "Field %s not found in variable %s", field_name, var_name);
                        return 1;
                    }

                    // Replace the field tag with the actual value
                    html->childs[i] = field;
                }
            }



        } else if (strcmp(html->name,"?load") == 0) {
            // we are loading a file
            // get the file path
            char* file_path = get_attribute_value(html,"src");
            if (file_path == NULL) {
                msg(ERROR,"Error: no src attribute in ?load tag");
                return 1;
            }
        }
    } else {
        // loop over the childs
        for (int i = 0; i < html->childs_count; i++) {
            int eval_res = hcml_eval(html->childs[i],vars);
            if (eval_res != 0) {
                return eval_res;
            }
        }
    }

    return 0;

}

struct html_tag* hcml_load_file(char* path) {
    if (path == NULL) {
        msg(ERROR,"Error: no src attribute in ?load tag");
        return NULL;
    }

    char* file_content;
    int file_size = rdfile(path,&file_content);
    if (file_size < 0) {
        msg(ERROR,"Error: could not read file %s (%d)",path,file_size);
        return NULL;
    }

    // parse the file content
    struct html_tag* file_html = html_parser(file_content);
    if (file_html == NULL) {
        msg(ERROR,"Error: could not parse file %s",path);
        return NULL;
    }

    free(file_content);

    return file_html;
}

int hcml_compile(struct html_tag* html) {
    printf("getting code from %s\n",html->name);


    // if root that is hcml ERROR
    if (html->parent == NULL && html->name[0] == '?') {
        msg(ERROR,"Error: root tag cannot be a hcml tag");
        return 1;
    }
    
    // we are basically running an interpreter
    // we are going to loop over the tags

    // get all the load tags
    struct html_tag** load_tags = get_tags_by_name(html,"?load");
    // loop over the load tags
    for (int i = 0; load_tags[i]; i++) {
        // get the file path
        char* file_path = get_attribute_value(load_tags[i],"src");
        
        // load the file
        struct html_tag* file_html = hcml_load_file(file_path);

        // add the file content to the parent
        load_tags[i]->parent->childs[get_index(load_tags[i])] = file_html;

        // destroy the load tag
        destroy_html(load_tags[i]);
    }

    struct html_tag* vars;

    // evaluate the hcml tags
    int eval_res = hcml_eval(html,&vars);
    if (eval_res != 0) {
        msg(ERROR,"Error: could not evaluate hcml");
        return 1;
    }

    // dont free the vars

    return 0;
}

struct html_tag* hcml_set_create(const char* name,char* content) {
    // Create base tag
    struct html_tag* set = malloc(sizeof(struct html_tag));
    if (!set) return NULL;

    // Initialize basic fields
    set->name = strdup("?set");
    set->parent = NULL;
    set->childs = NULL;
    set->childs_count = 0;
    set->content = content ? strdup(content) : NULL;

    // Add name attribute
    set->attributes = malloc(sizeof(struct tag_attribute*));
    set->attributes[0] = malloc(sizeof(struct tag_attribute));
    set->attributes[0]->name = strdup("name");
    set->attributes[0]->value = strdup(name);
    set->attributes_count = 1;

    return set;
}

// Helper to add a child tag
void hcml_set_append(struct html_tag* set, const char* tag_name, const char* content) {
    struct html_tag* child = malloc(sizeof(struct html_tag));

    child->name = strdup(tag_name);
    child->content = content ? strdup(content) : NULL;
    child->attributes = NULL;
    child->attributes_count = 0;
    child->childs = NULL;
    child->childs_count = 0;
    child->parent = set;

    // Add to parent
    insert_tag(set, child, set->childs_count);
}



// HTMX

struct htmx_context* process_htmx(struct html_tag* root) {

    struct html_tag** on_tags = get_tags_by_name(root, "?hx-on");

    struct htmx_context* ctx = calloc(1, sizeof(struct htmx_context));
    ctx->capacity = 16;
    ctx->handlers = calloc(ctx->capacity, sizeof(struct htmx_handler*));

    for(int i = 0; on_tags[i]; i++) {
        struct html_tag* on_tag = on_tags[i];
        struct htmx_handler* handler = calloc(1, sizeof(struct htmx_handler));

        struct html_tag* htmx_tag = on_tags[i]->parent;
        if (!htmx_tag) {
            msg(ERROR, "hx-on tag must have a parent");
            free(handler);
            continue;
        }

        enum http_method method;
        if (strcmp(htmx_tag->name, "hx-get") == 0) {
            handler->method = GET;
        } else if (strcmp(htmx_tag->name, "hx-post") == 0) {
            method = POST;
        } else if (strcmp(htmx_tag->name, "hx-put") == 0) {
            method = PUT;
        } else if (strcmp(htmx_tag->name, "hx-delete") == 0) {
            method = DELETE;
        } else {
            msg(ERROR, "Invalid method %s", htmx_tag->name);
            free(handler);
            continue;
        }


        // Get path
        handler->path = get_attribute_value(htmx_tag, "hx-path");
        if (!handler->path) {
            msg(ERROR, "Missing hx-path attribute");
            free(handler);
            continue;
        }


        // Get code
        handler->hcmx_handler = on_tag;

        // Add handler to context
        if (ctx->count >= ctx->capacity) {
            ctx->capacity *= 2;
            ctx->handlers = realloc(ctx->handlers, ctx->capacity * sizeof(struct htmx_handler*));
        }
        ctx->handlers[ctx->count] = handler;
        ctx->count++;
    }


    return ctx;
}

