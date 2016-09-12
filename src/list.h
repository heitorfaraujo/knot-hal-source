#ifndef _LIST_H_
#define _LIST_H_

#ifdef __cplusplus
extern "C"{
#endif


struct dlist_head {
	struct dlist_head *next, *prev;
};

void init_list_head(struct dlist_head *list)
{
	list->prev = list;
	list->next = list;
}

void list_add(struct dlist_head *new, struct dlist_head *head)
{
	head->next->prev = new;
	new->next = head->next;
	new->prev = head;
	head->next = new;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif //	_LIST_H_
