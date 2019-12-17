#ifndef _LINK_H_
#define _LINK_H_

#define ELEMENT_SIZE 128

typedef struct _link{
	char ele_name[128];
	struct _link *next;
}link_s;

void link_init();
void link_delete();
/* 尾插发，保证结果和ip一一对应 */
void link_insert(char *element, int ele_len);
char *link_search(int index);

#endif
