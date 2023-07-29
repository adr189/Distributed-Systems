#
#	Alexandre Rodrigues | FC54472
#

BIN_DIR = binary
INC_DIR = include
LIB_DIR = lib
OBJ_DIR = object
SRC_DIR = source
OBJECTOS = client-lib.o tree-client.o tree-server.o
CC = gcc
CFLAGS = -Wall -g -I 
LIBS = -lpthread

all: client-lib.o tree-client tree-server

client-lib.o: data.o entry.o message.o client_stub.o network_client.o sdmessage.pb-c.o
	ld -r $(addprefix $(OBJ_DIR)/,$^) -o $(LIB_DIR)/$@

tree-client: data.o entry.o message.o tree_client.o client_stub.o network_client.o sdmessage.pb-c.o
	$(CC) $(addprefix $(OBJ_DIR)/,$^) -I/usr/local/include -L/usr/local/lib -lprotobuf-c -o $(BIN_DIR)/$@

tree-server: data.o entry.o tree.o message.o tree_server.o network_server.o tree_skel.o sdmessage.pb-c.o
	$(CC) $(addprefix $(OBJ_DIR)/,$^) -I/usr/local/include -L/usr/local/lib -lprotobuf-c $(LIBS) -o $(BIN_DIR)/$@

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INC_DIR) -o $(OBJ_DIR)/$@ -c $<

clean:
	rm –f $(OBJ_DIR)/.o
	rm –f $(BIN_DIR)/
	rm –f $(LIB_DIR)/