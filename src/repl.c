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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "datatypes.h"
#include "b-tree.h"
#include "mm.h"
#include "parser.h"
#include "vm.h"


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

	char filename[32];
	InputBuffer *input = malloc(sizeof(InputBuffer));

	input->buffer = NULL;
	input->buffer_length = 0;
	input->input_length = 0;

	printf("\nPlease select a database.");
	printf("\n?> ");
	scanf("%s", &filename);

	Table *table = db_open(filename);

	getline(&(input->buffer), &(input->buffer_length), stdin);
	printf("Done.");

	history = malloc(sizeof(char *) * 32);
	head = 0;

	history[head] = malloc(sizeof(char*));
	history_add(input->buffer);

	while(1) {
		prompt();

		// while (1) {
		// 	int i = 0;
		// 	if (getchar() == '\033') { // if the first value is esc
		// 	    getchar(); // skip the [
		// 	    switch(getchar()) { // the real value
		// 	        case 'A':
		// 	            // code for arrow up
		// 	        	prompt();
		// 	        	printf("%s", history[head - i % head]);
		// 	        	i++;
		// 	        case 'B':
		// 	            // code for arrow down
		// 	        	prompt();
		// 	        	printf("%s", history[head - i % head]);
		// 	        	i--;
		// 	        default:
		// 	        	break;
		// 	    }
	 	//     }
		// }

		history[head] = malloc(sizeof(char*));
		ssize_t n = getline(&(input->buffer), &(input->buffer_length), stdin);

		if (n <= 0) {
			printf("Error reading input\n");
			return -1;
		}

		input->input_length = n - 1;
		input->buffer[n - 1] = 0;

		if(strcmp(input->buffer, "quit") == 0 || strcmp(input->buffer, "exit") == 0) {
			db_close(table);
			free(input->buffer);
			free(input);

			return 0;
		} 

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
	}
}

