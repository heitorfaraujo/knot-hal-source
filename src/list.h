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

void list_del(struct dlist_head *l)
{
	l->prev->next = l->next;
	l->next->prev = l->prev;
}

void list_add_tail(struct dlist_head *new, struct dlist_head *head)
{
	struct dlist_head *temp = head->prev;

	head->prev->next = new;
	head->prev = new;
	new->next = head;
	new->prev = temp;
}

void list_move_tail(struct dlist_head *list, struct dlist_head *head)
{
	list_del(list);
	init_list_head(list);

	list_add_tail(list, head);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif //	_LIST_H_
