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

uint8_t rbuff[512]={0};
static uint16_t ri_end=0;
static uint16_t ri=0;
static uint8_t wbuff[512]={0};
static uint8_t* wp;
static uint8_t start_pattern[4]={0x01,0x02,0x03,0x04}; 
static uint8_t stop_pattern[4]={0x04,0x03,0x02,0x01}; 

serial_connection_t serial_create_structure(char* device) {
    serial_connection_t serial = {0};
    serial.device=device;
    return serial;
}

int serial_link(serial_connection_t* serial) {
    // Get File Descriptor of Serial Port
    serial->serial_port=open(serial->device, O_RDWR & ~O_NONBLOCK); //open ttyACM0 read/write

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

    /* Configure TTY */  
    // Control Modes - c_cflags
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

    // Set In/Out Baud Rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Copy TTY to serial structure
    memcpy(&serial->tty,&tty,sizeof(tty));

    //Apply Configuration to TTY, check for error.
    if(tcsetattr(serial->serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return -1;
    }

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
        n += (bd>-1) ? bd : 0;
    } while (n<size);
    return (n==size) ? 1 : -1;
}


int serial_read(serial_connection_t* serial, uint8_t* payload, uint16_t size) {
    int rn = read(serial->serial_port, payload, size);
    return (rn == size) ? 1 : -1;
}

int serial_struct_tx(serial_connection_t* serial, transfer_t structure) {
    wp = &wbuff[0];
    int nt = 0;

    int n = sizeof(start_pattern);
    memcpy(wp,&start_pattern[0],n);
    wp += n;
    nt += n;

    n=sizeof(structure.id);
    memcpy(wp,&structure.id,n);
    wp += n;
    nt += n;

    n=sizeof(structure.size);
    memcpy(wp,&structure.size,n);
    wp += n;
    nt += n;

    n = structure.size;
    memcpy(wp,&structure.payload[0],n);
    wp += n;
    nt += n;

    n = sizeof(stop_pattern);
    memcpy(wp, &stop_pattern[0],n);
    nt += n;

    return serial_ewrite(serial,&wbuff[0],nt);
}

int serial_receive(serial_connection_t* serial,uint8_t* buff,uint16_t n) { // n will be removed once debug is unnecessary
    int bytes2read=0;
    int bytesread=0;
    if (ioctl(serial->serial_port, FIONREAD, &bytes2read)<0) {
        return -1;
    } else {
        if( (bytes2read+ri_end) <= sizeof(rbuff)) {
            bytesread = read(serial->serial_port, &rbuff[ri_end], bytes2read);
            ri_end += (bytesread > -1) ? bytesread : 0;
            printf("The value of ri_end %d\n",ri_end);

        } else if( (bytes2read+ri_end) > sizeof(rbuff)) {
            bytesread = read(serial->serial_port, &rbuff[ri_end], sizeof(rbuff)-ri_end);
            ri_end += (bytesread > -1) ? bytesread : 0;

        }
    }
    memcpy(buff,rbuff,n);
    return 1;

}


int parse_buffer(transfer_t* structure) {
    transfer_t tmp={0};
    if( (ri_end==0) || (ri_end>sizeof(rbuff))) return -1;
    
    uint8_t* rp = memchr(rbuff,start_pattern[0],ri_end);
    if(rp==0) {
        ri_end=0;
        return -1;
    } else {
        ri_end-=rp-rbuff;
        memcpy(rbuff,rp,ri_end);
        int m=memcmp(rbuff,start_pattern,sizeof(start_pattern));
        if(m!=0) {
            ri_end-=1; //sizeof(start_pattern);
            memcpy(rbuff,rbuff+1,ri_end); //sizeof(start_pattern)
            return parse_buffer(structure);
        } else if(ri_end >= (sizeof(start_pattern)+sizeof(stop_pattern)+sizeof(tmp.id)+sizeof(tmp.size))) {
            memcpy(&tmp.id,rbuff+sizeof(start_pattern),sizeof(tmp.id));
            memcpy(&tmp.size,rbuff+sizeof(start_pattern)+sizeof(tmp.size),sizeof(tmp.size));
            if(ri_end<(sizeof(start_pattern)+sizeof(stop_pattern)+sizeof(tmp.id)+sizeof(tmp.size)+tmp.size)) return -1;
        } else {
            return -1;
        }
        m=memcmp(rbuff+sizeof(start_pattern)+sizeof(tmp.id)+sizeof(tmp.size)+tmp.size,&stop_pattern,sizeof(stop_pattern));
        if(m!=0) {
            ri_end-=sizeof(start_pattern);
            memcpy(rbuff,rbuff+sizeof(start_pattern),ri_end);
            return parse_buffer(structure);
        } else {
            structure->id=tmp.id;
            structure->size=tmp.size;
            memcpy(&structure->payload,rbuff+sizeof(start_pattern)+sizeof(tmp.id)+sizeof(tmp.size),tmp.size);
            int total_size=sizeof(tmp.id) +sizeof(tmp.size)+tmp.size+sizeof(start_pattern)+sizeof(stop_pattern);
            ri_end-=total_size;
            memcpy(rbuff,rbuff+total_size,ri_end);
            return 1;
        }
    }
    return -1;
}





