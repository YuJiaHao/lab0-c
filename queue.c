#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *l = malloc(1 * sizeof(struct list_head));
    if (!l)
        return NULL;
    INIT_LIST_HEAD(l);
    return l;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *ele;
    element_t *safe;
    list_for_each_entry_safe (ele, safe, l, list) {
        list_del(&ele->list);
        free(ele->value);
        free(ele);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ele = malloc(1 * sizeof(element_t));
    if (!ele)
        return false;

    size_t s_size = strlen(s) + 1;
    ele->value = (char *) malloc(s_size * sizeof(char));
    if (!ele->value) {
        free(ele);
        return false;
    }
    snprintf(ele->value, s_size, "%s", s);

    list_add(&ele->list, head);

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ele = malloc(1 * sizeof(element_t));
    if (!ele)
        return false;

    size_t s_size = strlen(s) + 1;
    ele->value = (char *) malloc(s_size * sizeof(char));
    if (!ele->value) {
        free(ele);
        return false;
    }
    snprintf(ele->value, s_size, "%s", s);

    list_add_tail(&ele->list, head);

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *ele = list_entry(head->next, element_t, list);
    if (!ele)
        return NULL;

    if (sp) {
        size_t sz =
            (bufsize > strlen(ele->value)) ? strlen(ele->value) : (bufsize - 1);
        memset(sp, '\0', sz + 1);
        strncpy(sp, ele->value, sz);
    }
    list_del(head->next);

    return ele;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *ele = list_entry(head->prev, element_t, list);
    if (!ele)
        return NULL;

    if (sp) {
        size_t sz =
            (bufsize > strlen(ele->value)) ? strlen(ele->value) : (bufsize - 1);
        memset(sp, '\0', sz + 1);
        strncpy(sp, ele->value, sz);
    }
    list_del(head->prev);

    return ele;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    int len = 0;
    struct list_head *li;
    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head **indir = &(head->next);
    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next)
        indir = &(*indir)->next;
    struct list_head *del = *indir;
    *indir = (*indir)->next;
    list_del(del);  // remove node from queue

    element_t *ele = list_entry(del, element_t, list);
    if (ele && ele->value)
        free(ele->value);
    if (ele)
        free(ele);
    // free(del) no need to free del, because memory of list_head pointer is
    // contained in element_t structure
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    if (list_empty(head) || list_is_singular(head))
        return true;
    struct list_head *cur = head->next;
    struct list_head *del;

    struct list_head tmp;  // this queue store nodes which need to remove
    INIT_LIST_HEAD(&tmp);

    while (cur != head) {
        if (cur->next != head &&
            (strcmp(list_entry(cur, element_t, list)->value,
                    list_entry(cur->next, element_t, list)->value) == 0)) {
            list_move_tail(cur->next, &tmp);
        } else if (!list_empty(&tmp) &&
                   (strcmp(list_entry(tmp.prev, element_t, list)->value,
                           list_entry(cur, element_t, list)->value) == 0)) {
            del = cur;
            cur = cur->next;
            list_move_tail(del, &tmp);
        } else {
            cur = cur->next;
        }
    }
    // Delete all nodes that have duplicate string here
    element_t *ele;
    element_t *safe;
    list_for_each_entry_safe (ele, safe, &tmp, list) {
        list_del(&ele->list);
        free(ele->value);
        free(ele);
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    struct list_head *prev = head, *cur = head->next;
    while (cur != head && cur->next != head) {
        // swap next pointer
        prev->next = cur->next;
        cur->next = prev->next->next;
        prev->next->next = cur;

        // swap prev pointer
        cur->prev = prev->next;
        cur->next->prev = cur;
        prev->next->prev = prev;

        prev = cur;
        cur = cur->next;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *li = head;
    do {
        struct list_head *tmp = li->next;
        li->next = li->prev;
        li->prev = tmp;
        li = tmp;
    } while (li != head);
}

/*
 * merge elements of two lists in ascending order into a list
 * return entry pointer for sorted list.
 */
void list_merge(struct list_head *entry1,
                struct list_head *entry2,
                struct list_head *sorted)
{
    while (!list_empty(entry1) && !list_empty(entry2)) {
        struct list_head *node =
            (strcmp(list_entry(entry1->next, element_t, list)->value,
                    list_entry(entry2->next, element_t, list)->value) <= 0)
                ? entry1->next
                : entry2->next;
        list_move_tail(node, sorted);
    }
    list_splice_tail(list_empty(entry1) ? entry2 : entry1, sorted);
}

/*
 */
void list_merge_sort(struct list_head *entry)
{
    if (list_empty(entry) || list_is_singular(entry))
        return;

    struct list_head left;
    struct list_head sorted;
    INIT_LIST_HEAD(&left);
    INIT_LIST_HEAD(&sorted);

    // Find mid node in list
    struct list_head **indir = &(entry->next);
    for (struct list_head *fast = entry->next;
         fast != entry && fast->next != entry; fast = fast->next->next)
        indir = &(*indir)->next;

    list_cut_position(&left, entry, (*indir)->prev);
    list_merge_sort(&left);
    list_merge_sort(entry);
    list_merge(&left, entry, &sorted);
    INIT_LIST_HEAD(entry);
    list_splice_tail(&sorted, entry);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    list_merge_sort(head);
}
