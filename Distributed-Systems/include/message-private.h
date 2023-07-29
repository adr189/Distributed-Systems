/*
	Alexandre Rodrigues | FC54472
*/
#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include "sdmessage.pb-c.h"
#include "inet.h"

/*
 * Função que envia uma string inteira pela rede
*/
int write_all(int sock, void *buf, int len);

/*
 * Função que recebe uma string inteira pela rede
*/
int read_all(int sock, void *buf, int len);

#endif