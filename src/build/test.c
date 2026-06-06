typedef struct {
    size_t len;
    size_t cap;
} ListHeader;

void list_push(int **list, int x) {
    if(*list == NULL) {
	ListHeader *new_head = mem_alloc(4 * sizeof(x) + sizeof(ListHeader));
	*list = (int *)(new_head + 1);
	new_head->len = 1;
	new_head->cap = 4;
    }
    ListHeader *head = (ListHeader *)*list - 1;
    if(head->len + 1 > head->cap) {
	size_t old_len = head->len;
	size_t new_cap = (size_t)ceil(1.5 * (head->cap + 1));
	void *temp = head;
	head = mem_alloc(new_cap * sizeof(x) + sizeof(ListHeader));
	memcpy(head, temp, old_len * sizeof(x) + sizeof(ListHeader));
	head->cap = new_cap;
	*list = (int *)(head + 1);
    }
    *list[head->len] = x;
    head->len++;
}

#define JK_ListPush(list, x) do {
}
