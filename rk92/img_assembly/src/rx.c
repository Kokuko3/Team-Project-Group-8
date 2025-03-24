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
    printf("Allocating Space For First Image: %p", (void *) images);
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
    int writepoll_timeout_count = 0;

    while(1) {

        sleep(0);
        send:
        writepoll_status = poll(writepoll,1,100);
        if(writepoll_status < 0) {
            printf("Error polling for write\n%d: %s\n",errno,strerror(errno));
        } else if(writepoll_status == 0) {
            writepoll_timeout_count++;
        } else {
            if(writepoll[0].revents & POLLOUT) {
                /*
                    Here is where you place or set something when the time comes
                    to send data that is not purely reactive to something that
                    was received.
                */
                //send_start_packet(&serial_tx,trans_start);
                //trans_start.pc+=1;
                //send_data_packet(&serial_tx,trans_data);
                //trans_data.pn=current_data_id+1;
            } else {
                printf("Non-POLLOUT result from write poll\n");
                assert(0);
            }
        }
        
        receive:
        // it might be worth changing this to a for or while loop that breaks once readpoll_timeout_count gets past some value...
        // add a block that checks if there is something to read until a timeout, if not goes to transmit
        readpoll_status = poll(readpoll,1,250); // I wonder how this handles multiple packets being available to read...
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
                        if(!hashfile(tmp_img_path,images[current_img_id].md5)) {
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


        /*
        serial_receive(&serial);
        if(parse_buffer(&recv_transfer)>0) {
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
                continue;
            }

            switch(recv_transfer.id) {
            case START:
                if(images[current_img_id].img_head) destroy(images[current_img_id].img_head);
                images[current_img_id]=recv_start;
                images[current_img_id].img_head=create();
                send_start_packet(&serial,images[current_img_id]);
                break;
            case DATA:
                // This block could see the send data packet-trans data stuff moved out of the block...
                if(recv_data.pn == current_data_id + 1) {
                    append(images[current_img_id].img_head,&recv_data,sizeof(recv_data));
                    current_data_id += 1;
                    trans_data.pn = current_data_id;
                    trans_data.psize = 0;
                    send_data_packet(&serial,trans_data);
                } else if (recv_data.pn == current_data_id) {
                    delete(images[current_img_id].img_head,current_data_id);
                    append(images[current_img_id].img_head,&recv_data,sizeof(recv_data));
                    trans_data.pn = current_data_id;
                    trans_data.psize = 0;
                    send_data_packet(&serial,trans_data);
                } else {
                    // Update transmitter code to be able to go backwards? Hopefully not necessary...
                    trans_data.pn = current_data_id;
                    trans_data.psize = 0;
                    send_data_packet(&serial,trans_data);
                }
                break;
            case RACK:
                    if(length(images[current_img_id].img_head) == images[current_img_id].pc) {
                        tmp_img=tmpnam(tmp_img);
                        tmpfp = fopen(tmp_img,"wb+");
                        data2file(tmpfp,images[current_img_id].img_head);
                        fclose(tmpfp);

                        if(1) {
                            rename(tmp_img,"file");
                            trans_ack.type=ACKNOWLEDGE;
                            trans_ack.response=0;
                            trans_ack.psize=0;
                            send_ack_packet(&serial,trans_ack);

                        } else {
                            remove(tmp_img);
                            trans_ack.type=NACKNOWLEDGE;
                            trans_ack.response=0;
                            trans_ack.psize=0;
                            send_ack_packet(&serial,trans_ack);
                        }

                    } else {
                        trans_ack.type=NACKNOWLEDGE;
                        trans_ack.response=0;
                        trans_ack.psize=0;
                        send_ack_packet(&serial,trans_ack);
                    }
                break;
            case ACK:
                printf("Received ACK, receiver not setup to process that yet...\n");
                break;
            default:
                assert(0);
            }
        }

        transmit: //arguably t
        /*
        //check the current_img_id exists in the range so the rest of this doesn't blow up...
        assert(current_img_id <= n_images);
        switch(mode) {
        case MODE_SEND_START:
            break;
        case MODE_SEND_DATA:
            break;
        case MODE_SEND_RACK:
            break;
        case MODE_DETERMINE_IMG_COUNT:
            break;
        case MODE_GET_IMG_COUNT:
            break;
        case MODE_GET_IMAGES:
            mode
            break;
        default:
            printf("mode=%d",mode);
            assert(0);
        }
            */


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


void transmitter() {
    // setup
    // create array for the final names of each image
    // create an array for the start packet of each image
    // create an array for the data packets of each image
    // set current_image_id = 0;
    // set current_data_id = 1;
    // MODE {START,DATA} = START

    /*
    for i in 0 to number of images-1 {
        open file name
            create list for current image
            read image to the list
            close file name
        write length of list to packet count in start of current image
        hash the file
    }

    while(current_image_id<number of images) {
        attempt to receive serial data
        parse serial data for packets
        if packet,
            switch(transfer_packet.type) {
            case start
                convert transfer packet to start packet
            case data
                convert transfer packet to data packet
            case rack
                convert transfer packet to rack packet
            case ack
                convert transfer packet to ack packet
            default
                goto attempt to receive serial data
            }

            switch(transfer_packet.type) {
                case start
                    if start packet matches the one sent, set mode = DATA
                    else resend the start packet, set mode = START

                case data
                    if data packet ID == the current data packet ID, current data packet ID+=1.
                    ^^This received data packet should have a payload of 0 length.

                case rack
                    send acknowledge of total images to be sent and current image index

                case ack
                    if ack and ID matches, current image ID+=1 and data packet ID=0.
                default
                    assert(0)
            }
        }
    
        switch(mode)
        ...............

        if current data packet ID 0,
            send start packet of image ID.
        else 1 <= current data packet ID <= length(list)
            send current data packet
        else if (current data packet ID > list)
            send rack for current image.
        else
            assert(0)
        end
    }
    */



}

void receiver() {
    // setup
    // array for start of images
    // create list for each image.
    // last id of packet = 0;


    //attempt to receive serial data
    // parse serial data for packets
    // if no packet, go to receive serial data.
    // if packet,
    /*
        switch(transfer_packet.type)
        case start
            convert transfer packet to start packet
        case data
            convert transfer packet to data packet
        case rack
            convert transfer packet to rack packet
        default:
            goto attempt to receiver serial data
    */
    /*
        switch(transfer_packet.type)
        case start
            nullify(list) of current index in start packet array
            assign the start packet to the current
            send data packet for ID = 0 or start packet matching the received start packet
        case data packet
            check to see if ID == last data packet ID
                if not, append data packet to the list of data
            
                if they match,
                delete the last data packet (which is necessarily associated with ID)
                append the data packet for ID.
            send data packet with the received ID.
        case rack
            check to see the the length of data packet list == packett count in start packet.
                if yes, assemble image.
                if not, send nack with current image ID.
            check to see that the hashes match,
                if yes,
                    send ack with the current image ID
                    increment ID.
                if no,
                    delete the assembled image.
                    send nack with the current image ID.
        case ack
            terminate program or terminate if a timer has expired because it hasn't received any packets
        default
            assert(0)

    */
}
