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
char *serial_path = "/dev/ttyACM1";

size_t n_img = 0;
struct start_packet *img = 0;
FILE *tmp_img = 0;
FILE *cin_img = 0;

char fn_format[] = "file_%d";
#define fn_len 32
//char *tmp_img_path = "tmpfile";
//FILE* tmp_img_file;

size_t CIN = 0;
size_t CPN = 0;

int mode = 0;

int grow_img() {
    struct start_packet *tmp_img = realloc(img,sizeof(struct start_packet)*(n_img+1));
    if(!tmp_img) {
        printf("grow_img(): error growing image\n");
        return -1;
    } else {
        img=tmp_img;
        n_img++;
        memset(&img[n_img-1],0x00,sizeof(struct start_packet));
        img[n_img-1].img_head=create();
        return 1;
    }

}

int main(int argc, char *argv[]) {    
    printf("Receiver Start\nPID: %d\n\n",getpid());

    if(argc<2) {
        printf("Missing Serial Arguments\n");
        return -1;
    }

    printf("Allocating Space for First File\n");
    grow_img();
    assert(img);


    serial_connection_t serial = serial_create_structure(argv[1]);
    printf("Serial Link Status: %d\n",serial_link(&serial));

    transfer_t recv_transfer = {0};
    struct start_packet recv_start = {0};
    struct data_packet recv_data = {0};
    struct rack_packet recv_rack = {0};
    struct ack_packet recv_ack = {0}; 

    //struct node *record = {0};
    //struct start_packet trans_start = {0};
    //struct data_packet trans_data = {0};
    //struct rack_packet trans_rack = {0};
    //struct ack_packet trans_ack = {0};
   

    int p_type = 0;
    while(1) {
        tcflush(serial.serial_port,TCIFLUSH);
        p_type = poll_packet(&serial, 1, 250, &recv_transfer);
        if(p_type == NO_PACKET) {
            //Build to exit condition.
            continue;

        } else if(p_type == START) {
            // Deallocate previous data packet list.
            if(img[CIN].img_head) destroy(img[CIN].img_head);

            // Impant new start packet, create data packet list, and reset current packet number.
            transfer2start(&recv_transfer,&img[CIN]);
            img[CIN].img_head = create();
            CPN = 0;

            // Confirm to receiver that packet was received.
            send_start_packet(&serial,img[CIN]);
            continue;

        } else if(p_type == DATA) {
            assert(img[CIN].img_head);

            transfer2data(&recv_transfer,&recv_data);
            if(recv_data.pn == CPN + 1) {
                // Append the next data packet to the list.
                CPN+=1;
                insert(img[CIN].img_head,CPN,&recv_data,sizeof(recv_data));
                
                // Send acknowledgement that the packet was accepted
                send_data_packet(&serial,create_dack(CPN));
            } else {
                // send nack, junk, or nothing...
            }

            continue;

        } else if(p_type == RACK) {
            assert(img[CIN].img_head);

            // Don't create empty files.
            if(CPN==0) continue;

            // Store image data to tempfile
            tmp_img = tmpfile();
            data2file(tmp_img,img[CIN].img_head);

            // Compare hashes. If mismatch, close out of the tempfile; otherwise, save the file and move on.
            if(hash(tmp_img,img[CIN].md5)<0) {
                fclose(tmp_img);

            } else {
                send_ack_packet(&serial,create_pack());
                grow_img();

                char fn[fn_len] = "";
                snprintf(fn,sizeof(fn),fn_format,CIN);
                cin_img = fopen(fn,"wb+");
                copyfile(tmp_img,cin_img);
                fclose(tmp_img);
                fclose(cin_img);
                CIN++;
                CPN = 0;

            }

            continue;

        } else if(p_type == ACK) {
            assert(img[CIN].img_head);

            transfer2ack(&recv_transfer,&recv_ack);
            if(recv_ack.type==TERMINATE) {
                printf("Termination Request Received\n");
                break;
            }
            continue;

        } else {
            assert(0); // All cases should be covered.

            continue;
        }


    }

    // 
    printf("Exiting the Receiver\n");

    // Close the serial connection
    serial_delink(&serial);

    // Deallocate all of the image data.
    for(size_t i = 0; i<n_img; i++) {
        destroy(img[i].img_head);
    }

    // Free start of images
    free(img);


    return 0;
}