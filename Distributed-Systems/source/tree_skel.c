/*
	Alexandre Rodrigues | FC54472
*/
#include "inet.h"
#include "tree_skel-private.h"
#include "sdmessage.pb-c.h"
#include "tree.h"
#include "entry.h"
#include <pthread.h>
#include <stdbool.h>

struct tree_t *tree_s;
struct op_proc *operation;
int last_assigned;
struct request_t *queue_head;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_t *thread;
int *thread_param;
int thread_number;
int *r;
int verify(int);


/* Inicia o skeleton da árvore.
* O main() do servidor deve chamar esta função antes de poder usar a
* função invoke().
* A função deve lançar N threads secundárias responsáveis por atender
* pedidos de escrita na árvore.
* Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
*/
int tree_skel_init(int N) {  
    operation = malloc(sizeof(struct op_proc));
    operation->in_progress = malloc(sizeof(int) * N);
    thread_number = N;
    if (operation == NULL) {
        free(operation->in_progress);
        free(operation);
    }
    for (int i = 0; i < N; i++) {
        operation->in_progress[i] = 0;
    } 
    operation->max_proc = 0;
    last_assigned = 1;
    queue_head = NULL;
    tree_s = tree_create();
    int n_threads = N;
    thread = (pthread_t*) malloc(sizeof(pthread_t) * n_threads); //NAO LIBERTADO FALTA JOIN
    thread_param = malloc(sizeof(int) * n_threads); //NAO LIBERTADO FALTA JOIN

    printf("main() a iniciar\n");
    for (int i=0; i < n_threads; i++){
        thread_param[i] = i+1;
        if (pthread_create(&thread[i], NULL, &thread_impressao, (void *) &thread_param[i]) != 0){
            printf("\nThread %d não criada.\n", i);
            exit(EXIT_FAILURE);
        }
    }

    if (tree_s == NULL) {
        return -1;
    }

    return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy() {
    free(thread_param);
    free(operation->in_progress);
    free(operation);
    tree_destroy(tree_s);
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(MessageT *msg) {
    
    struct data_t *data;
    MessageT__Opcode opCode = msg->opcode;
    char **keysR;
    void **valuesR;
    int treeSize = tree_size(tree_s);

    switch(opCode) {

        case MESSAGE_T__OPCODE__OP_SIZE:
            msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->size = treeSize;
            return 0;
            break;

        

        case MESSAGE_T__OPCODE__OP_PUT:

            struct request_t *request1 = (struct request_t *) malloc(sizeof(struct request_t));
            if(request1 == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
                break;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
                msg->size = last_assigned;

                request1->op_n = last_assigned;
                last_assigned++;
                request1->op = 1;
                char *key_r = malloc(strlen(msg->entry->key) + 1);
                strcpy(key_r, msg->entry->key);
                request1->key = key_r;
                char *data_r = malloc(strlen(msg->entry->data->data) + 1);
                strcpy(data_r, msg->entry->data->data);
                request1->data = data_r;
                request1->datasize = msg->entry->data->datasize;

                pthread_mutex_lock(&queue_lock);

                if(queue_head == NULL){
                    queue_head = request1;
                    request1->next_request = NULL;
                } else{
                    struct request_t *tptr = queue_head;
                    while (tptr->next_request != NULL)
                        tptr=tptr->next_request;
                    tptr->next_request=request1; 
                    request1->next_request=NULL;
                }
                pthread_cond_signal(&queue_not_empty);
                pthread_mutex_unlock(&queue_lock);

                return 0;
                break;
            }

        case MESSAGE_T__OPCODE__OP_DEL:

            struct request_t *request2 = (struct request_t *) malloc(sizeof(struct request_t));
            if(request2 == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
                break;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
                msg->size = last_assigned;

                request2->op_n = last_assigned;
                last_assigned++;
                request2->op = 0;
                char *key_r = malloc(strlen(msg->entry->key) + 1);
                strcpy(key_r, msg->entry->key);
                request2->key = key_r;
                request2->data = NULL;
                request2->datasize = 0;

                pthread_mutex_lock(&queue_lock);

                if(queue_head == NULL){
                    queue_head = request2;
                    request2->next_request = NULL;
                } else{
                    struct request_t *tptr = queue_head;
                    while (tptr->next_request != NULL)
                        tptr=tptr->next_request;
                    tptr->next_request=request2; 
                    request2->next_request=NULL;
                }
                pthread_cond_signal(&queue_not_empty);
                pthread_mutex_unlock(&queue_lock);

                return 0;
                break;
            }
            
        case MESSAGE_T__OPCODE__OP_GET:
            data = tree_get(tree_s, msg->entry->key);
            if(data == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
                break;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
                msg->entry->data->datasize = data->datasize;
                msg->entry->data->data = data->data;
                return 0;
                break;
            }

        case MESSAGE_T__OPCODE__OP_HEIGHT:
            msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->height = tree_height(tree_s);
            return 0;
            break;

        case MESSAGE_T__OPCODE__OP_GETKEYS:
            keysR = tree_get_keys(tree_s);
            if(keysR == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
                break;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
                msg->n_keys = treeSize;
                msg->keys = keysR;
                msg->size = sizeof(keysR);
                return 0;
                break;
            }

        case MESSAGE_T__OPCODE__OP_GETVALUES:
            valuesR = tree_get_values(tree_s);
            if(valuesR == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
                break;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_VALUES;
                msg->n_data_s = treeSize;
                msg->data_s = (ProtobufCBinaryData*) valuesR;
                msg->size = treeSize;
                return 0;
                break;
            }

        case MESSAGE_T__OPCODE__OP_VERIFY:
            if(verify(msg->size) == 0) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
                break;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
                return 0;
                break;
            }


        default:
            return -1;
            break;
    }
    return -1;
}

/* Verifica se a operação identificada por op_n foi executada.
*/
int verify(int op_n) {
    if (op_n <= operation->max_proc) { 
        return 1;
    } 
    else if (op_n > operation->max_proc) {
        return 0;
    }
    else {
        for(int k = 0; k < thread_number; k++) {
            if(operation->in_progress[k] == op_n) {
                return 1;
            }
        }
        return 0;
    }
}

/* Função da thread secundária que vai processar pedidos de escrita.
*/
void *process_request(void *params) {

    int *thread_number = (int *) params;
    while(1){

        pthread_mutex_lock(&queue_lock);

        while(queue_head==NULL)
            pthread_cond_wait(&queue_not_empty, &queue_lock);

        struct request_t *request = queue_head;
        operation->in_progress[*thread_number-1] = request->op_n;
        

        if(request->op == 0){ //DELETE
            pthread_mutex_lock(&tree_lock);  
            if(tree_del(tree_s, request->key) == -1) {
                    printf("Ocorreu um erro a apagar o elemento %s da árvore", request->key);
                    if((request->op_n > operation->max_proc))
                        operation->max_proc = request->op_n;
                } else {
                    if((request->op_n > operation->max_proc))
                        operation->max_proc = request->op_n;
                }
            pthread_mutex_unlock(&tree_lock);
        } else { //PUT
            pthread_mutex_lock(&tree_lock); 
            struct data_t *data;
            data = data_create2(request->datasize, request->data);
            if(tree_put(tree_s, request->key, data) == -1) {
                printf("Ocorreu um erro a colocar o elemento %s na árvore", request->key);
                if((request->op_n > operation->max_proc))
                    operation->max_proc = request->op_n;
            } else {
                if((request->op_n > operation->max_proc))
                    operation->max_proc = request->op_n;
            }
            free(request->data);
            pthread_mutex_unlock(&tree_lock);
        }
        queue_head = request->next_request;
        free(request->key);
        free(request);
        pthread_mutex_unlock(&queue_lock);
    }
    pthread_exit(NULL);
    return NULL;
}

void *thread_impressao(void *params){
	int *thread_number = (int *) params;

	printf("Thread %d a iniciar\n", *thread_number);

		process_request(params);

	printf("Thread %d a terminar\n", *thread_number);

	return 0;
}