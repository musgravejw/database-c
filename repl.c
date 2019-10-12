#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datatypes.h"
#include "parser.h"
#include "vm.h"

int main(int argc, char* argv[]) {
	InputBuffer* input = malloc(sizeof(InputBuffer));
	input->buffer = NULL;
	input->buffer_length = 0;
	input->input_length = 0;

	while(1) {
		prompt();

		ssize_t n = getline(&(input->buffer), &(input->buffer_length), stdin);

		if (n <= 0) {
			printf("Error reading input\n");
			return 1;
		}

		input->input_length = n - 1;
		input->buffer[n - 1] = 0;

		if(strcmp(input->buffer, "quit") == 0 || 
			strcmp(input->buffer, "exit") == 0) {
			free(input->buffer);
			free(input);
			return 0;
		} 

		Statement statement;
	
		switch (prepare_statement(input, &statement)) {
			case (PREPARE_SUCCESS):
				break;
			case (PREPARE_UNRECOGNIZED_STATEMENT):
				printf("Unrecognized keyword at start of '%s'.\n",
				input->buffer);
			continue;
		}

		execute_statement(&statement);
		printf("Executed.\n");
	}
}
