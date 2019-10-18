typedef enum { 
	NODE_INTERNAL, 
	NODE_LEAF 
} NodeType;

/*
 * Common Node Header Layout
 */
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint8_t COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

/*
 * Leaf Node Header Layout
 */
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_NEXT_LEAF_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NEXT_LEAF_OFFSET = LEAF_NODE_NUM_CELLS_OFFSET + LEAF_NODE_NUM_CELLS_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE + LEAF_NODE_NEXT_LEAF_SIZE;
 
/*
 * Leaf Node Body Layout
 */
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

/*
 * Internal Node Header Layout
 */
const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET = INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;
const uint32_t INTERNAL_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + INTERNAL_NODE_NUM_KEYS_SIZE + INTERNAL_NODE_RIGHT_CHILD_SIZE;
const uint32_t INTERNAL_NODE_MAX_CELLS = 3;

/*
 * Internal Node Body Layout
 */
const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CELL_SIZE = INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE;


/*
 * Forward Declarations
 */
void* get_page(Pager *pager, uint32_t page_num);
void serialize_row(Row *src, void* dst);
uint32_t get_node_max_key(void* node);
void create_new_root(Table *table, uint32_t right_child_page_num);
void leaf_node_split_and_insert(Cursor *cursor, uint32_t key, Row *value);
uint32_t internal_node_find_child(void* node, uint32_t key);


/*
Until we start recycling free pages, new pages will always
go onto the end of the database file
*/
uint32_t get_unused_page_num(Pager* pager) { return pager->num_pages; }

const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) / 2;
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;


uint32_t* internal_node_cell(void* node, 
							 uint32_t cell_num) {
	return node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE;
}


uint32_t* internal_node_right_child(void* node) {
	return node + INTERNAL_NODE_RIGHT_CHILD_OFFSET;
}


uint32_t* internal_node_key(void* node, 
							uint32_t key_num) {
	return (void*)internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE;
}


uint32_t* internal_node_num_keys(void* node) {
	return node + INTERNAL_NODE_NUM_KEYS_OFFSET;
}


uint32_t* internal_node_child(void* node, 
							  uint32_t child_num) {
	uint32_t num_keys = *internal_node_num_keys(node);

	if (child_num > num_keys) {
		printf("Tried to access child_num %d > num_keys %d\n", child_num, num_keys);
		exit(EXIT_FAILURE);
	} else if (child_num == num_keys) {
		return internal_node_right_child(node);
	} else {
		return internal_node_cell(node, child_num);
	}
}


int is_node_root(void* node) {
	uint8_t value = *((uint8_t*)(node + IS_ROOT_OFFSET));
	return (int)value;
}


uint32_t* leaf_node_num_cells(void* node) {
  return node + LEAF_NODE_NUM_CELLS_OFFSET;
}


void* leaf_node_cell(void* node, 
					 uint32_t cell_num) {
  return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}


uint32_t* leaf_node_key(void* node, 
						uint32_t cell_num) {
  return leaf_node_cell(node, cell_num);
}


void* leaf_node_value(void* node, 
					  uint32_t cell_num) {
  return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}


void set_node_type(void* node, 
				   NodeType type) {
	uint8_t value = type;
	*((uint8_t*)(node + NODE_TYPE_OFFSET)) = value;
}


void set_node_root(void* node, 
				   int is_root) {
	uint8_t value = is_root;
	*((uint8_t*)(node + IS_ROOT_OFFSET)) = value;
}


uint32_t* leaf_node_next_leaf(void* node) {
	return node + LEAF_NODE_NEXT_LEAF_OFFSET;
}


void initialize_leaf_node(void* node) {
	set_node_type(node, NODE_LEAF);
	set_node_root(node, 0);
	*leaf_node_num_cells(node) = 0;
	*leaf_node_next_leaf(node) = 0;  // 0 represents no sibling
}


void initialize_internal_node(void* node) {
	set_node_type(node, NODE_INTERNAL);
	set_node_root(node, 0);
	*internal_node_num_keys(node) = 0;
}


uint32_t* node_parent(void* node) { return node + PARENT_POINTER_OFFSET; }


void update_internal_node_key(void* node,
							  uint32_t old_key, 
							  uint32_t new_key) {
	uint32_t old_child_index = internal_node_find_child(node, old_key);
	*internal_node_key(node, old_child_index) = new_key;
}


void indent(uint32_t level) {
	for (uint32_t i = 0; i < level; i++) {
		printf("  ");
	}
}


void leaf_node_insert(Cursor *cursor, 
					  uint32_t key, 
					  Row *value) {
	void* node = get_page(cursor->table->pager, cursor->page_num);
	uint32_t num_cells = *leaf_node_num_cells(node);

	if (num_cells >= LEAF_NODE_MAX_CELLS) {
		// Node full
		leaf_node_split_and_insert(cursor, key, value);
		return;
	}

	if (cursor->cell_num < num_cells) {
		// Make room for new cell
		for (uint32_t i = num_cells; i > cursor->cell_num; i--)
			memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
	}

	*(leaf_node_num_cells(node)) += 1;
	*(leaf_node_key(node, cursor->cell_num)) = key;
	serialize_row(value, leaf_node_value(node, cursor->cell_num));
}


Cursor* leaf_node_find(Table *table, 
					   uint32_t page_num, 
					   uint32_t key) {
	void* node = get_page(table->pager, page_num);
	uint32_t num_cells = *leaf_node_num_cells(node);

	Cursor *cursor = malloc(sizeof(Cursor));
	cursor->table = table;
	cursor->page_num = page_num;

	// Binary search
	uint32_t min_index = 0;
	uint32_t one_past_max_index = num_cells;
	while (one_past_max_index != min_index) {
		uint32_t index = (min_index + one_past_max_index) / 2;
		uint32_t key_at_index = *leaf_node_key(node, index);

		if (key == key_at_index) {
			cursor->cell_num = index;
			return cursor;
		}

		if (key < key_at_index) {
			one_past_max_index = index;
		} else {
			min_index = index + 1;
		}
	}

	cursor->cell_num = min_index;
	return cursor;
}


NodeType get_node_type(void* node) {
	uint8_t value = *((uint8_t*)(node + NODE_TYPE_OFFSET));
	return (NodeType)value;
}


void internal_node_insert(Table *table, 
						  uint32_t parent_page_num,
                          uint32_t child_page_num) {
	/*
	Add a new child/key pair to parent that corresponds to child
	*/
	void* parent = get_page(table->pager, parent_page_num);
	void* child = get_page(table->pager, child_page_num);
	uint32_t child_max_key = get_node_max_key(child);
	uint32_t index = internal_node_find_child(parent, child_max_key);

	uint32_t original_num_keys = *internal_node_num_keys(parent);
	*internal_node_num_keys(parent) = original_num_keys + 1;

	if (original_num_keys >= INTERNAL_NODE_MAX_CELLS) {
		printf("Need to implement splitting internal node\n");
		exit(EXIT_FAILURE);
	}

	uint32_t right_child_page_num = *internal_node_right_child(parent);
	void* right_child = get_page(table->pager, right_child_page_num);

	if (child_max_key > get_node_max_key(right_child)) {
		/* Replace right child */
		*internal_node_child(parent, original_num_keys) = right_child_page_num;
		*internal_node_key(parent, original_num_keys) =
		get_node_max_key(right_child);
		*internal_node_right_child(parent) = child_page_num;
	} else {
		/* Make room for the new cell */
		for (uint32_t i = original_num_keys; i > index; i--) {
			void* destination = internal_node_cell(parent, i);
			void* source = internal_node_cell(parent, i - 1);
			memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
		}

		*internal_node_child(parent, index) = child_page_num;
		*internal_node_key(parent, index) = child_max_key;
	}
}


uint32_t internal_node_find_child(void* node, 
								  uint32_t key) {
	/*
	Return the index of the child which should contain
	the given key.
	*/
	uint32_t num_keys = *internal_node_num_keys(node);

	/* Binary search */
	uint32_t min_index = 0;
	uint32_t max_index = num_keys; /* there is one more child than key */

	while (min_index != max_index) {
		uint32_t index = (min_index + max_index) / 2;
		uint32_t key_to_right = *internal_node_key(node, index);
		
		if (key_to_right >= key) {
			max_index = index;
		} else {
			min_index = index + 1;
		}
	}

	return min_index;
}


Cursor* internal_node_find(Table *table, 
						   uint32_t page_num, 
						   uint32_t key) {
	void* node = get_page(table->pager, page_num);

	uint32_t child_index = internal_node_find_child(node, key);
	uint32_t child_num = *internal_node_child(node, child_index);

	void* child = get_page(table->pager, child_num);
	
	switch (get_node_type(child)) {
		case NODE_LEAF:
			return leaf_node_find(table, child_num, key);
		case NODE_INTERNAL:
			return internal_node_find(table, child_num, key);
	}
}


void leaf_node_split_and_insert(Cursor *cursor, 
								uint32_t key, 
								Row *value) {
	/*
	Create a new node and move half the cells over.
	Insert the new value in one of the two nodes.
	Update parent or create a new parent.
	*/
	void* old_node = get_page(cursor->table->pager, cursor->page_num);
	uint32_t old_max = get_node_max_key(old_node);
	uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
	void* new_node = get_page(cursor->table->pager, new_page_num);
	initialize_leaf_node(new_node);
	*node_parent(new_node) = *node_parent(old_node);
	*leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
	*leaf_node_next_leaf(old_node) = new_page_num;

	/*
	All existing keys plus new key should be divided
	evenly between old (left) and new (right) nodes.
	Starting from the right, move each key to correct position.
	*/
	for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
		void* destination_node;

		if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
			destination_node = new_node;
		} else {
			destination_node = old_node;
		}

		uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
		void* destination = leaf_node_cell(destination_node, index_within_node);

		if (i == cursor->cell_num) {
			serialize_row(value, leaf_node_value(destination_node, index_within_node));
			*leaf_node_key(destination_node, index_within_node) = key;
		} else if (i > cursor->cell_num) {
			memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
		} else {
			memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
		}
	}

	/* Update cell count on both leaf nodes */
	*(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
	*(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

	if (is_node_root(old_node)) {
		create_new_root(cursor->table, new_page_num);
	} else {
		uint32_t parent_page_num = *node_parent(old_node);
		uint32_t new_max = get_node_max_key(old_node);
		void* parent = get_page(cursor->table->pager, parent_page_num);

		update_internal_node_key(parent, old_max, new_max);
		internal_node_insert(cursor->table, parent_page_num, new_page_num);

    	return;
	}
}


void create_new_root(Table *table, 
					 uint32_t right_child_page_num) {
	/*
	Handle splitting the root.
	Old root copied to new page, becomes left child.
	Address of right child passed in.
	Re-initialize root page to contain the new root node.
	New root node points to two children.
	*/
	void* root = get_page(table->pager, table->root_page_num);
	void* right_child = get_page(table->pager, right_child_page_num);
	uint32_t left_child_page_num = get_unused_page_num(table->pager);
	void* left_child = get_page(table->pager, left_child_page_num);

	/* Left child has data copied from old root */
	memcpy(left_child, root, PAGE_SIZE);
	set_node_root(left_child, 0);

	/* Root node is a new internal node with one key and two children */
	initialize_internal_node(root);
	set_node_root(root, 1);
	*internal_node_num_keys(root) = 1;
	*internal_node_child(root, 0) = left_child_page_num;
	uint32_t left_child_max_key = get_node_max_key(left_child);
	*internal_node_key(root, 0) = left_child_max_key;
	*internal_node_right_child(root) = right_child_page_num;
	*node_parent(left_child) = table->root_page_num;
	*node_parent(right_child) = table->root_page_num;
}


uint32_t get_node_max_key(void* node) {
	switch (get_node_type(node)) {
		case NODE_INTERNAL:
			return *internal_node_key(node, *internal_node_num_keys(node) - 1);
		case NODE_LEAF:
			return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
	}
}


void print_tree(Pager *pager, 
				uint32_t page_num, 
				uint32_t indentation_level) {
	void* node = get_page(pager, page_num);
	uint32_t num_keys, child;

	switch (get_node_type(node)) {
		case (NODE_LEAF):
			num_keys = *leaf_node_num_cells(node);
			indent(indentation_level);
			printf("- leaf (size %d)\n", num_keys);
			
			for (uint32_t i = 0; i < num_keys; i++) {
				indent(indentation_level + 1);
				printf("- %d\n", *leaf_node_key(node, i));
			}

			break;
		case (NODE_INTERNAL):
			num_keys = *internal_node_num_keys(node);
			indent(indentation_level);
			printf("- internal (size %d)\n", num_keys);
			
			for (uint32_t i = 0; i < num_keys; i++) {
				child = *internal_node_child(node, i);
				print_tree(pager, child, indentation_level + 1);

				indent(indentation_level + 1);
				printf("- key %d\n", *internal_node_key(node, i));
			}

			child = *internal_node_right_child(node);
			print_tree(pager, child, indentation_level + 1);

			break;
	}
}