/*
	Alexandre Rodrigues | FC54472
*/

#define NFDESC 10


#include "inet.h"
#include "tree_skel.h"
#include "message-private.h"
#include "network_server.h"
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

struct sockaddr_in server;
int sockfd;
int sim;
void server_signal(int);

/* 
 * Função handler em caso de fecho inesperado do servidor
 */
void server_signal(int signal) {
	network_server_close();
	printf("\n");
    printf("Sinal de fecho de servidor.\n");
    exit(1);
}

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port) {
	// Cria socket TCP
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("Erro ao criar socket");
		return -1;
	}
	sim = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (int *)&sim, sizeof(sim)) < 0 ) {
		perror("SO_REUSEADDR setsockopt error");
	}
	// Preenche estrutura server para bind
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	signal(SIGINT, server_signal);
	// Faz bind
	if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("Erro ao fazer bind");
		close(sockfd);
		return -1;
	};
	// Faz listen
	if (listen(sockfd, 0) < 0){
		perror("Erro ao executar listen");
		close(sockfd);
		return -1;
	};
	return sockfd;
}

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */
int network_main_loop(int listening_socket) {
	signal(SIGINT, server_signal);
	struct sockaddr_in client;
	socklen_t size_client;
	struct pollfd desc_set[NFDESC];
	int i, nfds, kdfs;
	for (i = 0; i < NFDESC; i++) {
		desc_set[i].fd = -1;
		desc_set[i].revents = 0;
	}
  	desc_set[0].fd = listening_socket;
  	desc_set[0].events = POLLIN;
  	nfds = 1;
		while (((kdfs = (poll(desc_set, nfds, 10)) >= 0)) || signal(SIGINT, server_signal)) {
			if (kdfs > 0) {
				if ((desc_set[0].revents & POLLIN) && (nfds < NFDESC)) {
					if ((desc_set[nfds].fd = accept(desc_set[0].fd, (struct sockaddr *)&client, &size_client)) > 0){
						desc_set[nfds].events = POLLIN;
						nfds++;
					}
				}
				for (i = 1; i < nfds; i++) {
					if ((desc_set[i].revents & POLLIN) && desc_set[i].fd != -1) {
						MessageT *msg = network_receive(desc_set[i].fd);
						if (msg == NULL) {
							desc_set[i].fd = -1;
							free(msg);
						} else {
							invoke(msg);
							network_send(desc_set[i].fd, msg);
						}
					}
					
					if ((desc_set[i].revents & POLLERR) || (desc_set[i].revents & POLLHUP)) {
						close(desc_set[i].fd);
						desc_set[i].fd = -1;
					}
				}
			}

			for(int i = 1 ; i < nfds ;i++){
				if(desc_set[i].fd == -1){
					desc_set[i].fd = desc_set[nfds].fd;
					nfds --;
				}
			}
		}
	close(listening_socket);
  	return 0;
}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
MessageT *network_receive(int client_socket) {
	int nbytes = 0;
	int msg_size = 0;
	if ((nbytes = read_all(client_socket, &msg_size, sizeof(int)))== -1) {
		close(client_socket);
		return NULL;
	}
	int host_size;
	host_size = ntohl(msg_size);
	uint8_t *buf = malloc(host_size);
	if ((nbytes = read_all(client_socket, buf, host_size)) == -1) {
		close(client_socket);
		return NULL;
	}

	MessageT *msg = (MessageT * ) malloc(sizeof(MessageT));
	if(msg == NULL) {
		return NULL;
	}
	message_t__init(msg);
	msg = message_t__unpack(NULL, host_size, buf);
	free(buf);
	
	return msg;

}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, MessageT *msg) {
	int msg_psize = message_t__get_packed_size(msg);
	int net_size = htonl(msg_psize);
	uint8_t *buf = malloc(msg_psize);
	message_t__pack(msg, buf);
	if (write_all(client_socket, (uint8_t *) &net_size, sizeof(int)) == -1) {
		close(client_socket); 
		return -1;
	}
	if (write_all(client_socket, buf, msg_psize) == -1) {
		close(client_socket);
		return -1;
	}
	message_t__free_unpacked(msg, NULL);
	free(buf);
    return 0;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close() {
    return close(sockfd);
}
