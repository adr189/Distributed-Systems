/*
	Alexandre Rodrigues | FC54472
*/
#include "inet.h"
#include "network_server.h"
#include "tree_skel.h"
#include "tree_skel-private.h"


struct rtree_t *tree;

int main(int argc, char *argv[]) {

	if(argc != 3) {
		printf("Certifique-se que executou o programa corretamente, ./serve <port> <N>");
		printf("\n");
		return -1;
	}

	
	int socket = network_server_init(atoi(argv[1]));
	if(socket == -1) {
		printf("Ocorreu um erro ao criar o server");
	}
	tree_skel_init(atoi(argv[2]));
	network_main_loop(socket);
	network_server_close();
	tree_skel_destroy();

}