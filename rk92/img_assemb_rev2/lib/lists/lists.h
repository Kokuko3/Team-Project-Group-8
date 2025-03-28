#ifndef _LISTS_H
#define _LISTS_H
#else
//#error "This file has already been included"
#endif

#include<stdint.h>
#include<stdlib.h>

/*
struct start_codon {
    uint8_t md5[32];
    uint16_t pc;
    struct data_codon *head; //tail
};

#define psize_max 256
struct data_codon {
    struct data_codon* prev; // previous data codon
    uint16_t pn; // packet number in image
    uint8_t psize; // intended size of memory block
    //uint8_t* payload; // pointer to memory block
    uint8_t payload[psize_max]; //
    struct data_codon* next; // next data codon
};

// This may get nixed
struct ack_format {
    int8_t status;
    uint16_t* resend;
};
enum statuses {new_img = -1, restart_img = 0};

*/

//void sort(struct data_codon* list);
//void insertion_sort(struct data_codon* list);

//struct data_codon* a
//struct data_codon* append(struct data_codon* list, struct data_codon element);
//struct data_codon* delete(struct data_codon* element);

struct node {
    struct node* previous;
    struct node* next;
    void* data;
    size_t size;
};


struct node **create();
void destroy(struct node **list);
void nullify(struct node **list);
struct node *head(struct node **list);
struct node *tail(struct node **list);
struct node *next(struct node *record);
struct node *previous(struct node *record);
size_t length(struct node **list);
struct node* retrieve(struct node **list, size_t i);
struct node *alloc_record(void *src, size_t size);
void free_record(struct node *record);
struct node *prepend(struct node **list, void *src, size_t size);
struct node *append(struct node **list, void *src, size_t size);
struct node* insert(struct node **list, size_t i, void *src, size_t size);
void delete(struct node **list, size_t i);

//index
//address
//sort that takes a call back? (also have something to check that the elements are have adjacent packet numbers).
