/*
SQL supported:

CREATE TABLE [VALUES]
CREATE DATABASE [name]
INSERT INTO [table] VALUES([..])
CREATE INDEX [TYPE] [TABLE]
SELECT [..]

*/

PrepareResult prepare_statement(InputBuffer *input,
                                Statement *statement) {
	if (strncmp(input->buffer, "INSERT", 6) == 0) {
		int args_assigned = sscanf(input->buffer, 
								  "INSERT %d %s %s", 
								  &(statement->row_to_insert.id),
								  statement->row_to_insert.username, 
								  statement->row_to_insert.email);

		if (args_assigned < 3) return PREPARE_SYNTAX_ERROR;

		statement->type = STATEMENT_INSERT;

		return PREPARE_SUCCESS;
	} else if (strncmp(input->buffer, "insert", 6) == 0) {
		int args_assigned = sscanf(input->buffer, 
								  "insert %d %s %s", 
								  &(statement->row_to_insert.id),
								  statement->row_to_insert.username, 
								  statement->row_to_insert.email);

		if (args_assigned < 3) return PREPARE_SYNTAX_ERROR;

		statement->type = STATEMENT_INSERT;

		return PREPARE_SUCCESS;
	} else if (strncmp(input->buffer, "CREATE", 6) == 0) {

		statement->type = STATEMENT_INSERT;

		return PREPARE_SUCCESS;
	} else if (strncmp(input->buffer, "create", 6) == 0) {

		statement->type = STATEMENT_INSERT;

		return PREPARE_SUCCESS;
	} else if (strcmp(input->buffer, "SELECT") == 0 ||
		strcmp(input->buffer, "select") == 0) {
		statement->type = STATEMENT_SELECT;
	
		return PREPARE_SUCCESS;
	}

	return PREPARE_UNRECOGNIZED_STATEMENT;
}

