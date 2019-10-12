
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char* buffer;
	size_t buffer_length;
	ssize_t input_length;
} InputBuffer;


void prompt() {
	printf("?> ");
}


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
		} else {
			printf("Unknown identifier '%s'. \n", input->buffer);
		}
	}
}