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

