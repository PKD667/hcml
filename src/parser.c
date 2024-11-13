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
    while (html_string[*i] != ' ' && html_string[*i] != '>') {

        if (j >= name_alloc) {
            name_alloc *= 2;
            current_tag->name = realloc(current_tag->name,name_alloc);
        }

        current_tag->name[j] = html_string[*i];
        (*i)++;
        j++;
    }

    dbg(3,"tag name is %s",current_tag->name);


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
    }

    return current_tag;
}

struct html_tag* html_parser(char* html_string)
{
    int open_tags_alloc = 16;
    struct html_tag** open_tags = calloc(open_tags_alloc,sizeof(struct html_tag*));
    int open_tags_count = 0;

    dbg(2,"Parsing %s",html_string);
    struct html_tag* current_tag = NULL;
    
    int in_tag = 0;
    for (int i = 0; i < strlen(html_string); i++)
    {
        dbg(3,"c: %c , string: %s , in->%s",html_string[i],in_tag ? "tag" : "content",current_tag ? current_tag->name : "NULL");


        if (html_string[i] == '<') {
            //we are in a tag
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

                    //print skipped chars
                    dbg(3,"skipping : %.*s",(int)strlen(current_tag->name)+2,html_string+i);
                    i += strlen(current_tag->name)+2;
                    //the closing tag is valid
                    //check if the closing tag is the root tag
                    if (current_tag->parent == NULL) {
                        //we are closing the root tag
                        //we are done parsing
                        dbg(3,"closing root tag");
                        break;
                    } else {
                        //we are closing a child tag
                        //set the current tag to the parent tag
                        
                        // remove from the open tags
                        open_tags_count--;

                        current_tag = current_tag->parent;
                        dbg(3,"current_tag: %p",current_tag);
                    }

                } else {
                    // closing tag is not the same as the current tag
                    // assumme its a standalone tag

                    dbg(3,"Found standalone tag");

                    // skip the closing dash
                    i++;

                    current_tag = parse_tag(html_string,current_tag,&i);
                    if (current_tag == NULL) {
                        //error parsing tag
                        msg(ERROR,"Error parsing tag");
                    }

                    // check if current tag is in open tags
                    int is_open = 0;
                    for (int j = 0; j < open_tags_count; j++) {
                        if (open_tags[j]->name == current_tag->name) {
                            is_open = 1;
                            break;
                        }
                    }

                    if (is_open) {
                        // the tag is not open
                        // we are closing it
                        msg(ERROR,"Error: Tag conflicting with higher open tag",current_tag->name);
                        return NULL;
                    }

                    // since its standalone we are closing it
                    current_tag = current_tag->parent;
                }
            } else {
                //we are opening a tag
                current_tag = parse_tag(html_string,current_tag,&i);
                if (current_tag == NULL) {
                    //error parsing tag
                    msg(ERROR,"Error parsing tag");
                    return NULL;
                }

                //check if we need to reallocate the open tags array
                if (open_tags_count >= open_tags_alloc) {
                    open_tags_alloc *= 2;
                    open_tags = realloc(open_tags,open_tags_alloc*sizeof(struct html_tag*));
                }

                //add the tag to the open tags
                open_tags[open_tags_count] = current_tag;
                open_tags_count++;
            }
        } else if (html_string[i] == '>') {
            //we are out of a tag
            dbg(3,"out of tag");
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
    msg(INFO,"Parsing done");

    dbg(3,"open tags count: %d",open_tags_count);
    free(open_tags);
    dbg(3,"open tags freed");
    
    dbg(3,"returning %p",current_tag);
    return current_tag;
}

int destroy_html(struct html_tag* html)
{
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
            if (html->attributes[i]->name != NULL) free(html->attributes[i]->name);
            if (html->attributes[i]->value != NULL) free(html->attributes[i]->value);
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

int create_html(struct html_tag* html,char** code) {

    printf("creating html for %s\n",html->name);

    int code_alloc = strlen(html->name)+512;
    *code = calloc(code_alloc,sizeof(char));
    int code_len = 0;

    // reursively loop over tags 
    // and create the html code
    // add the tag name
    code_len += strlen(html->name)+1;
    strcat(*code,"<");
    strcat(*code,html->name);

    // add atributes
    for (int i = 0; i < html->attributes_count; i++) {

        code_len += strlen(html->attributes[i]->name)+strlen(html->attributes[i]->value)+4;

        if (code_len >= code_alloc - 16) {
            code_alloc *= 2;
            *code = realloc(*code,code_alloc);
        }

        strcat(*code," ");
        strcat(*code,html->attributes[i]->name);
        strcat(*code,"=\"");
        strcat(*code,html->attributes[i]->value);
        strcat(*code,"\"");

        
    }

    code_len++;
    strcat(*code,">");
    

    
    code_len += strlen(html->content);
    if (code_len  >= code_alloc - 16 ) {
        code_alloc *= 2;
        *code = realloc(*code,code_alloc);
    }
    // add the content
    strcat(*code,html->content);

    

    // add the childs
    for (int i = 0; i < html->childs_count; i++) {
        
        char* child_code = NULL;
        int child_size = create_html(html->childs[i],&child_code);

        code_len += child_size;

        if (code_len >= code_alloc - 16) {
            code_alloc *= 2;
            *code = realloc(*code,code_alloc);
        }

        strcat(*code,child_code);

        free(child_code);
    }

    code_len += strlen(html->name)+3;
    if (code_len >= code_alloc - 16) {
        code_alloc *= 2;
        *code = realloc(*code,code_alloc);
    }

    // add the closing tag
    strcat(*code,"</");
    strcat(*code,html->name);
    strcat(*code,">");
    return code_len;
}