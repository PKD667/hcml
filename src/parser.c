/*
    This is a very basic HTML parser,
    It is designed to not be strict and to be able to parse invalid HTML,
    It is designed to be used with the hcml language,
    It is not designed to be used as a general purpose HTML parser

    Copyright (C) 2024 PKD
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/hcml.h"
#include "../include/cutils.h"



struct html_tag* parse_tag(char* html_string,struct html_tag* current_tag,int* i) {
    //allocate a new tag
    dbg(3,"opening tag");

    // check for comments
    if (strncmp(html_string+*i,"<!--",4) == 0) {
        //skip the comment
        while (strncmp(html_string+*i,"-->",3) != 0) {
            (*i)++;
        }
        return current_tag;
    }

    struct html_tag* new_tag = calloc(1,sizeof(struct html_tag));
    //set the parent
    new_tag->parent = current_tag;
    if (new_tag->parent != NULL)
    {
        //add the new tag to the childs of the current tag
        current_tag->childs_count++;
        current_tag->childs = realloc(current_tag->childs,current_tag->childs_count*sizeof(struct html_tag*));
        current_tag->childs[current_tag->childs_count-1] = new_tag;
    }
    //set the new tag as the current tag
    current_tag = new_tag;

    //we are in a opening tag
    //allocate the name
    int name_alloc = 64;
    current_tag->name = calloc(name_alloc,sizeof(char));
    //copy the name
    int j = 0;
    (*i)++;
    while (html_string[*i] != ' ' && html_string[*i] != '>' && html_string[*i] != '/') {

        if (j >= name_alloc) {
            name_alloc *= 2;
            current_tag->name = realloc(current_tag->name,name_alloc);
        }

        current_tag->name[j] = html_string[*i];
        (*i)++;
        j++;
    }

    dbg(3,"tag name is %s",current_tag->name);

    // skip whitespaces
    while (html_string[*i] == ' ' || html_string[*i] == '/') {

        (*i)++;
    }


    dbg(3,"parsing attributes");
    while (html_string[*i] != '>') {

        // parse attribute
        dbg(3,"parsing attribute");
        //allocate a new attribute
        current_tag->attributes = realloc(current_tag->attributes,(current_tag->attributes_count+1)*sizeof(struct tag_attribute*));
        current_tag->attributes[current_tag->attributes_count] = calloc(1,sizeof(struct tag_attribute));
        //allocate the name
        int attribute_name_alloc = 64;
        current_tag->attributes[current_tag->attributes_count]->name = calloc(64,sizeof(char));
        //copy the name
        int j = 0;
        while (html_string[*i] != '=') {

            if (j >= attribute_name_alloc) {
                attribute_name_alloc *= 2;
                current_tag->attributes[current_tag->attributes_count]->name = realloc(current_tag->attributes[current_tag->attributes_count]->name,attribute_name_alloc);
            }

            current_tag->attributes[current_tag->attributes_count]->name[j] = html_string[*i];
            (*i)++;
            j++;
        }
        dbg(3,"attribute name is %s",current_tag->attributes[current_tag->attributes_count]->name);
        //skip the '='
        (*i)++;

        //allocate the value
        int attribute_value_alloc = 64;
        current_tag->attributes[current_tag->attributes_count]->value = calloc(64,sizeof(char));
        //copy the value
        j = 0;
        // remove the quotes
        (*i)++;
        while (html_string[*i] != '"') {
            
            if (j >= attribute_value_alloc) {
                attribute_value_alloc *= 2;
                current_tag->attributes[current_tag->attributes_count]->value = realloc(current_tag->attributes[current_tag->attributes_count]->value,attribute_value_alloc);
            }

            current_tag->attributes[current_tag->attributes_count]->value[j] = html_string[*i];
            (*i)++;
            j++;
        }
        dbg(3,"attribute value is %s",current_tag->attributes[current_tag->attributes_count]->value);
        //increment the attributes count
        current_tag->attributes_count++;

        //skip the closing quote
        (*i)++;

        // skip withespaces
        while (html_string[*i] == ' ' || html_string[*i] == '/') {
            (*i)++;
        }

    }

    return current_tag;
}

struct html_tag* html_parser(char* html_string) {

    // check for doctype
    if (strncmp(html_string,"<!DOCTYPE",9) == 0) {
        //skip the doctype
        int i = 9;
        while (html_string[i] != '>') {
            i++;
        }

        html_string = html_string+i+1;
    }

    // check if the string is empty
    if (html_string == NULL || strlen(html_string) == 0) {
        msg(ERROR,"Error: empty string");
        return NULL;
    }

    int open_tags_alloc = 16;
    struct html_tag** open_tags = calloc(open_tags_alloc,sizeof(struct html_tag*));
    int open_tags_count = 0;

    dbg(2,"Parsing %s",html_string);
    struct html_tag* current_tag = NULL;
    
    int in_tag = 0;
    for (int i = 0; i < strlen(html_string); i++)
    {
        dbg(3,"c: %c , string: %s , in->%s",html_string[i],in_tag ? "tag" : "content",current_tag ? current_tag->name : "NULL");

        // skip any whitespace or newlines
        if (html_string[i] == ' ' || html_string[i] == '\n' || html_string[i] == '\r') {
            continue;
        }

        if (html_string[i] == '<') {
            //we are in a tag
            dbg(3,"tag start");
            in_tag = 1;
            //check if we are closing a tag
            if (html_string[i+1] == '/') {

                // check if the closing tag is valid
                dbg(3,"Checking %s == %.*s",current_tag->name,(int)strlen(current_tag->name),html_string+i+2);
                if (strncmp(
                    html_string+i+2,
                    current_tag->name,
                    strlen(current_tag->name)
                ) == 0) {
                    
                    dbg(3,"closing tag is valid");
                    current_tag->self_closing = 0;

                    //print skipped chars
                    dbg(3,"skipping : %.*s",(int)strlen(current_tag->name)+2,html_string+i);
                    i += strlen(current_tag->name)+2;
                    //the closing tag is valid
                    //check if the closing tag is the root tag
                    if (current_tag->parent == NULL) {
                        //we are closing the root tag
                        //we are done parsing
                        dbg(3,"closing root tag");
                        // remove from open tags
                        open_tags_count--;
                        break;
                    } else {
                        //we are closing a child tag
                        //set the current tag to the parent tag
                        
                        // if the last open ttag isnt the tag we are closing
                        // Then we have a problem (html is invalid)
                        if (open_tags[open_tags_count-1] != current_tag) {
                            msg(ERROR,"Error: Tag conflicting with higher open tag",current_tag->name);
                            return NULL;
                        } 

                        open_tags_count--;

                        current_tag = current_tag->parent;
                        dbg(3,"current_tag: %p",current_tag);
                    }

                } else {
                    //the closing tag is invalid
                    msg(ERROR,"Error: invalid closing tag");
                    return NULL;
                }
            } else {
                //we are opening a tag
                current_tag = parse_tag(html_string,current_tag,&i);
                if (current_tag == NULL) {
                    //error parsing tag
                    msg(ERROR,"Error parsing tag");
                    return NULL;
                }

                // if is a self closing tag
                if (html_string[i-1] == '/') {
                    dbg(3,"self closing tag");

                    current_tag->self_closing = 1;

                    // if root tag then exit
                    if (current_tag->parent == NULL) {
                        // self closing root tag
                        dbg(3,"self closing root tag");
                        break;
                    }

                    current_tag = current_tag->parent;
                    continue;
                }

                //check if we need to reallocate the open tags array
                if (open_tags_count >= open_tags_alloc) {
                    open_tags_alloc *= 2;
                    open_tags = realloc(open_tags,open_tags_alloc*sizeof(struct html_tag*));
                }

                //add the tag to the open tags
                dbg(3,"adding tag %s to open tags",current_tag->name);
                open_tags[open_tags_count] = current_tag;
                open_tags_count++;
            }
        } else {
                //we are in a tag content
                //allocate the content
                // TODO: find a better aloocating system
                int content_alloc = 256;
                current_tag->content = calloc(content_alloc,sizeof(char));
                //copy the content
                int j = 0;
                while (html_string[i] != '<') {
                    
                    if (j >= content_alloc) {
                        content_alloc *= 2;
                        current_tag->content = realloc(current_tag->content,content_alloc);
                    }

                    current_tag->content[j] = html_string[i];
                    i++;
                    j++;
                }
                //go back one character
                i--;
        }
    }
    dbg(3,"parsing done");

    // cjeck for unclosed tags
    if (open_tags_count > 0) {
        msg(ERROR,"Error: unclosed tags");
        return NULL;
    }

    dbg(3,"open tags count: %d",open_tags_count);
    free(open_tags);
    dbg(3,"open tags freed");
    
    dbg(3,"returning %p",current_tag);
    
    return current_tag;
}

int destroy_html(struct html_tag* html) {

    dbg(3, "Destroying html %s", html->name);

    if (html == NULL)
    {
        return 0;
    }

    // destroy the html
    // recursively loop over the tags
    // and free the memory
    // free the attributes
    // free the tag
    for (int i = 0; i < html->childs_count; i++) {
        if (html->childs[i] != NULL) destroy_html(html->childs[i]);
    }
    // free the attributes
    for (int i = 0; i < html->attributes_count; i++)
    {
        if (html->attributes[i] != NULL) {
            dbg(3, "Freeing attribute %s=%s", html->attributes[i]->name, html->attributes[i]->value);
            if (html->attributes[i]->name != NULL) free(html->attributes[i]->name);
            if (html->attributes[i]->value != NULL) free(html->attributes[i]->value);
            dbg(3, "Freeing attribute");
            free(html->attributes[i]);
        }
    }
    // free the tag
    if (html->name != NULL) free(html->name);
    if (html->content != NULL) free(html->content);
    if (html->attributes != NULL) free(html->attributes);
    if (html->childs != NULL) free(html->childs);
    free(html);
    return 0;
}


int create_html(struct html_tag* html, char** code) {

    dbg(3, "Creating html code for %s", html->name);

    int code_alloc = strlen(html->name) + 512;
    dbg(3, "Allocating %d bytes", code_alloc);
    *code = calloc(code_alloc, sizeof(char));
    if (*code == NULL) {
        msg(ERROR, "Error: Allocation failed");
        return -1;
    }
    int code_len = 0;

    dbg(3, "Allocated %d bytes", code_alloc);

    // Add the opening tag
    int required_size = code_len + strlen("<") + strlen(html->name) + 1;
    if (required_size >= code_alloc) {
        code_alloc = required_size + 512;
        *code = realloc(*code, code_alloc);
        if (*code == NULL) {
            msg(ERROR, "Error: Reallocation failed");
            return -1;
        }
    }
    strcat(*code, "<");
    strcat(*code, html->name);
    code_len = required_size - 1;

    // Add attributes
    dbg(3, "Adding %d attributes", html->attributes_count);
    for (int i = 0; i < html->attributes_count; i++) {

        dbg(3, "Adding attribute %s=%s", html->attributes[i]->name, html->attributes[i]->value);

        required_size = code_len + strlen(" ") + strlen(html->attributes[i]->name) + strlen("=\"") + strlen(html->attributes[i]->value) + strlen("\"") + 1;
        if (required_size >= code_alloc) {
            code_alloc = required_size + 512;
            *code = realloc(*code, code_alloc);
            if (*code == NULL) {
                msg(ERROR, "Error: Reallocation failed");
                return -1;
            }
        }

        strcat(*code, " ");
        strcat(*code, html->attributes[i]->name);
        strcat(*code, "=\"");
        strcat(*code, html->attributes[i]->value);
        strcat(*code, "\"");
        code_len = required_size - 1;
    }

    // Close the opening tag
    required_size = code_len + 2;
    if (required_size >= code_alloc) {
        code_alloc = required_size + 512;
        *code = realloc(*code, code_alloc);
        if (*code == NULL) {
            msg(ERROR, "Error: Reallocation failed");
            return -1;
        }
    }

    if (html->self_closing) {
        strcat(*code, "/>");
        code_len = required_size;

        if (html->content != NULL) {
            msg(WARNING, "Warning: Self closing tag %s has content", html->name);
        } 

        if (html->childs_count > 0) {
            msg(WARNING, "Warning: Self closing tag %s has child tags", html->name);
        }

        return code_len;
    } 

    strcat(*code, ">");
    code_len = required_size - 1;

    // Add content if any
    if (html->content != NULL) {
        dbg(3, "Adding content %s", html->content);
        required_size = code_len + strlen(html->content) + 1;
        if (required_size >= code_alloc) {
            code_alloc = required_size + 512;
            *code = realloc(*code, code_alloc);
            if (*code == NULL) {
                msg(ERROR, "Error: Reallocation failed");
                return -1;
            }
        }
        strcat(*code, html->content);
        code_len = required_size - 1;
    }

    // Add the child tags recursively
    for (int i = 0; i < html->childs_count; i++) {

        char* child_code = NULL;
        int child_size = create_html(html->childs[i], &child_code);
        if (child_size < 0) {
            msg(ERROR, "Error: Failed to create child HTML");
            return -1;
        }

        required_size = code_len + child_size + 1;
        if (required_size >= code_alloc) {
            code_alloc = required_size + 512;
            *code = realloc(*code, code_alloc);
            if (*code == NULL) {
                msg(ERROR, "Error: Reallocation failed");
                free(child_code);
                return -1;
            }
        }

        strcat(*code, child_code);
        code_len = required_size - 1;

        free(child_code);
    }

    // Add the closing tag
    required_size = code_len + strlen("</") + strlen(html->name) + strlen(">") + 1;
    if (required_size >= code_alloc) {
        code_alloc = required_size + 512;
        *code = realloc(*code, code_alloc);
        if (*code == NULL) {
            msg(ERROR, "Error: Reallocation failed");
            return -1;
        }
    }
    strcat(*code, "</");
    strcat(*code, html->name);
    strcat(*code, ">");
    code_len = required_size - 1;

    dbg(3, "HTML code created");

    return code_len;
}