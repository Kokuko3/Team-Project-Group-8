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

size_t n_images = 1;
struct start_packet *images = 0;
char *tmp_img_path = "tmpfile";
FILE* tmp_img_file;

size_t current_img_id = 0;
size_t current_data_id = 0;

int mode = 0;

int main() {    
    printf("Receiver Start\nPID: %d\n\n",getpid());

    images = realloc(images,sizeof(struct start_packet)*(n_images));
    printf("Allocating Space For First Image: %p\n", (void *) images);
    assert(images);
    images[current_img_id].pc=0;
    images[current_img_id].img_head=0;

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

    while(1) {

        receive:
        // it might be worth changing this to a for or while loop that breaks once readpoll_timeout_count gets past some value...
        // add a block that checks if there is something to read until a timeout, if not goes to transmit
        readpoll_status = poll(readpoll,1,50); // I wonder how this handles multiple packets being available to read...
        if(readpoll_status < 0) {
            printf("Error polling for read\n%d: %s\n",errno,strerror(errno));
        } else if(readpoll_status == 0) {
            readpoll_timeout_count++;
        } else {
            if(readpoll[0].revents & POLLIN) {
                serial_receive(&serial);
                if(parse_buffer(&recv_transfer)<=0) {
                    printf("Failed to parse packet from within read poll\n");
                } else {
                    readpoll_timeout_count=0;
                    switch(recv_transfer.id) {
                    case START:
                        transfer2start(&recv_transfer,&recv_start);
                        break;
                    case DATA:
                        transfer2data(&recv_transfer,&recv_data);
                        break;
                    case RACK:
                        transfer2rack(&recv_transfer,&recv_rack);
                        break;
                    case ACK:
                        transfer2ack(&recv_transfer,&recv_ack);
                        break;
                    default:
                        printf("Entering default case for received transfer packet\n"); //I entered here... how?
                        assert(0);
                        break;
                    }

                    switch(recv_transfer.id) {
                    case START:
                        /*
                            If a start packet is received, delete data of the current image,
                            load the new start packet into the current image,
                            create a list to store image contents,
                            and update the current data ID to reflect that it's an empty image.

                            Then send back the image packet as an acknowledgement.
                        */
                        if(images[current_img_id].img_head) destroy(images[current_img_id].img_head);
                        images[current_img_id]=recv_start;
                        images[current_img_id].img_head=create();
                        current_data_id=0;
                        send_start_packet(&serial,images[current_img_id]);
                        break;

                    case DATA:
                        /*
                            If there is not a list to store data, create one.
                            If the next packet has arrived, append it to the list and increment the current packet its on.
                            If a packet of the furrent packet ID has arrived, delete it from the list and reappend it.
                            If an out of order packet has been received, print that it happened.

                            Regardless, send out the current data ID to verify that the right packet was received
                            to the transmitter.
                        */
                        if(!images[current_img_id].img_head) { // This is kinda shoogly :(
                            images[current_img_id].img_head=create();
                        }
                        if(recv_data.pn == current_data_id + 1) {
                            printf("Received next data packet\n");
                            ++current_data_id;
                            append(images[current_img_id].img_head,&recv_data,sizeof(recv_data));

                        } else if((recv_data.pn == current_data_id) && (current_data_id > 0)) {
                            printf("Received duplicate of current data packet\n");
                            delete(images[current_img_id].img_head,current_data_id);
                            insert(images[current_img_id].img_head,current_data_id,&recv_data,sizeof(recv_data));
                        } else {
                            printf("Received out of order data packet\n");
                        }
                        trans_data.pn=current_data_id;
                        trans_data.psize=0;
                        send_data_packet(&serial,trans_data);
                        break;

                    case RACK:
                        /*
                            If the list doesn't exist for the current image, create it.
                            Open a temp file for the image.
                            Assemble the image.
                            Close the temp file.
                            If the hash does not match the start packet, send nack and delete the image.
                            If the hash does match, send rename the file, move to the next image,
                                reset current data packet, and send ack packet.
                        */
                        if(!images[current_img_id].img_head) {
                            images[current_img_id].img_head=create();
                        }
                        tmp_img_file = fopen(tmp_img_path,"wb+");
                        data2file(tmp_img_file,images[current_img_id].img_head);
                        fclose(tmp_img_file);
                        if(hashfile(tmp_img_path,images[current_img_id].md5)>0) {
                            trans_ack.type=NACKNOWLEDGE;
                            trans_ack.psize=0;
                            send_ack_packet(&serial,trans_ack);
                            remove(tmp_img_path);
                        } else {
                            // Rename
                            char fn[32] = "";
                            snprintf(fn,sizeof(fn),"img_%ld.jpg",current_img_id);
                            rename(tmp_img_path,fn);

                            // Setup next image
                            n_images+=1;
                            images = realloc(images,sizeof(struct start_packet)*(n_images));
                            current_img_id+=1;
                            current_data_id=0;
                            memset(&images[current_img_id],0x00,sizeof(images[current_img_id]));
                            //Realloc does not zero the new memory, so this can cause it to pass a null check and point to nowhere and segfault...

                            // Send acknowledgement
                            trans_ack.type=ACKNOWLEDGE;
                            trans_ack.psize=0;
                            send_ack_packet(&serial,trans_ack);
                        }
                        break;
                    case ACK:
                        break;
                    default:
                        assert(0);
                        break;
                    }
                }
            } else {
                printf("Non-POLLIN result from read poll\n");
                assert(0);
            }
        }
    }


    serial_delink(&serial);

    // Deallocate all of the image data.
    for(size_t i = 0; i<=n_images; i++) {
        destroy(images[i].img_head);
    }

    // Free start of images
    free(images);

    return 0;
}