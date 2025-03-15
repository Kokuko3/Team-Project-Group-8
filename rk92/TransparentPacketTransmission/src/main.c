/* STD Library Includes */
#include<string.h>
#include<stdio.h>
#include<stdint.h>
#include<assert.h>

/* POSIX Headers */
#include<pty.h>
#include<termios.h>
#include<unistd.h>
//#define _POSIX_C_SOURCE 200809L
#include<time.h>

/* Local Libary Includes*/
#include "../lib/transfer/transfer.h"
#include "../lib/local/local.h"


/* Function Declarations */

/* Global Variables */
/*
    Description
*/

int main() {

    printf("Hello World!\n");
    printf("PID: %d\n\n",getpid());
    sleep(2);
    // Initialise serial link
    serial_connection_t serial1 = serial_create_structure("/dev/ttyACM0");
    serial_connection_t serial2 = serial_create_structure("/dev/ttyACM1");
    int status = serial_link(&serial1);
    printf("Link Status (1): %d\n",status);
    status = serial_link(&serial2);
    printf("Link Status (2): %d\n",status);
    sleep(2);

    struct timespec remaining = {0};
    struct timespec request = { 0, 0002}; 

    transfer_t tx={0};
    transfer_t rx={0};
    int old_size=-1;
    int max=256;

    tx.id=1;
    //char msg[]="0123456789 123456789 ";
    char msg[]="0123456789 123456789";
    tx.size=sizeof(msg);
    memcpy(tx.payload,msg,tx.size);
    
    //sleep(1);
    
    //sleep(5);
    //tcdrain(serial1.serial_port);
    //sleep(10);
    uint16_t i = 0;
    unsigned long j = 0;
    uint64_t k = 0;
    for(;;) {
        resend:
        serial_struct_tx(&serial1,tx);
        printf("Packet written\n");
        for(;;) {
            //sleep(1);
            if(ioctl(serial2.serial_port, FIONREAD,&j)<0) {
                printf("Slowing loop\n");
            } else if (j==0) {
                nanosleep(&request,&remaining);
                i++;
                if(i>250) { //1000 is good for a 21 byte payload...
                    i=0;
                    goto resend;
                }
                continue; 
            } else {
                //printf("There are %d bytes to read\n",j);
            }
            if(serial_receive(&serial2)>0) {
                int s = parse_buffer(&rx);
                if(s>0) {
                    printf("%06d: Packet ID: %3d, Size: %d, Payload: %s\n",k,rx.id,rx.size,rx.payload);
                    tx.id++;
                    k++;
                    i=0;
                    goto resend;
                } else {
                    
                    //printf("Rip in peace\n");
                }
            }

        }

    }
    

    // Deinitialise serial link
    status = serial_delink(&serial1);
    printf("Delink Status (1): %d\n",status);
    status = serial_delink(&serial2);
    printf("Delink Status (2): %d\n",status);



    return 0;
}


