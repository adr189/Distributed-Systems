/*
	Alexandre Rodrigues | FC54472
*/
#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"
#include "entry.h"
#include "data.h"
#include <stdbool.h>

/*
* Estrutura que define uma tree, quardado o nó que serve de root e o tamanho da tree
*/
struct tree_t {
	struct node_t *root; //Valor da raiz da arvore
	int size; //tamanho da arvore	
};

/*
* Estrutura que define um elemto de uma tree, com o seu valor(value) dois pointers que apontam para
* a left child e right child do nó respetivo
*/
struct node_t {
	struct entry_t *value; //valor do node atual
	struct node_t *leftChild; //pointer para o node filho esquerdo;
	struct node_t *rightChild; //pointer para o node filho direito;
};

/*
*	Funcão que cria um nó vazio
*/
struct node_t *node_create();

/*
*	Função que destroi um nó, libertanto a memória por ela ocupada
*/
void node_destroy(struct node_t *node);

/*
* 	Função para removere um nó com uma certa key a partir do root de uma tree
*/
struct node_t *node_del(struct node_t *node, char *key);

/*
*  Função que encontra o maior node da sub-tree esquerda com um node especifico a fazer de root
*/
struct node_t *findInorderSucessor(struct node_t *node);

/*
	Função que persquisa uma key especifica numa tree 
*/
bool search_tree(char *key, struct node_t *node);

/*
	Função que substitui uma entrada com uma chave pré-existente
*/
struct node_t *getNode(char *key, struct node_t * node);

/*
* Função que devolve a profundidade de uma tree
*/
int searchTreeDepth(struct node_t *node);

/*
* Função que modifica um array values, introduzindo todos os values de uma tree com root node,
* de forma que fique ordenada lexograficamente
*/
void getValues(struct node_t *node, void **values, int *i);

/*
* Função que modifica um array keys, introduzindo todos as keys de uma tree com root node,
* de forma que fique ordenada lexograficamente
*/
void getKeysInorder(struct node_t *node, char **keys, int *pos);

#endif
