CC=gcc
CFLAGS=-Wall -g -pthread
SHARED=shared
SERVER=server
CLIENT=client

all: server client

server: $(SERVER)/server.c $(SERVER)/db/db.c $(SERVER)/handlers/patient_handler.c $(SERVER)/handlers/doctor_handler.c $(SERVER)/handlers/admin_handler.c $(SHARED)/socket.c $(SHARED)/protocol.c
	$(CC) $(CFLAGS) -I$(SHARED) -I$(SERVER)/db -I$(SERVER)/handlers -o server/server \
		$(SERVER)/server.c $(SERVER)/db/db.c $(SERVER)/handlers/patient_handler.c $(SERVER)/handlers/doctor_handler.c $(SERVER)/handlers/admin_handler.c $(SHARED)/socket.c $(SHARED)/protocol.c

client: $(CLIENT)/client.c $(CLIENT)/menus/patient_menu.c $(CLIENT)/menus/doctor_menu.c $(CLIENT)/menus/admin_menu.c $(SHARED)/socket.c $(SHARED)/protocol.c
	$(CC) $(CFLAGS) -I$(SHARED) -I$(CLIENT)/menus -o client/client \
		$(CLIENT)/client.c $(CLIENT)/menus/patient_menu.c $(CLIENT)/menus/doctor_menu.c $(CLIENT)/menus/admin_menu.c $(SHARED)/socket.c $(SHARED)/protocol.c

clean:
	rm -f server/server client/client

.PHONY: all server client clean
