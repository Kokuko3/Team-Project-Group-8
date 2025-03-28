#ifndef _LOCAL_H
#define _LOCAL_H
#else
#error "This file has already been included"
#endif

#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include<openssl/evp.h>
#include "../transfer/transfer.h"
#include "../lists/lists.h"

enum {
    MODE_DETERMINE_IMG_COUNT = 10,
    MODE_SEND_IMG_COUNT = 20,
    MODE_GET_IMG_COUNT = 30,
    MODE_SEND_START = 40,
    MODE_SEND_DATA = 50,
    MODE_SEND_RACK = 60,
    MODE_GET_IMAGES = 70
};

enum {
    START = 10,
    DATA = 20,
    RACK = 30,
    ACK = 40
};

struct start_packet {
    uint8_t md5[16];
    uint32_t pc;
    struct node **img_head;
};

#define psize_max 16
struct data_packet {
    uint32_t pn;
    uint8_t psize;
    uint8_t payload[psize_max];
};

struct rack_packet {
    uint8_t type;
    int8_t request;
};

struct ack_packet {
    uint8_t type;
    int8_t response;
    uint8_t psize;
    uint8_t payload[psize_max];

};

enum {
    ACKNOWLEDGE,
    NACKNOWLEDGE
};

/*
    Consider adding a data packet acknowledge;
*/

int hash(FILE *fr, uint8_t md5[16]);
int hashfile(char *filepath, uint8_t md5[32]);
void node2data(struct node *record, struct data_packet *data);
void file2data(FILE *fp, struct node **list);
void data2file(FILE *fp, struct node **list);
void file2list(FILE *fp, struct node **list);
void list2file(FILE *fp, struct node **list);

int send_start_packet(serial_connection_t *serial, struct start_packet start);
int send_data_packet(serial_connection_t *serial, struct data_packet data);
int send_ack_packet(serial_connection_t *serial, struct ack_packet ack);
int send_rack_packet(serial_connection_t *serial, struct rack_packet rack);

void transfer2data(transfer_t *trans,struct data_packet *data);
void transfer2start(transfer_t *trans, struct start_packet *start);
void transfer2rack(transfer_t *trans, struct rack_packet *rack);
void transfer2ack(transfer_t *trans, struct ack_packet *ack);

int compare_start(struct start_packet s1, struct start_packet s2);

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