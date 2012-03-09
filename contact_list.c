/* 
 * contact_list.c
 *
 * Copyright (C) 2012 James Booth <boothj5@gmail.com>
 * 
 * This file is part of Profanity.
 *
 * Profanity is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Profanity is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Profanity.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "contact_list.h"

// contact list node
struct _contact_node_t {
    contact_t *contact;    
    struct _contact_node_t *next;
};

// the contact list
static struct _contact_node_t *_contact_list = NULL;

static struct _contact_node_t * _make_contact_node(const char * const name, 
    const char * const show);
static contact_t * _new_contact(const char * const name, const char * const show);
static void _destroy_contact(contact_t *contact);

void contact_list_clear(void)
{
    struct _contact_node_t *curr = _contact_list;
    
    if (curr) {
        while(curr) {
            contact_t *contact = curr->contact;
            _destroy_contact(contact);
            curr = curr->next;
        }

        free(_contact_list);
        _contact_list = NULL;
    }
}

int contact_list_remove(const char * const name)
{
    if (!_contact_list) {
        return 0;
    } else {
        struct _contact_node_t *curr = _contact_list;
        struct _contact_node_t *prev = NULL;
        
        while(curr) {
            contact_t *contact = curr->contact;
            if (strcmp(contact->name, name) == 0) {
                if (prev)
                    prev->next = curr->next;
                else
                    _contact_list = curr->next;

                _destroy_contact(contact);
                free(curr);

                return 1;
            }

            prev = curr;
            curr = curr->next;
        }

        return 0;
    }
}

int contact_list_add(const char * const name, const char * const show)
{

    if (!_contact_list) {
        _contact_list = _make_contact_node(name, show);
        
        return 1;
    } else {
        struct _contact_node_t *curr = _contact_list;
        struct _contact_node_t *prev = NULL;

        while(curr) {
            contact_t *curr_contact = curr->contact;
            if (strcmp(curr_contact->name, name) == 0) {
                if (curr_contact->show != NULL) {
                    free(curr_contact->show);
                    curr_contact->show = NULL;
                    
                    if (show != NULL) {
                        curr_contact->show = 
                            (char *) malloc((strlen(show) + 1) * sizeof(char));
                        strcpy(curr_contact->show, show);
                    }
                }
                
                return 0;
            }

            prev = curr;
            curr = curr->next;
        }

        curr = _make_contact_node(name, show);    
        
        if (prev)
            prev->next = curr;

        return 1;
    }
}

contact_list_t *get_contact_list(void)
{
    int count = 0;
    
    contact_list_t *list = 
        (contact_list_t *) malloc(sizeof(contact_list_t));

    struct _contact_node_t *curr = _contact_list;
    
    if (!curr) {
        list->contacts = NULL;
    } else {
        list->contacts = (contact_t **) malloc(sizeof(contact_t *));

        while(curr) {
            contact_t *curr_contact = curr->contact;
            list->contacts[count] = 
                _new_contact(curr_contact->name, curr_contact->show);
            count++;
            curr = curr->next;
        }
    }

    list->size = count;

    return list;
}

struct _contact_node_t * _make_contact_node(const char * const name, 
    const char * const show)
{
    struct _contact_node_t *new = 
        (struct _contact_node_t *) malloc(sizeof(struct _contact_node_t));
    new->contact = _new_contact(name, show);
    new->next = NULL;

    return new;
}

static contact_t * _new_contact(const char * const name, const char * const show)
{
    contact_t *new = (contact_t *) malloc(sizeof(contact_t));
    new->name = (char *) malloc((strlen(name) + 1) * sizeof(char));
    strcpy(new->name, name);
    
    if (show != NULL) {
        new->show = (char *) malloc((strlen(show) + 1) * sizeof(char));
        strcpy(new->show, show);
    } else {
        new->show = NULL;
    }

    return new;
}

static void _destroy_contact(contact_t *contact)
{
    free(contact->name);

    if (contact->show != NULL) {
        free(contact->show);
    }

    free(contact);
}
