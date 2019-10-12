
typedef struct {
	char *buffer;
	size_t buffer_length;
	ssize_t input_length;
} InputBuffer;


typedef enum {
	PREPARE_SUCCESS,
	PREPARE_SYNTAX_ERROR,
	PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;


typedef enum { 
	EXECUTE_SUCCESS, 
	EXECUTE_TABLE_FULL 
} ExecuteResult;

typedef enum { 
	STATEMENT_INSERT, 
	STATEMENT_SELECT 
} StatementType;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 100


typedef struct {
	int file_descriptor;
	uint32_t file_length;
	void* pages[TABLE_MAX_PAGES];
} Pager;


typedef struct {
	uint32_t num_rows;
	Pager *pager;
	void* pages[TABLE_MAX_PAGES];
} Table;


typedef struct {
	uint32_t id;
	char username[COLUMN_USERNAME_SIZE];
	char email[COLUMN_EMAIL_SIZE];
} Row;


typedef struct {
	StatementType type;
	Row row_to_insert;  // only used by insert statement
} Statement;


typedef struct {
	Table *table;
	uint32_t row_num;
	int end_of_table;  // Indicates a position one past the last element
} Cursor;


// find the size of an attribute in a struct
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;
