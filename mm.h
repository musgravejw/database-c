
Pager* pager_open(const char *filename) {
	// Read/Write mode
	// Create file if it does not exist
	// User write permission
	// User read permission
	int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

	if (fd == -1) {
		printf("Unable to open file\n");
		exit(EXIT_FAILURE);
	}

	off_t file_length = lseek(fd, 0, SEEK_END);

	Pager* pager = malloc(sizeof(Pager));
	pager->file_descriptor = fd;
	pager->file_length = file_length;

	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		pager->pages[i] = NULL;
	}

	return pager;
}


void pager_flush(Pager *pager, 
				 uint32_t page_num, 
				 uint32_t size) {
	if (pager->pages[page_num] == NULL) {
		printf("Tried to flush null page\n");
		exit(EXIT_FAILURE);
	}

	off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

	if (offset == -1) {
		printf("Error seeking: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	ssize_t bytes_written =
	write(pager->file_descriptor, pager->pages[page_num], size);

	if (bytes_written == -1) {
		printf("Error writing: %d\n", errno);
		exit(EXIT_FAILURE);
	}
}


Table* db_open(const char *filename) {
	Pager *pager = pager_open(filename);
	Table *table = malloc(sizeof(Table));
	uint32_t num_rows = pager->file_length / ROW_SIZE;

	table->pager = pager;
	table->num_rows = 0;
	
	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		table->pages[i] = NULL;
	}

	return table;
}


void db_close(Table* table) {
	Pager *pager = table->pager;
	uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

	for (uint32_t i = 0; i < num_full_pages; i++) {
		if (pager->pages[i] == NULL) {
			continue;
		}

		pager_flush(pager, i, PAGE_SIZE);
		free(pager->pages[i]);
		pager->pages[i] = NULL;
	}

	// There may be a partial page to write to the end of the file
	// This should not be needed after we switch to a B-tree
	uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;

	if (num_additional_rows > 0) {
		uint32_t page_num = num_full_pages;
		if (pager->pages[page_num] != NULL) {
			pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
			free(pager->pages[page_num]);
			pager->pages[page_num] = NULL;
		}
	}

	int result = close(pager->file_descriptor);

	if (result == -1) {
		printf("Error closing db file.\n");
		exit(EXIT_FAILURE);
	}

	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		void* page = pager->pages[i];
		if (page) {
			free(page);
			pager->pages[i] = NULL;
		}
	}

	free(pager);
	free(table);
}


void free_table(Table *table) {
    for (int i = 0; table->pages[i]; i++) {
		free(table->pages[i]);
    }

    free(table);
}


void print_row(Row *row) {
	printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}


void serialize_row(Row *src, 
				   void* dst) {
	memcpy(dst + ID_OFFSET, &(src->id), ID_SIZE);
	memcpy(dst + USERNAME_OFFSET, &(src->username), USERNAME_SIZE);
	memcpy(dst + EMAIL_OFFSET, &(src->email), EMAIL_SIZE);
}


void deserialize_row(void* src, 
					 Row *dst) {
	memcpy(&(dst->id), src + ID_OFFSET, ID_SIZE);
	memcpy(&(dst->username), src + USERNAME_OFFSET, USERNAME_SIZE);
	memcpy(&(dst->email), src + EMAIL_OFFSET, EMAIL_SIZE);
}


void* get_page(Pager *pager, 
			   uint32_t page_num) {
	if (page_num > TABLE_MAX_PAGES) {
		printf("Tried to fetch page number out of bounds. %d > %d\n", page_num, TABLE_MAX_PAGES);
		exit(EXIT_FAILURE);
	}

	if (pager->pages[page_num] == NULL) {
		// Cache miss. Allocate memory and load from file.
		void* page = malloc(PAGE_SIZE);
		uint32_t num_pages = pager->file_length / PAGE_SIZE;

		// We might save a partial page at the end of the file
		if (pager->file_length % PAGE_SIZE) num_pages += 1;

		if (page_num <= num_pages) {
			lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
			ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
			if (bytes_read == -1) {
				printf("Error reading file: %d\n", errno);
				exit(EXIT_FAILURE);
			}
		}

		pager->pages[page_num] = page;
	}

	return pager->pages[page_num];
}


void* cursor_value(Cursor *cursor) {
	Table *table = cursor->table;

	uint32_t row_num = cursor->row_num;
	uint32_t page_num = row_num / ROWS_PER_PAGE;

	void* page = get_page(table->pager, page_num);

	uint32_t row_offset = row_num % ROWS_PER_PAGE;
	uint32_t byte_offset = row_offset * ROW_SIZE;
	
	return page + byte_offset;
}


Cursor* table_start(Table *table) {
  Cursor *cursor = malloc(sizeof(Cursor));

  cursor->table = table;
  cursor->row_num = 0;
  cursor->end_of_table = (table->num_rows == 0);

  return cursor;
}


Cursor* table_end(Table *table) {
  Cursor *cursor = malloc(sizeof(Cursor));

  cursor->table = table;
  cursor->row_num = table->num_rows;
  cursor->end_of_table = 1;

  return cursor;
}


void cursor_advance(Cursor *cursor) {
	cursor->row_num += 1;
	Table *table = cursor->table;

	if (cursor->row_num >= table->num_rows) {
		cursor->end_of_table = 1;
	}
}
