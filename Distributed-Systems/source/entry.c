/*
	Alexandre Rodrigues | FC54472
*/
#include "entry.h"
#include <stdio.h>
#include "data.h"
#include <string.h>
#include <stdlib.h>


struct entry_t *entry_create(char *key, struct data_t *data){
    struct entry_t *newEntry = malloc(sizeof(struct entry_t));
    if(newEntry == NULL){
        return NULL;
    }
    newEntry->key = key;
    newEntry->value = data;
    return newEntry;
}

void entry_destroy(struct entry_t *entry){
    if(entry != NULL){
        if(entry->value != NULL) { 
            data_destroy(entry->value);
        }
        if(entry->key != NULL) { 
            free(entry->key);
        }
        free(entry);
    }  
}

struct entry_t *entry_dup(struct entry_t *entry){
    if(entry != NULL && entry->key != NULL && entry -> value != NULL){
        struct data_t *copyEntry = data_dup(entry->value);
        char *key = strdup(entry->key);
        return entry_create(key, copyEntry);
    } else {
        return NULL;
    }
}

void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value){
    data_destroy(entry->value);
    free(entry->key);
    entry-> key = new_key;
    entry-> value = new_value;
}

int entry_compare(struct entry_t *entry1, struct entry_t *entry2){
        int cmp = strcmp(entry1->key,entry2->key);
        if(cmp == 0){
            return 0;
        } else if (cmp < 0){
            return -1;
        } else {
            return 1;
        }
}