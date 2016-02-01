/*
 *	File name   : linked_list_queue.c
 *  Created on  : Jan 23, 2016
 *  Author      : yanly
 *  Description : A simple queue using a linked list written in C
 *  How does it work?
 *
	char* obj = strdup("test");
	void *q = llqueue_new();
	llqueue_offer(q, obj);
	printf("object from queue: %s\n", llqueue_poll(q));

 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <assert.h>

#include "linked_list_queue.h"

void *llqueue_new()
{
    linked_list_queue_t *qu;

    qu = calloc(1, sizeof(linked_list_queue_t));
    return qu;
}

void llqueue_free(
    linked_list_queue_t * qu
)
{
    llqnode_t *node;

    node = qu->head;

    while (node)
    {
        llqnode_t *prev;

        prev = node;
        node = node->next;
        free(prev);
    }
    free(qu);
}

void *llqueue_poll(
    linked_list_queue_t * qu
)
{
    llqnode_t *node;

    void *item;

    if (qu->head == NULL)
        return NULL;

    node = qu->head;
    item = node->item;
    if (qu->tail == qu->head)
        qu->tail = NULL;
    qu->head = node->next;
    free(node);
    qu->count--;

    return item;
}

void llqueue_offer(
    linked_list_queue_t * qu,
    void *item
)
{
    llqnode_t *node;

    node = malloc(sizeof(llqnode_t));
    node->item = item;
    node->next = NULL;
    if (qu->tail)
        qu->tail->next = node;
    if (!qu->head)
        qu->head = node;
    qu->tail = node;
    qu->count++;
}

void *llqueue_remove_item(
    linked_list_queue_t * qu,
    const void *item
)
{
    llqnode_t *node, *prev;

    prev = NULL;
    node = qu->head;

    while (node)
    {
        void *ritem;

        if (node->item == item)
        {
            if (node == qu->head)
            {
                return llqueue_poll(qu);
            }
            else
            {
                prev->next = node->next;
                if (node == qu->tail)
                {
                    qu->tail = prev;
                }

                qu->count--;
                ritem = node->item;
                free(node);
                return ritem;
            }
        }

        prev = node;
        node = node->next;
    }

    return NULL;
}

void *llqueue_remove_item_via_cmpfunction(
    linked_list_queue_t * qu,
    const void *item,
    int (*cmp)(const void*, const void*)
)
{
    llqnode_t *node, *prev;

    assert(cmp);
    assert(item);

    prev = NULL;
    node = qu->head;

    while (node)
    {
        void *ritem;

        if (0 == cmp(node->item,item))
        {
            if (node == qu->head)
            {
                return llqueue_poll(qu);
            }
            else
            {
                prev->next = node->next;
                if (node == qu->tail)
                {
                    qu->tail = prev;
                }

                qu->count--;
                ritem = node->item;
                free(node);
                return ritem;
            }
        }

        prev = node;
        node = node->next;
    }

    return NULL;
}

void *llqueue_get_item_via_cmpfunction(
    linked_list_queue_t * qu,
    const void *item,
    long (*cmp)(const void*, const void*)
)
{
    llqnode_t *node;

    assert(cmp);
    assert(item);

    node = qu->head;

    while (node)
    {
        if (0 == cmp(node->item,item))
        {
            return node->item;
        }

        node = node->next;
    }

    return NULL;
}

int llqueue_count(
    const linked_list_queue_t * qu
)
{
    return qu->count;
}
