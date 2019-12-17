#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "link.h"

static link_s *link_head;
static link_s *link_tail;

void link_init()
{
	link_head = (link_s *)malloc(sizeof(link_s));
	memset(link_head, 0, sizeof(link_s));
	link_tail = link_head;

	return;
}

void link_delete()
{
	link_s *p;

	p = link_head;
	while(p != NULL)
	{
		link_head = link_head->next;
		free(p);
		p = link_head;
	}
	link_tail = link_head = NULL;

	return;
}

/* 尾插发，保证结果和ip一一对应 */
void link_insert(char *element, int ele_len)
{
	int size;
	link_s *p;
	
	size = ((ELEMENT_SIZE - 1) > ele_len) ? ele_len : (ELEMENT_SIZE - 1);
	p = (link_s *)malloc(sizeof(link_s));
	memset(p, 0, sizeof(link_s));
	strncpy(p->ele_name, element, size);
	p->next = NULL;
	link_tail->next = p;
	link_tail = p;

	return;
}

char *link_search(int index)
{
	int i;
	link_s *p;

	p = link_head;
	for(i = 0; i <= index; i++)
	{
		if(p == NULL)
		{
			return NULL;
		}
		p = p->next;
	}
	if(p == NULL)
	{
		return NULL;
	}

	return p->ele_name;
}
