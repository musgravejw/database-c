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
	printf("\n#############\n# Hey there #\n#############\n");
}


int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Must supply a database filename.\n");
		return -1;
	}

	motd();

	char *filename = argv[1];
	Table *table = db_open(filename);
	InputBuffer *input = malloc(sizeof(InputBuffer));

	input->buffer = NULL;
	input->buffer_length = 0;
	input->input_length = 0;

	while(1) {
		prompt();

		// TODO: maintain command history

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

		switch (execute_statement(&statement, table)) {
			case (EXECUTE_SUCCESS):
				printf("Completed\n");
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
