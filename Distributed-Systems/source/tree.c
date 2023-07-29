/*
	Alexandre Rodrigues | FC54472
*/
#include "tree.h"
#include "entry.h"
#include "data.h"
#include "tree-private.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


struct tree_t *tree_create(){
    
    struct tree_t *newTree = malloc(sizeof(struct tree_t));
    if(newTree == NULL){
       // free(newTree);
        return NULL;
    }
    newTree -> root = node_create();
    newTree -> size = 0;

    return newTree;
}   

void tree_destroy(struct tree_t *tree){
    if(tree != NULL){
        node_destroy(tree->root);
        free(tree);
    }
}

int tree_put(struct tree_t *tree, char *key, struct data_t *value){
    if(key != NULL && tree != NULL && value != NULL){
        char *createdKey = malloc(strlen(key)+1);
        strcpy(createdKey,key);
        struct data_t *createdData = data_dup(value);
        struct entry_t *createdEntry = entry_create(createdKey,createdData);

        if(tree->root->value == NULL){
            tree->root->value = createdEntry;
            tree->size = (tree->size)+1;
        } else {
            struct node_t *correctNode = getNode(key, tree->root);
            if(correctNode != NULL){
                int res = strcmp(key, correctNode->value->key);
                if(res == 0){
                    entry_destroy(correctNode->value);
                    correctNode->value = createdEntry;
                } else if(res < 0) {
                    correctNode->leftChild = node_create();
                    correctNode->leftChild->value = createdEntry;
                    tree->size = (tree->size)+1;
                } else {
                    correctNode->rightChild = node_create();
                    correctNode->rightChild->value = createdEntry;
                    tree->size = (tree->size)+1;
                }
            } 
        } 
        return 0;
    } else {
        return -1;
    }
}


struct data_t *tree_get(struct tree_t *tree, char *key){
    
    struct node_t *correctNode = getNode(key, tree->root);
    int res = strcmp(key, correctNode->value->key);
    if(correctNode!=NULL){
        if(res == 0){
            return data_dup(correctNode->value->value);
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }

}

int tree_del(struct tree_t *tree, char *key) {
    if(key != NULL && tree != NULL){
        if(search_tree(key, tree->root) == true){
            node_del(tree->root, key);
            tree->size = (tree->size) - 1;
            return 0;
        } else {
            return -1;
        }
    } else return -1;
}

int tree_size(struct tree_t *tree){
    return tree->size;
}

int tree_height(struct tree_t *tree){
    return searchTreeDepth(tree->root);
}

char **tree_get_keys(struct tree_t *tree){

    if(tree == NULL){
        return NULL;
    }
    
    char **keys = (char **) calloc(tree_size(tree)+1, sizeof(char*));

    if(keys == NULL) {
        return NULL;
    }

    int arrayPos = 0;
    int *pointer;
    
    pointer = &arrayPos;

    getKeysInorder(tree->root, keys, pointer);

    return keys;
}


void **tree_get_values(struct tree_t *tree){

    if(tree == NULL){
        return NULL;
    }
    
    void **values = (void **) calloc(tree_size(tree)+1, sizeof(struct data_t));

    if(values == NULL){
        return NULL;
    }
    int arrayPos = 0;
    int *pointer;
    
    pointer = &arrayPos;

    getValues(tree->root, values, pointer);
    
    return values;
}

void tree_free_keys(char **keys){
    int i = 0;
    while (keys[i] != NULL) {
        free(keys[i]);
        i++;
    }
    free(keys);
}

void tree_free_values(void **values){
    int i = 0;
    while (values[i] != NULL) {
        data_destroy(values[i]);
        i++;
    }
    free(values);
}


//---------------------///-----------------------//

struct node_t *node_del(struct node_t *node,char *key) {
    int cmp = strcmp(key, node->value->key);

    if(cmp < 0) {
        node->leftChild = node_del(node->leftChild, key);
    } else if (cmp > 0) {
        node->rightChild = node_del(node->rightChild, key);
    } else {

        if(node -> leftChild == NULL && node ->rightChild == NULL) {
            node_destroy(node);
            return NULL;
            
        } else if(node -> leftChild != NULL && node ->rightChild == NULL) {
            struct node_t *newNode = node->leftChild;
            node_destroy(node);
            return newNode;

        } else if(node -> leftChild == NULL && node ->rightChild != NULL) {
            struct node_t *newNode = node->rightChild;
            node_destroy(node);
            return newNode;

        } else {
            struct node_t *InorderSuccesor = findInorderSucessor(node->leftChild);
            node->value = entry_dup(InorderSuccesor->value);
            node->leftChild = node_del(node->leftChild, InorderSuccesor->value->key);
        }

    }

    return node;
    
}

struct node_t *findInorderSucessor(struct node_t *node) {
    if(node != NULL) {
        struct node_t *currentNode = node;

        while(currentNode->rightChild != NULL) {
            currentNode = currentNode->rightChild;
        }

        return currentNode;
    }

    return NULL;
    
}

void getValues(struct node_t *node, void **values, int *i){
    if(node != NULL){
        getValues(node->leftChild, values, i);
        void *temp_value = data_dup(node->value->value->data);
        values[*i] = temp_value;
        (*i)++;  
        getValues(node->rightChild, values, i);
    }
}

void getKeysInorder(struct node_t *node, char **keys, int *pos) {
    if(node != NULL) {
        getKeysInorder(node->leftChild, keys, pos);
        char *temp_key = malloc(strlen(node->value->key)+1);
        strcpy(temp_key, node->value->key);
        keys[*pos] = temp_key;
        (*pos)++;  
        getKeysInorder(node->rightChild, keys, pos);
    }
}


int searchTreeDepth(struct node_t *node){
    if (node != NULL){
        int leftDepth = searchTreeDepth(node->leftChild);
        int rightDepth = searchTreeDepth(node->rightChild);
        
        if(node->leftChild != NULL) leftDepth++;
        if(node->rightChild != NULL) rightDepth++;
        
        if(leftDepth>rightDepth) return leftDepth;
        else return rightDepth;
    } else {    
        return 0;
    }
}



bool search_tree(char *key, struct node_t *node){
   
    if(key != NULL && (node->value) != NULL){
        int res = strcmp(key,node->value->key);
        if(res == 0){
            return true;
        } else if (res < 0){           
            if((node->leftChild) == NULL){
                return false;
            } else {
                return search_tree(key, node->leftChild);
            }
        } else {
            if((node->rightChild) == NULL){
                return false;
            } else {
                return search_tree(key, node->rightChild);
            }
        }
    } else {
        return false;
    }
}

struct node_t *getNode(char *key, struct node_t * node){
        int res = strcmp(key,node->value->key);
        if(res == 0){
            return node;
        } else if (res < 0){           
            if((node->leftChild) == NULL){
                return node;
            } else {
                return getNode(key, node->leftChild);
            }
        } else {
            if((node->rightChild) == NULL){
                return node;
            } else {
                return getNode(key, node->rightChild);
            }
        }
}


struct node_t *node_create(){
    
    struct node_t *newNode = malloc(sizeof(struct node_t));
    if(newNode == NULL){
        return NULL;
    }
    newNode -> value = NULL;
    newNode -> leftChild = NULL;
    newNode -> rightChild = NULL;

    return newNode;
}

void node_destroy(struct node_t *node){
    entry_destroy(node->value);
    free(node);
}