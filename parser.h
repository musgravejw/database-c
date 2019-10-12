
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datatypes.h"

PrepareResult prepare_statement(InputBuffer* input,
                                Statement* statement) {
	if (strncmp(input->buffer, "INSERT", 6) == 0 ||
		strncmp(input->buffer, "insert", 6) == 0) {
		statement->type = STATEMENT_INSERT;
		return PREPARE_SUCCESS;
	}

	if (strcmp(input->buffer, "SELECT") == 0 ||
		strcmp(input->buffer, "select") == 0) {
		statement->type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}

	return PREPARE_UNRECOGNIZED_STATEMENT;
}