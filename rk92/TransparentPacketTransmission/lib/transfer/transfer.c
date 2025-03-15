/* Includes */
//Associated Header
#include "transfer.h"

// C Standard Library Headers
#include <stdio.h>  // provides printf
#include <string.h> // provides memcpy, memset, etc.

// POSIX Headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/ioctl.h>

#define rbf_size 256
#define wbf_size 256
static uint8_t rbf[rbf_size];
static int8_t wbf[wbf_size];
static uint8_t rbfi=0;
static uint8_t wbfi=0;

#define startp_size 4
#define stopp_size 4
static uint8_t startp[startp_size]={0x01,0x02,0x03,0x02};
static uint8_t stopp[stopp_size]={0x04,0x04,0x05,0x06};



serial_connection_t serial_create_structure(char* device) {
    serial_connection_t serial = {0};
    serial.device=device;
    return serial;
}

int serial_link(serial_connection_t* serial) {
    // Get File Descriptor of Serial Port
    serial->serial_port=open(serial->device, O_RDWR | O_EXCL); //open ttyACM0 read/write

    // Check that the serial port opened correctly.
    if(serial->serial_port<0) {
        printf("Error %i from open: %s\n",errno,strerror(errno)); 
        //If open throws a bad value, print the error number and error text.
        return -1;
    }
    struct termios tty;

    // Read preconfigured state. Throw error on failure.
    if(tcgetattr(serial->serial_port, &tty) != 0) {
        printf("error %i from open: %s\n",errno,strerror(errno));
        return -1;
    }

    int old_flags = fcntl(serial->serial_port, F_GETFL, 0);
    int status = fcntl(serial->serial_port,F_SETFL,old_flags | O_NONBLOCK);
    int new_flags = fcntl(serial->serial_port,F_GETFL,0);
    /* Configure TTY */  
    // Control Modes - c_cflags
    /*
    tty.c_cflag &= ~PARENB; // Disables Parity Bit
    tty.c_cflag &= ~CSTOPB; // Use Single Stop Bit
    tty.c_cflag |= CS8; //Sets bits in byte to 8.
        //tty.c_cflag &= ~CRTSCTS; // This is not defined in posix...
    tty.c_cflag |= CREAD; // Turn on read. 
    tty.c_cflag |= CLOCAL; // Disable modem control lines.

    // Local Modes - c_lflags
    tty.c_lflag &= ~ICANON; // Disable cannonical form -> preserves binary data
    tty.c_lflag &= ~ECHO; // Disables Echoing
    tty.c_lflag &= ~ECHOE; // Disables Erasurep via Erase Character
    tty.c_lflag &= ~ECHOK; // Disables Kill Character Erasure of Line
    tty.c_lflag &= ~ECHONL; // Disable New-Line Echo
    tty.c_lflag &= ~ISIG; // Disables INTR, QUIT, SUSP, DSUSP control characters interpretation.

    // Input Modes - c_iflags
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable Software Flow Control
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable special character handling

    // Output Modes - c_oflags
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    // Terminal Special Characters - c_cc
    tty.c_cc[VTIME] = 0; // [=] Deciseconds - timeout for noncannonical reads.
    tty.c_cc[VMIN] = 0; // [=] Bytes - Minimum number of bytes for noncannonical read.
    /*
        VMIN=0, VTIME=0 - Read() returns whatever is in the buffer instantly -> polling
        VMIN>0, VTIME=0 - Read() waits for VMIN bytes before returning.
        VMIN=0, VTIME>0 - Read() returns any amount of incoming data until VTIME timeout.
        VMIN>0, VTIME>0 - Read() blocks until either VMIN bytes have been received VTIME has elapsed between characters/bytes.

        I think VMIN>0,VTIME>0 will work well for this application -> gives a reset opportunity. (?)
    */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_cflag &= ~(CSIZE | PARENB);
    tty.c_cflag |= CS8;
    if(1) {
        tty.c_cc[VTIME] = 1; // [=] Deciseconds - timeout for noncannonical reads.
        tty.c_cc[VMIN] = 1; // [=] Bytes - Minimum number of bytes for noncannonical read.
    }

    //cfmakeraw(&tty);

    // Set In/Out Baud Rate to be 9600
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Copy TTY to serial structure
    memcpy(&serial->tty,&tty,sizeof(tty));

    //Apply Configuration to TTY, check for error.
    if(tcsetattr(serial->serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return -1;
    }

    fcntl(serial->serial_port,F_SETLK,F_RDLCK);
    fcntl(serial->serial_port,F_SETLK,F_WRLCK);
    return 0;

}

int serial_delink(serial_connection_t* serial) {
    int r=close(serial->serial_port);
    serial->serial_port=-1;
    memset(&serial->tty,0x00,sizeof(serial->tty));
    return r;
}

int serial_write(serial_connection_t* serial, uint8_t* payload, uint16_t size) {
    int wn = write(serial->serial_port, payload, size);
    return (wn == size) ? 1 : -1;
}

int serial_ewrite(serial_connection_t* serial, uint8_t* payload, uint16_t size) {
    uint16_t n = 0;
    int bd = 0;
    do {
        bd = write(serial->serial_port, payload, size);
        tcdrain(serial->serial_port);
        n += (bd>-1) ? bd : 0;
    } while (n<size);
    return (n==size) ? 1 : -1;
}


int serial_read(serial_connection_t* serial, uint8_t* payload, uint16_t size) {
    int rn = read(serial->serial_port, payload, size);
    return (rn == size) ? 1 : -1;
}

int serial_struct_tx(serial_connection_t* serial, transfer_t packet) {
    int n=sizeof(startp);
    int nt=0;
    memcpy(wbf,startp,n);
    nt+=n;
    n = sizeof(packet.id);
    memcpy((wbf+nt),&packet.id,n);
    nt+=n;
    n=sizeof(packet.size);
    memcpy((wbf+nt),&packet.size,n);
    nt+=n;
    n=packet.size;
    memcpy((wbf+nt),&packet.payload,n);
    nt+=n;
    n=sizeof(stopp);
    memcpy((wbf+nt),stopp,n);
    nt+=n;
    return serial_ewrite(serial,wbf,nt);

}

int serial_receive(serial_connection_t* serial) { // n will be removed once debug is unnecessary
    int bytes_read = read(serial->serial_port, &rbf[rbfi], rbf_size-rbfi);
    if(bytes_read>-1) {
        rbfi+=bytes_read;
        //printf("rbfi set to %d (+-%d)\n",rbfi,bytes_read);
        return 1;
    } else {
        //printf("rbfi unchanged: error\n");
        return -1;
    }

}


int parse_buffer(transfer_t* packet) {

    int i; // index
    int d; // size of field-index delta
    int sspc; // start-stop pattern compare
    transfer_t tmp={0};
  
    if(rbfi==0 || rbfi>=rbf_size) {
        //sleep(2);
        //printf("Exiting Parser: rbfi (%d) is out of bounds\n",rbfi);
        return -1; // Returns early if there is no data in the buffer or the read buffer index is beyond the size of the read buffer.
    }

    uint8_t* rbpp = (uint8_t*) memchr(rbf,*startp,rbfi); //Searches for the first character of the start pattern from the 0 to rbfi in the read buffer.
    if (rbpp==0) { // Checks null pointer (no pattern found)
      rbfi=0; // The pattern does not exist in the buffer, so the buffer has nothing of value.
      return -1;
    } else {
      rbfi-= rbpp-rbf; // Get the length of buffer contents after the pattern.
      memcpy(rbf,rbpp,rbfi); // Shift the contents to the start of the buffer.
      sspc=memcmp(rbf,startp,startp_size); // Compare to Start Patter
      if(sspc!=0) { // If there is no match:
        rbfi-=1; // Decrement the number of bytes in the buffer
        memcpy(rbf,(rbf+1),rbfi); // And remove the first byte - ie the beginning of the start pattern.
        return parse_buffer(packet); //  Reparse buffer for packets.
  
      } else if(rbfi >= (startp_size+psize_offset+sizeof(tmp.size)+stopp_size)) { // If the buffer contents are large enough to support a packet:
        // It may be better to define another variable here and do += based on the sizeof each field.
        i=startp_size;
        d=sizeof(tmp.id);
        memcpy(&tmp.id,rbf+i,d); // Populate the ID field
        i+=d;
        d=sizeof(tmp.size);
        memcpy(&tmp.size,rbf+i,d); // Populate the Size Field
        i+=d;
        if(rbfi<(i+tmp.size+stopp_size-1)) return -1; // Early return if buffer does not have enough data. Maybe add a check to see if rbfi has increased since last function call to avoid copying and comparing so much...
      } else {
        return -1; // Buffer contents are to small.
      }
      sspc=memcmp(rbf+i+tmp.size,stopp,stopp_size);
      if(sspc!=0) {
        rbfi-=1;
        memcpy(rbf,rbf+1,rbfi);
        return parse_buffer(packet);
      } else {
        packet->id=tmp.id;
        packet->size=tmp.size;
        d=tmp.size;
        memcpy(&packet->payload,rbf+i,d);
        i+=d;
        d=stopp_size;
        i+=d;
        rbfi-=i;
        memcpy(rbf,rbf+i,rbfi); // Remove packet from buffer
        return 1;
      }
  
  
    }
    return -1;



}