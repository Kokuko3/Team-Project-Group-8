#include <stddef.h> // offsetof macro
#include <stdint.h> // uints
#include <stdio.h> // printf
#include <string.h> // Provides memcpy, memset,etc.

#define rbf_size 256 // read buffer size
#define wbf_size 256 // write buffer size
uint8_t rbf[rbf_size]; // read buffer
uint8_t wbf[wbf_size]; // write buffer
uint8_t rbfi=0; // read buffer index.
uint8_t wbfi=0; // write buffer index.

#define startp_size 4 // start pattern size
#define stopp_size 4 // stop pattern size
uint8_t startp[startp_size]={0x12,0x09,0x16,0x83}; //start pattern
uint8_t stopp[stopp_size]={0x04,0x18,0x20,0x02}; // stop pattern

#define psize_max 256
struct transf { // packet structure
  uint8_t id;
  uint8_t size;
  uint8_t payload[psize_max];
};
struct transf rx_packet; // receive packet structure
struct transf tx_packet; // transmit packet structure
uint16_t psize_offset=1; // the index offset of the size field when unpadded. 
//uint16_t psize=1;
//offsetof(struct transf,size);

void setup() {
  // Zero buffers
  memset(rbf,0x00,rbf_size);
  memset(wbf,0x00,wbf_size);

  // Configure and Open Serial
  Serial.setTimeout(1000); // [=] ms
  Serial.begin(9600);
  while(!Serial) {};

  /*
  char halp[32];
  char help[]="The offset is %d\n";
  sprintf(halp,help,psize_offset);
  Serial.print(halp);
  */
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(tx_packet.size);
  packetWrite(tx_packet);
  tx_packet.size+=1;
  tx_packet.size=tx_packet.size%256;
  delay(2000);
  Serial.println("");
}

int serialReadBuffer() {
  int bytes_read = Serial.readBytes(&rbf[rbfi],rbf_size-rbfi);
  if(bytes_read>-1) {
    rbfi+=bytes_read;
    return 1;
  } else {
    return -1;

  }

}

int serialParser(struct transf* packet) {
  int i; // index
  int d; // size of field-index delta
  int sspc; // start-stop pattern compare
  struct transf tmp={0};

  if(rbfi==0 || rbfi>=rbf_size) return -1; // Returns early if there is no data in the buffer or the read buffer index is beyond the size of the read buffer.

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
      return serialParser(packet); //  Reparse buffer for packets.

    } else if(rbfi >= (startp_size+psize_offset+sizeof(tmp.size)+stopp_size)) { // If the buffer contents are large enough to support a packet:
      // It may be better to define another variable here and do += based on the sizeof each field.
      i=startp_size;
      d=sizeof(tmp.id);
      memcpy(&tmp.id,rbf+i,d); // Populate the ID field
      i+=d;
      d=sizeof(tmp.size);
      memcpy(&tmp.size,rbf+i,d); // Populate the Size Field
      i+=d;
      if(rbfi<(rbfi+i+tmp.size+stopp_size-1)) return -1; // Early return if buffer does not have enough data. Maybe add a check to see if rbfi has increased since last function call to avoid copying and comparing so much...
    } else {
      return -1; // Buffer contents are to small.
    }
    sspc=memcmp(rbf+i+tmp.size,stopp,stopp_size);
    if(sspc!=0) {
      rbfi-=1;
      memcpy(rbf,rbf+1,rbfi);
      return serialParser(packet);
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

int packetWrite(struct transf packet) {
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
  
  //n=0;
  //do {
  //  n+=Serial.write(wbf+n,nt-n);
  //} while(n<nt);
  //return n;
  return Serial.write(wbf,nt);
}
