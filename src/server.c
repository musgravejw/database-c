/* Database
 *
 * Copyright (c) 2019, John Musgrave <musgrajw at uc dot mail dot edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of this project nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


// @musgravejw
// heavily drawn from cstack.github.com/db_tutorial with my own changes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include "datatypes.h"
#include "b-tree.h"
#include "mm.h"
#include "parser.h"
#include "vm.h"


#define CLIENT_COUNT 10000

struct client {
    struct sockaddr_in address;
    socklen_t addr_len;
    int socket;
};


void prompt() {
	printf("\n?> ");
}


void motd() {
	printf("\n ############################## \
\n ##  ________  ________      ##\
\n ## |\\   ___ \\|\\   __  \\     ##\
\n ## \\ \\  \\_|\\ \\ \\  \\|\\ /_    ##\
\n ##  \\ \\  \\  \\ \\ \\   __  \\   ##\
\n ##   \\ \\  \\__\\ \\ \\  \\|\\  \\  ##\
\n ##    \\ \\_______\\ \\_______\\ ##\
\n ##     \\|_______|\\|_______| ##\
\n ##############################\n");

}

char **history;
int head;

int history_add(char *command) {
	strcpy(history[head], command);
	head++;

	if (sizeof(history) / sizeof(char *) / 2 > head) {
		// reallocate history
	}
}


int main(int argc, char* argv[]) {
	motd();

	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	struct protoent *protocol;
	socklen_t client_socklen;
	int server_socket;
	int client_socket;
	int enable = 1;
	ssize_t bytes_read;
	char *buffer;
	struct client clients[CLIENT_COUNT] = { 0 };
	size_t client_count = 0;
	char filename[32] = "mydb";

	printf("\nStarting server...");

	// open socket on host
	protocol = getprotobyname("tcp");
	server_socket = socket(AF_INET, SOCK_STREAM, protocol->p_proto);

	if (protocol == -1 || server_socket == -1) return -1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) return -1;

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(8080);

	if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) return -1;
	if (listen(server_socket, 5) == -1) return -1;

	fcntl(server_socket, F_SETFL, fcntl(server_socket, F_GETFL, 0) | O_NONBLOCK);
	buffer = malloc(1028);

	printf("\nListening on port 8080...\n");

	InputBuffer *input = malloc(sizeof(InputBuffer));

	input->buffer = malloc(1028);
	input->buffer_length = 0;
	input->input_length = 0;

	Table *table = db_open(filename);
	history = malloc(sizeof(char *) * 32);
	head = 0;

	while(1) {
		history[head] = malloc(sizeof(char*));

		if (client_count < sizeof clients / sizeof *clients) {
			client_socklen = sizeof(client_address);
			client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_socklen);

			if (client_socket != -1) {
				fcntl(client_socket, F_SETFL, fcntl(client_socket, F_GETFL, 0) | O_NONBLOCK);
				clients[client_count++] = (struct client) { .address = client_address, .addr_len = client_socklen, .socket = client_socket };

				printf("\nNew client connection: %d", client_count);                                                         
			}
		}

		for (size_t index = 0; index < client_count; index++) {
			ssize_t bytes_recvd = read(clients[index].socket, input->buffer, 1028);
			printf("\nReceived: %s", input->buffer);

			input->input_length = bytes_recvd - 1;
			input->buffer[bytes_recvd - 1] = 0;

			Statement statement;

			switch (prepare_statement(input, &statement)) {
				case (PREPARE_SUCCESS):
					break;
			    case (PREPARE_SYNTAX_ERROR):
			        printf("Syntax error. Could not parse statement.\n");
			        continue;
				case (PREPARE_UNRECOGNIZED_STATEMENT):
					printf("Unrecognized keyword at start of '%s'.\n", input->buffer);
				continue;
			}

			history_add(input->buffer);

			switch (execute_statement(&statement, table)) {
				case (EXECUTE_SUCCESS):
					printf("Done.\n");
					break;
				case (EXECUTE_DUPLICATE_KEY):
					printf("Error: Duplicate key.\n");
					break;
				case (EXECUTE_TABLE_FULL):
					printf("Error: Table full.\n");
					break;
			}

			if (bytes_recvd == 0) {
				db_close(table);
				free(input->buffer);
				free(input);
				close(clients[index].socket);
				client_count--;
				memmove(clients + index, clients + index + 1, (client_count - index) * sizeof clients);
				continue;
			}
		}

		sleep(0);
	}
}

