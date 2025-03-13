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
    printf("Hello World\n");
/*
    int master=0;
    int slave=0;
    struct termios tty={0}; 
    struct winsize winp={0};
    int status = openpty(&master, &slave,"pty",&tty,&winp);
*/

    uint8_t mbuff[512]={0};

    serial_connection_t serial = serial_create_structure("/dev/ttyACM0");
    int status = serial_link(&serial);
    printf("Status: %d\n",status);

    transfer_t received = {0};
    transfer_t received1 = {0};
    transfer_t bob = {0};
    bob.id=69;
    bob.size=32;
    memset(&bob.payload,11,32);
    bob.payload[31]=69;


    //serial_tx_struct(&serial,bob);
    uint8_t wbuff[512]={0};
    int i;
    for(i=0; i<500; i++) {
        wbuff[i]=i;   
    }
    //int st1=serial_ewrite(&serial,wbuff,i);
    //sleep(1);
    //serial_receive(&serial,mbuff,sizeof(mbuff));
    int st1=serial_ewrite(&serial,wbuff,i);
    sleep(2);
    int st2=serial_struct_tx(&serial,bob);
    sleep(2);
    int st3=serial_ewrite(&serial,wbuff,i);
    sleep(2);
    bob.id=42;
    int st4=serial_struct_tx(&serial,bob);
    sleep(2);
    int st5=serial_ewrite(&serial,wbuff,i);
    sleep(2);
    int bytes2read=0;
    ioctl(serial.serial_port, FIONREAD, &bytes2read);
    printf("The number of bytes to read is %d\n",bytes2read);
    do{
        serial_receive(&serial,mbuff,sizeof(mbuff));
        i=parse_buffer(&received);
        printf("Parse Status: %d ID: %d\n",i,received.id);
        sleep(2);
    } while(1);

    //serial_read(&serial,&mrbuff[0],sizeof(mrbuff));

    status = serial_delink(&serial);
    printf("Status: %d\n",status);
    //printf("The received IDs were: %d %d\n",received.id,received1.id);

    //printf("The size of the transfer structure %d\n",(int) sizeof(transfer_t));
    return 0;
}


