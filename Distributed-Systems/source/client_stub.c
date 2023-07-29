/*
	Alexandre Rodrigues | FC54472
*/
#include "inet.h"
#include "client_stub-private.h"
#include "client_stub.h"
#include "data.h"
#include "entry.h"
#include "network_client.h"
#include "sdmessage.pb-c.h"
#include "tree_skel-private.h"
#include <signal.h>

/* Remote tree. A definir pelo grupo em client_stub-private.h
 */
struct rtree_t *tree_c;
void client_stub_signal(int);

/* 
 * Função handler que deteta e trata o sinal em caso de fecho inesperado do cliente
 */
void client_stub_signal(int signal) {
    free(tree_c);
    printf("\n");
    printf("Sinal de fecho de cliente, pressionou Ctrl + C.\n");
    exit(1);
}

/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect(const char *address_port) {
    //Dados
    tree_c = (struct rtree_t *) malloc (sizeof(struct rtree_t));
    char *host = strtok((char *)address_port, ":");
    int port = atoi(strtok(NULL,":"));

    tree_c->socket.sin_family = AF_INET;
    tree_c->socket.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &tree_c->socket.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        close(tree_c->sockfd);
        return NULL;
    }

    // Estabelece conexão com o servidor definido em server (a socket é criada em network_client)
    int connect = network_connect(tree_c);
    if(connect == -1) { //Verificar se a conexão foi bem sucedida em network_connect
        free(tree_c);
        return NULL;
    }

    signal(SIGINT, client_stub_signal);

    return tree_c;
}

/* Termina a associação entre o cliente e o servidor, fechando a 
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree) {
    if (rtree == NULL) {
        return -1;
    }
    if (network_close(rtree) != 0) {
        return -1;
    }
    free(rtree);
    return 0;

}

/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */

int rtree_put(struct rtree_t *rtree, struct entry_t *entry) {
    if (rtree == NULL) {
        return -1;
    }
    MessageT *msg = (MessageT*) malloc(sizeof(MessageT));
    message_t__init(msg);
    if(msg == NULL) {
        return -1;
    }
    msg->entry = (EntryT*) malloc (sizeof(EntryT));
    entry_t__init(msg->entry);
    if(msg->entry == NULL) {
        message_t__free_unpacked(msg, NULL);
        return -1;
    }
    msg->entry->key = malloc(sizeof(entry->key)+1);
    strcpy(msg->entry->key, entry->key);
    msg->entry->data = (DataT*) malloc (sizeof(DataT));
    data_t__init(msg->entry->data);
    if(msg->entry->data == NULL) {
        message_t__free_unpacked(msg, NULL);
        return -1;
    }
    msg->entry->data->datasize = entry->value->datasize;
    msg->entry->data->data = entry->value->data;
    msg->opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    MessageT *responseMsg = network_send_receive(rtree, msg);
    if(responseMsg == NULL) {
        return -1;
    }
    printf("O comando put com número de operação %d será executado em breve", responseMsg->size);
    printf("\n");
    message_t__free_unpacked(responseMsg, NULL);
    free(msg->entry->key);
    free(msg->entry->data);
    free(msg->entry);
    free(msg);
    return 0;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */

struct data_t *rtree_get(struct rtree_t *rtree, char *key) {
    if (rtree == NULL) {
        return NULL;
    }
    MessageT *msg = (MessageT*) malloc(sizeof(MessageT));
    message_t__init(msg);
    if(msg == NULL) {
        message_t__free_unpacked(msg, NULL);
        return NULL;
    }
    msg->entry = (EntryT*) malloc (sizeof(EntryT));
    entry_t__init(msg->entry);
    if(msg->entry == NULL) {
        message_t__free_unpacked(msg, NULL);
        return NULL;
    }
    msg->entry->data = (DataT*) malloc (sizeof(DataT));
    data_t__init(msg->entry->data);
    if(msg->entry->data == NULL) {
        message_t__free_unpacked(msg, NULL);
        return NULL;
    }
    msg->entry->key = key;
    msg->opcode = MESSAGE_T__OPCODE__OP_GET;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;

    MessageT *responseMsg = network_send_receive(rtree, msg);
    if(responseMsg == NULL) {
        return NULL;
    }
    int d_size = responseMsg->entry->data->datasize;
    struct data_t *return_data = data_create2(d_size, responseMsg->entry->data->data);
    message_t__free_unpacked(responseMsg, NULL);
    printf("Na key %s foi encontrada uma data com tamanho %d", key, return_data->datasize);
    printf("\n");
    free(msg);
    return return_data;
}

/* Função para remover um elemento da árvore. Vai libertar 
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */

int rtree_del(struct rtree_t *rtree, char *key) {
    if (rtree == NULL) {
        return -1;
    }
    MessageT *msg = (MessageT*) malloc(sizeof(MessageT));
    message_t__init(msg);
    if(msg == NULL) {
        return -1;
    }
    msg->entry = (EntryT*) malloc (sizeof(EntryT));
    entry_t__init(msg->entry);
     if(msg->entry == NULL) {
        return -1;
    }
    msg->entry->key = key;
    msg->opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;

    MessageT *responseMsg = network_send_receive(rtree, msg);
    if(responseMsg->opcode != MESSAGE_T__OPCODE__OP_ERROR) {
        printf("O comando del com número de operação %d será executado em breve", responseMsg->size);
        printf("\n");
    }
    printf("\n");
    if(responseMsg == NULL) {
        return -1;
    }
    free(msg->entry);
    free(msg);
    return 0;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree) {
    if (rtree == NULL) {
        return -1;
    }
    MessageT *msg = (MessageT*) malloc(sizeof(MessageT));
    message_t__init(msg);
    if(msg == NULL) {
        return -1;
    }
    msg->opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *responseMsg = network_send_receive(rtree, msg); 
    if(responseMsg == NULL) {
        return -1;
    }

    printf("O nº de elementos da árvore é: %d\n", responseMsg->size);
    printf("\n");
    int temp = responseMsg->size;
    free(msg);
    return temp;
}

/* Função que devolve a altura da árvore.
 */

int rtree_height(struct rtree_t *rtree) {
    if (rtree == NULL) {
        return -1;
    }
    MessageT *msg = (MessageT*) malloc(sizeof(MessageT));
    message_t__init(msg);
    if(msg == NULL) {
        return -1;
    }
    msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *responseMsg = network_send_receive(rtree, msg);
    if(responseMsg == NULL) {
        return -1;
    }
    printf("A altura da árvore é: %d\n", responseMsg->height);
    printf("\n");
    int temp2 = responseMsg->height;
    free(msg);
    return temp2;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */

char **rtree_get_keys(struct rtree_t *rtree) {
    if (rtree == NULL) {
        return NULL;
    }
    MessageT *msg = (MessageT*) malloc(sizeof(MessageT));
    if(msg == NULL) {
        return NULL;
    }
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->n_keys = 0;
    msg->keys = NULL;
    msg->size = 0;

    MessageT *responseMsg = network_send_receive(rtree, msg); 
    if(responseMsg == NULL) {
        free(msg);
        free(responseMsg);
        return NULL;
    } else {
        char **keys = malloc(responseMsg->size + 1);
        keys = responseMsg->keys;
        printf("As seguintes keys estão na árvore: ");
        printf("\n");
        for(int i = 0;  i  < responseMsg->n_keys; i++) {
            printf("%s", keys[i]);
            printf("\n");
        }
        message_t__free_unpacked(responseMsg, NULL);
        free(msg);
        return keys;
    }
}

/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */

void **rtree_get_values(struct rtree_t *rtree) {
    printf("Esta função não foi implementada \n");
    return NULL;
}

/* Verifica se a operação identificada por op_n foi executada.
*/
int rtree_verify(struct rtree_t *rtree, int op_n) {
    if (rtree == NULL) {
        return -1;
    }
    MessageT *msg = (MessageT*) malloc(sizeof(MessageT));
    if(msg == NULL) {
        return -1;
    }
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg->size = op_n;

    MessageT *responseMsg = network_send_receive(rtree, msg); 

    if(responseMsg == NULL) {
        printf("Existe um erro na verificação da operação ou  a operação ainda não foi concluída");
        return -1;
    }
    printf("A operação indetificada por %d foi executada", op_n);
    return 0;

}