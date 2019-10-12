
typedef struct {
	char* buffer;
	size_t buffer_length;
	ssize_t input_length;
} InputBuffer;


typedef enum { PREPARE_SUCCESS, PREPARE_UNRECOGNIZED_STATEMENT } PrepareResult;
typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;


typedef struct {
  StatementType type;
} Statement;


void prompt() {
	printf("?> ");
}
