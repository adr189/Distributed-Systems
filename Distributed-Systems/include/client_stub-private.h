/*
	Alexandre Rodrigues | FC54472
*/
#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"
#include "inet.h"

struct rtree_t {
    struct sockaddr_in socket; //Informação pertinente ao server
    int sockfd; //Socket a usar pelo cliente
};

#endif