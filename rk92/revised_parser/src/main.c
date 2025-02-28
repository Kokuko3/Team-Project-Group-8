/* STD Library Includes */
#include<string.h>
#include<stdio.h>
#include<stdint.h>
#include<assert.h>

/* POSIX Headers */
#include<pty.h>
#include<termios.h>
#include<unistd.h>

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
    // Initialise serial link
    serial_connection_t serial = serial_create_structure("/dev/ttyACM0");
    int status = serial_link(&serial);
    printf("Status: %d\n",status);

    transfer_t rx={0};
    int old_size=-1;
    int max=256;

    while(1) {
        sleep(1);
        serial_receive(&serial);
        //sleep(1);
        int s = parse_buffer(&rx);
        if(s>0) {
            printf("%d: The size field reads %d\n",s,rx.size);
            printf("\t%d: The next size should have been %3d; it was %3d\n",(old_size+1)%max == rx.size,(old_size+1)%max,rx.size);
            old_size=rx.size;
        }
    }

    // Deinitialise serial link
    status = serial_delink(&serial);
    printf("Status: %d\n",status);

    return 0;
}


