#ifndef _TRANSFER_H
#define _TRANSFER_H
#else
#error "This file has already been included"
#endif

#include<stdint.h>
#include<termios.h>

enum yellow_pages {
    transfer_id = 1
};

struct transfer {
    uint8_t id;
    uint8_t size;
    uint8_t payload[256];
};
typedef struct transfer transfer_t;


struct serial_connection {
    int serial_port;
    char* device;
    struct termios tty;
};
typedef struct serial_connection serial_connection_t;

serial_connection_t serial_create_structure(char* device);
int serial_link(serial_connection_t*);
int serial_delink(serial_connection_t*);
int serial_write(serial_connection_t*,uint8_t*,uint16_t);
int serial_ewrite(serial_connection_t*,uint8_t*,uint16_t);
int serial_read(serial_connection_t*,uint8_t*,uint16_t);
int serial_struct_tx(serial_connection_t*, transfer_t);
int serial_struct_rx(serial_connection_t*, transfer_t*);
int serial_receive(serial_connection_t*, uint8_t*, uint16_t);
int parse_buffer(transfer_t* structure);