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

/* Other Headers*/
#include <openssl/evp.h>

/* Local Libary Includes*/
//#include "../lib/transfer/transfer.h"
#include "../lib/local/local.h"
#include "../lib/lists/lists.h"


/* Function Declarations */

/* Global Variables */
/*
    Description
*/

int main() {

    printf("Hello World!\n");
    printf("PID: %d\n\n",getpid());

    uint8_t h[16] = {0};
    FILE *fp = fopen("makefile","rb+");
    hash(fp,h);
    fclose(fp);
    int i = hashfile("makefile",h);
    printf("The result is %d:",i);

    if(0) {
        struct node **slist = 0;
        struct node **rlist = 0;
        struct node *record = 0;
        slist = create();
        rlist = create();

        char fin[] = "src/main.c"; //"lokomotive293.jpg"; //"src/main.c";
        char fout[] = "out.txt";

        // Open files
        FILE *fr = fopen(fin, "rb+");
        FILE *fw = fopen(fout,"wb+");
        FILE *fw_d = fopen("debug","wb+");

        // Initialise serial link
        serial_connection_t serial1 = serial_create_structure("/dev/ttyACM2");
        serial_connection_t serial2 = serial_create_structure("/dev/ttyACM3");

        int status1 = serial_link(&serial1);
        int status2 = serial_link(&serial2);
        printf("Link Status (1): %d\n",status1);
        printf("Link Status (2): %d\n",status2);
        sleep(2);

        struct timespec remaining = {0};
        struct timespec request = { 0, 0002}; 

        struct data_packet tmp = {0};
        struct data_packet rtmp = {0};
        transfer_t trans = {0};

        file2data(fr,slist);
        size_t l = length(slist);
        for(size_t i=1; i<=l; i++) {
            record = retrieve(slist,i);
            node2data(record,&tmp);
            send_data_packet(&serial1,tmp);
            nanosleep(&request,&remaining);

            retry:
            serial_receive(&serial2);
            if(parse_buffer(&trans)>0) {
                transfer2data(&trans,&rtmp);
                printf("PN: %d, PSIZE: %d CONTENTS: ",rtmp.pn,rtmp.psize);
                for(int i = 0; i<rtmp.psize; i++) printf("%c",rtmp.payload[i]);
                printf("\n");
            } else {
                sleep(1);
                //nanosleep(&request,&remaining);
                goto retry; 
            }
        }


        // Deinitialise serial link
        status1 = serial_delink(&serial1);
        printf("Delink Status (1): %d\n",status1);
        status2 = serial_delink(&serial2);
        printf("Delink Status (2): %d\n",status2);

        // Close files
        fclose(fr);
        fclose(fw);
        fclose(fw_d);

        // Destroy Lists
        destroy(slist);
        destroy(rlist);
        return 0;
    }
}


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
