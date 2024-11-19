
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"

#include "../include/hcml.h"
#include "../include/cutils.h"

int get_index(struct html_tag* html) {
    // get the index of a tag
    // recursively loop over the tags
    // and return the index
    if (html->parent == NULL) {
        return -2;
    }
    for (int i = 0; i < html->parent->childs_count; i++) {
        if (html->parent->childs[i] == html) {
            return i;
        }
    }
    return -1;
}

bool matches_suffix_wildcard(const char* pattern, const char* str) {
    size_t pattern_len = strlen(pattern);
    if (pattern[pattern_len - 1] != '*')
        return strcmp(pattern, str) == 0;

    // Remove the * and just compare the prefix
    return strncmp(pattern, str, pattern_len - 1) == 0;
}

struct html_tag** get_tags_by_name(struct html_tag* html,char* tag_name) {
    
    unsigned int capacity = 16;
    struct html_tag** tags = calloc(16,sizeof(struct html_tag*));
    unsigned int count = 0;

    for (int i = 0; i < html->childs_count; i++) {

        // Check if the tag name matches or if it has a wildcard(*)
        // only works for suffix wildcards
        if (matches_suffix_wildcard(tag_name, html->childs[i]->name)) {
            if (count >= capacity) {
                capacity *= 2;
                tags = realloc(tags,capacity*sizeof(struct html_tag*));
            }
            tags[count] = html->childs[i];
            count++;
        } else {
            struct html_tag** found_child_tags = get_tags_by_name(html->childs[i],tag_name);
            for (int j = 0; found_child_tags[j]; j++) {
                if (count >= capacity) {
                    capacity *= 2;
                    tags = realloc(tags,capacity*sizeof(struct html_tag*));
                }
                tags[count] = found_child_tags[j];
                count++;
            }
            free(found_child_tags);
        }
    }

    return tags;
    
}

struct html_tag* get_tag_by_id(struct html_tag* html,char* id)
{
    // get a tag by id
    // recursively loop over the tags
    // and return the tag with the id
    if (html->attributes_count > 0)
    {
        for (int i = 0; i < html->attributes_count; i++)
        {
            if (strcmp(html->attributes[i]->name,"id") == 0)
            {
                if (strcmp(html->attributes[i]->value,id) == 0)
                {
                    return html;
                }
            }
        }
    }
    for (int i = 0; i < html->childs_count; i++)
    {
        struct html_tag* tag = get_tag_by_id(html->childs[i],id);
        if (tag != NULL)
        {
            return tag;
        }
    }
    return NULL;
}

char* get_attribute_value(struct html_tag* html,char* attribute_name)
{
    // get the value of an attribute
    // recursively loop over the tags
    // and return the value of the attribute
    if (html->attributes_count > 0)
    {
        for (int i = 0; i < html->attributes_count; i++)
        {
            if (strcmp(html->attributes[i]->name,attribute_name) == 0)
            {
                return html->attributes[i]->value;
            }
        }
    }
    for (int i = 0; i < html->childs_count; i++)
    {
        char* value = get_attribute_value(html->childs[i],attribute_name);
        if (value != NULL) {
            return value;
        }
    }
    return NULL;
}

int remove_tag(struct html_tag* html) {
    if (!html || !html->parent) return -1;

    // First find our index in parent's array
    int idx = -1;
    for (int i = 0; i < html->parent->childs_count; i++) {
        if (html->parent->childs[i] == html) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return -1;  // Not found in parent?!

    // Shift remaining elements left
    for (int i = idx; i < html->parent->childs_count - 1; i++) {
        html->parent->childs[i] = html->parent->childs[i + 1];
    }

    // NOW decrease count
    html->parent->childs_count--;

    // Then proceed with cleanup
    destroy_html(html);

    return 0;
}

int insert_tag(struct html_tag* parent, struct html_tag* child, int index) {
    if (!parent || !child || index < 0 || index > parent->childs_count) return -1;

    // First, resize the parent's array
    struct html_tag** temp = realloc(parent->childs, (parent->childs_count + 1) * sizeof(struct html_tag*));
    if (!temp) return -1; // Handle allocation failure
    parent->childs = temp;

    // Shift elements to the right
    for (int i = parent->childs_count; i > index; i--) {
        parent->childs[i] = parent->childs[i - 1];
    }

    // Insert the child
    parent->childs[index] = child;
    parent->childs_count++;

    return 0;
}

// remove a tag
int pop_tag(struct html_tag* tag) {
    if (tag == NULL) {
        return 1;
    }
    if (tag->parent == NULL) {
        return 1;
    }

    // get the index of the tag
    int index = get_index(tag);

    // remove the tag
    if (index == -1) {
        return 1;
    }

    // shift the tags
    for (int i = index; i < tag->parent->childs_count - 1; i++) {
        tag->parent->childs[i] = tag->parent->childs[i + 1];
    }

    // decrease the count
    tag->parent->childs_count--;

    // free the tag
    destroy_html(tag);

    return 0;
}