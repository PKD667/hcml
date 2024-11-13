#pragma once

#include "stdlib.h"

struct tag_attribute {
    char *name;
    char *value;
};

struct html_tag {
    char* name;
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

// Getters  
struct html_tag* get_tag_by_id(struct html_tag* html,char* id);
struct html_tag** get_tags_by_name(struct html_tag* html,char* tag_name);
char* get_attribute_value(struct html_tag* html,char* attribute_name);

// Free
int destroy_html(struct html_tag* html);


// HCML specific
// Hyper C Markup Language
int hcml_compile(struct html_tag* html);

struct html_tag* hcml_set_create(const char* name,char* content);
void hcml_set_append(struct html_tag* set, const char* tag_name, const char* content);