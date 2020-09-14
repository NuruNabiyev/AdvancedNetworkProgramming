#include "systems_headers.h"
#include "subuff.h"
#include "linklist.h"

void free_sub(struct subuff *sub)
{
    if (sub->refcnt < 1) {
        //printf(" >> %s : freeing the sub at %p \n", __FUNCTION__, sub);
        free(sub->head);
        free(sub);
    }
}

void *sub_reserve(struct subuff *sub, unsigned int len)
{
    sub->data += len;
    return sub->data;
}

uint8_t *sub_head(struct subuff *sub)
{
    return sub->head;
}

uint8_t *sub_push(struct subuff *sub, unsigned int len)
{
    sub->data -= len;
    sub->len += len;
    return sub->data;
}

void sub_reset_header(struct subuff *sub)
{
    sub->data = sub->end - sub->dlen;
    sub->len = sub->dlen;
}
struct subuff *alloc_sub(unsigned int size)
{
    struct subuff *sub = calloc(sizeof(*sub), 1);
    sub->data = calloc(size, 1);
    sub->head = sub->data;
    sub->end = sub->data + size;
    sub->refcnt = 0;
    list_init(&sub->list);
    return sub;
}
