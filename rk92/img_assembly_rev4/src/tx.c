/* STD Library Includes */
#include<string.h>
#include<errno.h>
#include<stdio.h>
#include<stdint.h>
#include<assert.h>

/* POSIX Headers */
#include<pty.h>
#include<termios.h>
#include<unistd.h>
#include<time.h>
#include<poll.h>

/* Other Headers*/
#include <openssl/evp.h>

/* Local Libary Includes*/
//#include "../lib/transfer/transfer.h"
#include "../lib/local/local.h"
#include "../lib/lists/lists.h"


/* Function Declarations */

/* Global Variables */
size_t CIN = 0; //current image number
size_t CPN = 0; // current packet number

int n_img = 0;
struct start_packet *img = 0;
char *serial_device = 0;

int main(int argc, char *argv[]) {    
    printf("Transceiver Start\nPID: %d\n",getpid());

    if(argc==1) {
        printf("Missing Serial and File Arguments\n");
        return -1;
    } else if(argc<=2) {
        printf("Missing File Arguments\n");
        return -1;
    }

    printf("\vFile Inputs:\n");
    for(int i = 2; i<argc; i++) {
        printf("argv[%d] -> %s\n",i,argv[i]);
    }

    printf("\vAllocating Space for %d Files\n",argc-2);
    img = calloc(argc-2,sizeof(struct start_packet));
    n_img = argc-2;
    if(!img) {
        printf("Error Allocating Space For %d Files\n",n_img);
        return -1;
    } 

    for(int i = 0; i<argc-2; i++) {
        FILE *tmp = fopen(argv[i+2],"rb+");
        if(!tmp) {
            printf("Error Reading File %s: Errno %d: %s\n",argv[i+2],errno,strerror(errno));
            return -1;
        } else {
            img[i].img_head=create();
            file2data(tmp,img[i].img_head);
            img[i].pc=length(img[i].img_head);
            // There may need to be an assert for a zero packet count.
            hash(tmp,img[i].md5);
            fclose(tmp);
        }
    }
    
    serial_device = argv[1];
    serial_connection_t serial = serial_create_structure(serial_device);
    printf("Serial Link Status: %d\n",serial_link(&serial));

    transfer_t recv_transfer = {0};
    struct start_packet recv_start = {0};
    struct data_packet recv_data = {0};
    //struct rack_packet recv_rack = {0};
    struct ack_packet recv_ack = {0}; 

    struct node *record = {0};
    //struct start_packet trans_start = {0};
    struct data_packet trans_data = {0};
    struct rack_packet trans_rack = {0};
    //struct ack_packet trans_ack = {0};

    int p_type = 0;
    while(CIN < n_img) {
        p_type = poll_packet(&serial,2,250,&recv_transfer);
        if(p_type == START) { // This would allow the receiver to tell the transmitter to restart by sending a wrong data packet...
            transfer2start(&recv_transfer,&recv_start);
            if(!compare_start(img[CIN],recv_start)) {
                CPN = 0;
                tcflush(serial.serial_port,TCIOFLUSH);

            } else {
                CPN = 1;

            }
            //continue;

        } else if(p_type == DATA) {
            transfer2data(&recv_transfer,&recv_data);
            if(recv_data.pn == CPN) {
                CPN++;
                tcflush(serial.serial_port,TCIOFLUSH);

            } else {
                // Build towards some threshold to restart image.

            }
            //continue;

        } else if(p_type == ACK) {
            transfer2ack(&recv_transfer,&recv_ack);
            if((recv_ack.type == ACKNOWLEDGE) && (img[CIN].pc + 1 == CPN)) {
                CIN++;
                CPN = 0;
                tcflush(serial.serial_port,TCIOFLUSH);

            } else {
                CPN = 0;

            }
            //continue;

        } else if(p_type == RACK) {
            // Do nothing :3

        } else { // Send Information
            tcflush(serial.serial_port,TCOFLUSH);
            if(CPN == 0) {
                send_start_packet(&serial,img[CIN]);

            } else if(CPN<=img[CIN].pc) {
                record = retrieve(img[CIN].img_head,CPN);
                node2data(record,&trans_data);
                send_data_packet(&serial,trans_data);

            } else {
                send_rack_packet(&serial,trans_rack);

            }
            //sleep(1);
            nsleep(100000000);

        }

    }
   
    for(size_t i=0;i<100;i++) {
        send_ack_packet(&serial,create_tack());
    }
    printf("Exiting Transmitter\n");

    // Release control over serial.
    serial_delink(&serial);

    // Deallocate all of the image data.
    for(size_t i = 0; i<n_img; i++) {
        destroy(img[i].img_head);
    }

    return 0;
}

