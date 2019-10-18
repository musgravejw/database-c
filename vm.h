
ExecuteResult execute_insert(Statement *statement, 
							 Table *table) {
	void* node = get_page(table->pager, table->root_page_num);
	
	uint32_t num_cells = (*leaf_node_num_cells(node));
	Row* row_to_insert = &(statement->row_to_insert);
	uint32_t key_to_insert = row_to_insert->id;
	Cursor* cursor = table_find(table, key_to_insert);

	if (cursor->cell_num < num_cells) {
		uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);

		if (key_at_index == key_to_insert)
			return EXECUTE_DUPLICATE_KEY;
	}

	leaf_node_insert(cursor, row_to_insert->id, row_to_insert);

	printf("Tree:\n");
	print_tree(table->pager, 0, 0);
	free(cursor);

	return EXECUTE_SUCCESS;
}


ExecuteResult execute_select(Statement *statement, 
							 Table *table) {
	Cursor *cursor = table_start(table);
	Row row;
	
	while (!(cursor->end_of_table)) {
		deserialize_row(cursor_value(cursor), &row);
		print_row(&row);
		cursor_advance(cursor);
	}

	free(cursor);

	return EXECUTE_SUCCESS;
}


ExecuteResult execute_statement(Statement *statement, 
								Table *table) {
	switch (statement->type) {
		case (STATEMENT_INSERT):
			return execute_insert(statement, table);
		case (STATEMENT_SELECT):
			return execute_select(statement, table);
	}
}
