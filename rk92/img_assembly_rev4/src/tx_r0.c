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
char *serial_path = "/dev/ttyACM0";

#define n_images 2
char *image_path[n_images] = {"msg","msg2"}; //{"/home/nikolaus/Photos/khelmnytskyiUA.jpeg"};
struct start_packet images[n_images] = {0};
//struct start_packet *images = 0;
//char *tmp_img_path = "tmpfile";
//FILE* tmp_img_file;


size_t CIN = 0; //current image number
size_t CPN = 0; // current packet number
size_t TPC = 0;

int main() {    
    printf("Transceiver Start\nPID: %d\n\n",getpid());

    for(int i=0; i<n_images; i++) {
        FILE *load_img = fopen(image_path[i],"rb+");
        if(load_img==NULL) printf("Error opening file %s: Errno %d: %s\n",image_path[i],errno,strerror(errno));
        images[i].img_head=create();
        file2data(load_img,images[i].img_head);
        images[i].pc=length(images[i].img_head);
        hash(load_img,images[i].md5);
        assert(images[i].pc); // If an image is null, it will break the transmitter :3 sorry
        //remember to set up hash for images[i].md5.
        fclose(load_img);
    }
    
    serial_connection_t serial = serial_create_structure(serial_path);
    printf("Serial Link Status: %d\n",serial_link(&serial));

    transfer_t recv_transfer = {0};
    struct start_packet recv_start = {0};
    struct data_packet recv_data = {0};
    struct rack_packet recv_rack = {0};
    struct ack_packet recv_ack = {0}; 

    struct node *record = {0};
    struct start_packet trans_start = {0};
    struct data_packet trans_data = {0};
    struct rack_packet trans_rack = {0};
    struct ack_packet trans_ack = {0};
   
    // These serial statements are only for testing right now...
    //serial_connection_t serial_tx = serial_create_structure("/dev/ttyACM0");
    //printf("Serial Link Status: %d\n",serial_link(&serial_tx));


    struct pollfd readpoll[1] = {0};
    readpoll[0].fd = serial.serial_port;
    readpoll[0].events = POLLIN;
    struct pollfd writepoll[1] = {0};
    writepoll[0].fd=serial.serial_port;
    writepoll[0].events = POLLOUT;
    int readpoll_status = 0;
    int writepoll_status = 0;

    int readpoll_timeout_count = 0;
    int writepoll_timeout_count = 0;

    do {
        loopstart:
        if(CIN==n_images) break;
        if(CPN==0) {
            printf("Entering sendstart packet\n");
            sendstart:
            send_start_packet(&serial,images[CIN]);
            size_t start_timeout_count_threshold = 10;
            for(size_t i=0; i<start_timeout_count_threshold; i++) {
                readpoll_status = poll(&readpoll[0],1,50);
                if(readpoll_status<0) {
                    assert(0);
                } else if(readpoll_status==0) {
                    continue;
                } else {
                    if(serial_receive(&serial)<0) {
                        continue;
                    } else {
                        memset(&recv_transfer,0x00,sizeof(recv_transfer));
                        if(parse_buffer(&recv_transfer)<0) {
                            continue;
                        } else {
                            if(recv_transfer.id!=START) {
                                continue;
                            } else {
                                transfer2start(&recv_transfer,&recv_start);
                                if(!compare_start(images[CIN],recv_start)) {
                                    goto sendstart;
                                } else {
                                    CPN=1;
                                    goto loopstart;
                                }
                            }
                        }
                    }
                }
            }
            printf("Timed out of sendstart loop\n");
            // If there is a problem, I could put a goto sendstart or loopstart here...

        } else if((CPN >= 1) && (CPN <= images[CIN].pc)) {
            senddata:
            record = retrieve(images[CIN].img_head,CPN);
            node2data(record,&trans_data);
            send_data_packet(&serial,trans_data);
            size_t data_timeout_threshold = 10;
            for(size_t i=0; i<data_timeout_threshold; i++) {
                readpoll_status = poll(&readpoll[0],1,50);
                if(readpoll_status<0) {
                    assert(0);
                } else if(readpoll_status==0) {
                    continue;
                } else {
                    if(serial_receive(&serial)<0) {
                        continue;
                    } else {
                        memset(&recv_transfer,0x00,sizeof(recv_transfer));
                        if(parse_buffer(&recv_transfer)<0) {
                            continue;
                        } else {
                            if(recv_transfer.id!=DATA) {
                                continue;
                            } else {
                                transfer2data(&recv_transfer,&recv_data);
                                if(trans_data.pn!=recv_data.pn) {
                                    continue;
                                } else {
                                    CPN++;
                                    goto loopstart;
                                }
                            }
                        }
                    }
                }
            }
            printf("Timed out of senddata loop\n");

        } else if(CPN == images[CIN].pc + 1) {
            sendrack: // :3
            memset(&trans_rack,0x00,sizeof(trans_rack));
            send_rack_packet(&serial,trans_rack);
            size_t rack_timeout_threshold = 10;
            for(size_t i=0; i<rack_timeout_threshold; i++) {
                readpoll_status = poll(&readpoll[0],1,50);
                if(readpoll_status<0) {
                    assert(0);
                } else if(readpoll_status==0) {
                    continue;
                } else {
                    if(serial_receive(&serial)<0) {
                        continue;
                    } else {
                        memset(&recv_transfer,0x00,sizeof(recv_transfer));
                        if(parse_buffer(&recv_transfer)<0) {
                            continue;
                        } else {
                            if(recv_transfer.id!=ACK) {
                                continue;
                            } else {
                                transfer2ack(&recv_transfer,&recv_ack);
                                if(recv_ack.type==ACKNOWLEDGE) {
                                    CIN++;
                                }
                                CPN=0;
                                goto loopstart;
                            }
                        }
                    }
                }
            }
            // ACK
        } else {
            CPN=0;
        }

    loopend:
    } while(CIN < n_images);

    for(size_t i=0;i<100;i++) {
        send_ack_packet(&serial,create_tack());
    }
    printf("Exiting Transmitter\n");

    // Release control over serial.
    serial_delink(&serial);

    // Deallocate all of the image data.
    for(size_t i = 0; i<n_images; i++) {
        destroy(images[i].img_head);
    }

    return 0;
}


