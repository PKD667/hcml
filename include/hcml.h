#pragma once

#include "stdlib.h"
#include "stdbool.h"

struct tag_attribute {
    char *name;
    char *value;
};

struct html_tag {
    char* name;

    bool self_closing;

    struct tag_attribute** attributes;
    int attributes_count;

    struct html_tag* parent;

    struct html_tag** childs;
    int childs_count;

    char* content;
};

// Parsing
struct html_tag* html_parser(char* html_string);
int create_html(struct html_tag* html,char** code);

// parse util
int remove_tag(struct html_tag* html);
int get_index(struct html_tag* html) ;
int insert_tag(struct html_tag* parent, struct html_tag* child, int index);
int pop_tag(struct html_tag* tag);
char* html_to_string(struct html_tag* html);

// Getters  
struct html_tag* get_tag_by_id(struct html_tag* html,char* id);
struct html_tag** get_tags_by_name(struct html_tag* html,char* tag_name);
char* get_attribute_value(struct html_tag* html,char* attribute_name);

// Free
int destroy_html(struct html_tag* html);


// HCML specific
// Hyper C Markup Language
int hcml_compile(struct html_tag* html);
int hcml_eval(struct html_tag* html,struct html_tag** vars,size_t* vars_childs_alloc);

// EXPR evaluation
int math_eval(char* expr);

