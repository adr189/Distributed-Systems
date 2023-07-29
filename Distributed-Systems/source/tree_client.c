/*
	Alexandre Rodrigues | FC54472
*/
#include "inet.h"
#include "client_stub-private.h"
#include "client_stub.h"

struct rtree_t *tree;

int main(int argc, char *argv[]) {

	printf("Bem-vindo cliente");
	printf("\n");
	tree = rtree_connect((char *)argv[1]);
	if(tree == NULL) {
		printf("Ocorreu um erro na inicialização do programa. Certifique-se de que está a chamar o programa com 'ficheiro.c' 'hostname:port");
	}

	int quit = 0;
	while(quit == 0) {
		char* command = malloc(100);
		printf("Introduza um comando para ser executado sobre a árvore");
		printf("\n");
		char input [100];
		fgets(input, 100, stdin);
		command = strtok(input, " \n");
		printf("\n");
		if(strcmp(command, "size") == 0) {
			if(rtree_size(tree) == -1) {
				printf("Ocorreu um erro na busca de size");
				printf("\n");
			}
			printf("\n");
		} else if(strcmp(command, "put") == 0) {

			char *key = strtok(NULL, " ");
			void *data_temp = strtok(NULL, " ");
			void *data_input = strtok(data_temp, "\n");

			if(key != NULL && data_input != NULL) {
				char *data_ = malloc(strlen(data_input) +1);			
				strcpy(data_, data_temp);
				struct data_t *data5 = data_create2(strlen(data_input), data_);
				struct entry_t *entry = entry_create(key, data5);

				if(rtree_put(tree, entry) == -1) {
					printf("Ocorreu um erro a colocar o elemento na árvore");
					printf("\n");
				}
				data_destroy(data5);
				free(entry);
			
				printf("\n");
			} else {
				printf("Ocorreu um erro, certifique-se que está a usar put <key> <data>");
				printf("\n");
			}

		} else if(strcmp(command, "get") == 0) {

			char *key_temp = strtok(NULL, " ");
			char *key_input = strtok(key_temp, "\n");

			if(key_input != NULL) {
				char *key = malloc(strlen(key_input) + 1);
				strcpy(key, key_input);

				struct data_t *data = rtree_get(tree, key);
				if(data == NULL) {
					printf("Ocorreu um erro ao ir buscar o elemento a árvore");
					free(key);
					printf("\n");
				} else {
					free(key);
					printf("\n");
				}
			} else {
				printf("Ocorreu um erro, certifique-se que está a usar get <key>");
				printf("\n");
			}
			
			
		} else if(strcmp(command, "del") == 0) {
			char *key_temp = strtok(NULL, " ");
			char *key_input = strtok(key_temp, "\n");

			if(key_input != NULL) {
				char *key = malloc(strlen(key_input) + 1);
				strcpy(key, key_input);
				if(rtree_del(tree, key) == -1) {
					printf("Ocorreu um erro ao apagar elemento da árvore");
					free(key);
					printf("\n");
				} else {
					free(key);
					printf("\n");
				}
			} else {
				printf("Ocorreu um erro, certifique-se que está a usar del <key>");
				printf("\n");
			}

		} else if(strcmp(command, "height") == 0) {
			if(rtree_height(tree) == -1) {
				printf("Ocorreu um erro na busca de height");
				printf("\n");
			}
			printf("\n");
		
		} else if(strcmp(command, "getkeys") == 0) {
			char **keysC = rtree_get_keys(tree);
			if(keysC == NULL) {
				printf("Ocorreu um erro na busca das keys");
				printf("\n");
			} else {
				printf("\n");
			}
		} else if(strcmp(command, "getvalues") == 0){
			void **valuesC = rtree_get_values(tree);
			if(valuesC == NULL){
				printf("Ocorreu um erro na busca dos valores");
				printf("\n");
			} else {
				printf("\n");
			}
		}else if(strcmp(command, "quit") == 0) {
			rtree_disconnect(tree);
			quit = 1;
			printf("\n");
		}else if(strcmp(command, "verify") == 0){
			char *number_temp = strtok(NULL, " ");
			char *number = strtok(number_temp, "\n");
			rtree_verify(tree, atoi(number));
			printf("\n");

		} else {
			printf("Introduza um comando da list: ");
			printf("\n");
			printf("size");
			printf("\n");
			printf("put <key> <data>");
			printf("\n");
			printf("get <key>");
			printf("\n");
			printf("del <key>");
			printf("\n");
			printf("height");
			printf("\n");
			printf("getkeys");
			printf("\n");
			printf("getvalues");
			printf("\n");
			printf("quit");
			printf("\n");
			printf("verify <op_n>");

		}
		
 	}
	return 0;
}