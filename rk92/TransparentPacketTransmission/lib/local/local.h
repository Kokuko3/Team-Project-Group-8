#ifndef _LOCAL_H
#define _LOCAL_H
#else
#error "This file has already been included"
#endif

#include<stdint.h>

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



void sort(struct data_codon* list);
void insertion_sort(struct data_codon* list);

//struct data_codon* a
//struct data_codon* append(struct data_codon* list, struct data_codon element);
//struct data_codon* delete(struct data_codon* element);

