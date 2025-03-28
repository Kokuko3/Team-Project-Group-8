#include "lists.h"
#include <assert.h>
#include <stdio.h> //printf, fopen, fclose, fwrite
#include <string.h> // Provides memcpy
#include <stdlib.h> // Provides malloc, calloc, realloc, free

struct node **create() {
    return calloc(1,sizeof(struct node*));
}

void destroy(struct node **list) {
    assert(list);

    nullify(list);
    free(list);
}

void nullify(struct node **list) {
    assert(list);

    if(!*list) return;
    struct node *tmp = head(list);
    while(tmp->next) {
        tmp=tmp->next;
        free_record(tmp->previous);
    }
    free_record(tmp);
    *list=0;
}

struct node *head(struct node **list) {
    assert(list);

    if(!*list) return NULL;
    struct node *tmp = *list;
    while(1) {
        if(tmp->previous) {
            tmp = tmp->previous;
        } else {
            return tmp;
        }
    }

}

struct node *tail(struct node **list) {
    assert(list);

    if(!*list) return NULL;
    struct node *tmp = *list;
    while(1) {
        if(tmp->next) {
            tmp = tmp->next;
        } else {
            return tmp;
        }
    }

}

struct node *next(struct node *record) {
    assert(record);
    return record->next;
}

struct node *previous(struct node *record) {
    assert(record);
    return record->previous;
}

size_t length(struct node **list) {
    assert(list);

    if(!*list) return 0;
    struct node *tmp = *list;
    size_t i = 1;
    
    while(1) {
        if(!tmp) break;
        if(tmp->previous) {
            tmp = tmp->previous;
            i++;
        } else {
            break;
        }
    }

    tmp = *list;

    while(1) {
        if(!tmp) break;
        if(tmp->next) {
            tmp = tmp->next;
            i++;
        } else {
            break;
        }
    }

    return i;

}

struct node* retrieve(struct node **list, size_t i) {
    assert(list);
    assert(i);
    assert(i <= length(list));

    struct node *tmp = head(list);
    for(size_t j = 1; j < i; j++) {
        tmp = next(tmp);
    }
    return tmp;
}

struct node *alloc_record(void *src, size_t size) {
    assert(src);
    assert(size);

    struct node *record = calloc(1, sizeof(struct node));
    void *data = malloc(size);

    if(!record || !data) return NULL;

    record->data = data;
    record->size = size;
    memcpy(record->data, src, size);

    return record;
}

void free_record(struct node *record) {
    assert(record);

    free(record->data);
    free(record);
}

struct node *prepend(struct node **list, void *src, size_t size) {
    assert(list);
    assert(src);
    assert(size);

    struct node *record = alloc_record(src, size);
    if(!record) return NULL;

    struct node *tmp = head(list);
    if(!tmp) {
        *list = record;
    } else {
        record->next = tmp;
        tmp->previous = record;
    }

    return record;
}

struct node *append(struct node **list, void *src, size_t size) {
    assert(list);
    assert(src);
    assert(size);

    struct node *record = alloc_record(src, size);
    if(!record) return NULL;

    struct node *tmp = tail(list);
    if(!tmp) {
        *list = record;
    } else {
        tmp->next = record;
        record->previous = tmp;
    }
    return record;
}

struct node* insert(struct node **list, size_t i, void *src, size_t size) {
    assert(list);
    assert(i);
    assert(src);
    assert(size);
    size_t l = length(list);
    assert(i <= l + 1);


    struct node *record = alloc_record(src, size);
    if(!record) return NULL;

    struct node *tmp;

    if(i==1) {
        tmp = head(list);
        if(tmp) {
            tmp->previous = record;
            record->next = tmp;
        } else {
            *list=record;
        }
        
    } else if (i == l + 1 ) {
        tmp = tail(list);
        tmp->next = record;
        record->previous = tmp;

    } else {
        tmp = head(list);
        for (size_t j = 1; j < i; j++) {
            tmp = next(tmp);
        }
        record->previous = tmp->previous;
        record->next = tmp;
        (tmp->previous)->next = record;
        tmp->previous = record;

    }

    return record;
}


void delete(struct node **list, size_t i) {
    assert(list);
    assert(i);
    size_t l = length(list);
    assert(i <= l);

    struct node* tmp;
    struct node* tmp2;
    struct node* delnode;
    if(i==1) {
        delnode = head(list);
        tmp = next(delnode);
        if(tmp) {
            tmp->previous = 0; //If tmp is null, then the list has no next element.
        } else {
            *list = 0;
        }
        free_record(delnode);

    } else if (i==l) {
        delnode = tail(list);
        tmp = previous(delnode);
        tmp->next=0;
        free_record(delnode);

    } else {
        delnode = retrieve(list,i);
        tmp = previous(delnode);
        tmp2 = next(delnode);
        tmp->next = tmp2;
        tmp2->previous = tmp;
        free_record(delnode);

    }
    //maybe try to clean this up so it just uses pointers and not functions...

}


void isort(struct node **list, int (*comparison)(const void*,const void*)) {
    assert(list);
    assert(comparison);
    size_t l = length(list);
    assert(l);

    struct node *curr = {0};
    struct node *cmp = {0};
    int cmpr;

    for(size_t i = 1; i<=l; i++) {
        curr=retrieve(list,i);
        for(size_t j=i-1; j>0; j--) {
            cmp=retrieve(list,j);
            cmpr=comparison(curr,cmp);
            if(cmpr) {
                insert(list,i,curr->data,curr->size); //This may be wrong.
                delete(list,i+1);
            }
        }
    }
}




//void swap(struct list, size_t i, size_t j);
