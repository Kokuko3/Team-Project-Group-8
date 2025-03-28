#include "local.h"
#include <assert.h>
#include <stdio.h> //printf, fopen, fclose, fwrite
#include <string.h> // Provides memcpy
#include <stdlib.h> // Provides malloc, calloc, realloc, free


int hash(FILE *fr, uint8_t md5[16]) {
    assert(fr);
    assert(md5);
    EVP_MD_CTX *md5_ctx = 0;
    unsigned int md5_len = 0;
    uint8_t md5_value[EVP_MAX_MD_SIZE];
    uint8_t buffer[32];
    int8_t n = 0;

    //FILE *fr = fopen(filepath,"rb+");
    //if(!fr) return -1;

    md5_ctx = EVP_MD_CTX_new();
    //const EVP_MD *md5_evp_md = EVP_md5();    
    rewind(fr);
    EVP_DigestInit(md5_ctx,EVP_md5());
    while(1) {
        //n = fread(buffer,sizeof(buffer),1,fr);
        n = fread(buffer,1,sizeof(buffer),fr);
        if(n!=sizeof(buffer)) {
            if(feof(fr)) {
                EVP_DigestUpdate(md5_ctx,buffer,n);
                break;
            }
            if(ferror(fr)) return -1;
        } else {
            EVP_DigestUpdate(md5_ctx,buffer,n);
        }
    }
    //fclose(fr);

    EVP_DigestFinal_ex(md5_ctx,md5_value,&md5_len);
    
    memcpy(md5,md5_value,md5_len);
    return 1;
    //*len=md5_len;
 
}

int hashfile(char *filepath, uint8_t md5[16]) {
    EVP_MD_CTX *md5_ctx = 0;
    unsigned int md5_len = 0;
    uint8_t md5_value[EVP_MAX_MD_SIZE];
    uint8_t buffer[32];
    int8_t n = 0;

    FILE *fr = fopen(filepath,"rb+");
    if(!fr) return -1;

    md5_ctx = EVP_MD_CTX_new();
    //const EVP_MD *md5_evp_md = EVP_md5();    

    EVP_DigestInit(md5_ctx,EVP_md5());
    while(1) {
        //n = fread(buffer,sizeof(buffer),1,fr);
        n = fread(buffer,1,sizeof(buffer),fr);
        if(n!=sizeof(buffer)) {
            if(feof(fr)) {
                EVP_DigestUpdate(md5_ctx,buffer,n);
                break;
            }
            if(ferror(fr)) return -1;
        } else {
            EVP_DigestUpdate(md5_ctx,buffer,n);
        }
    }
    fclose(fr);

    EVP_DigestFinal_ex(md5_ctx,md5_value,&md5_len);
    //uint8_t md5_size = sizeof(md5);
    //assert(md5_len==md5_size); // This doesn't work because the size decays to the length of a pointer.
    int result = strncmp((const char*) md5_value, (const char*) md5,md5_len);


    
    return result ? 1 : -1;
}

void file2data(FILE *fp, struct node **list) {
    assert(fp);
    assert(list);

    struct data_packet tmp = {0};
    uint32_t count = 0;
    
    rewind(fp);
    for(;;) {
        tmp.pn=++count;
        tmp.psize = fread(tmp.payload,1,psize_max,fp);
        if(tmp.psize == psize_max) {
            append(list,&tmp,sizeof(tmp));
        } else if(feof(fp)) {
            if(!tmp.psize) {
                break;
            }
            append(list,&tmp,sizeof(tmp));
        } else if(ferror(fp)) {
            assert(0);
        } else {
            assert(0);
        }

    }

}

void data2file(FILE *fp, struct node **list) {
    assert(fp);
    assert(list);

    struct node* tmp_node = {0};
    struct data_packet tmp_data = {0};


    size_t l = length(list);
    for(size_t i=1; i<=l; i++) {
        tmp_node = retrieve(list,i);
        node2data(tmp_node,&tmp_data);
        fwrite(tmp_data.payload,1,tmp_data.psize,fp);
    }

}

void file2list(FILE *fp, struct node **list) {
    assert(fp);
    assert(list);

    uint8_t frbf[psize_max] = {0};
    size_t fbr = 0;

    for(;;) {
        fbr = fread(frbf,1,sizeof(frbf),fp);
        if(sizeof(frbf) == fbr) {
            append(list,frbf,fbr);
        } else if(feof(fp)) {
            if(!fbr) {
                break;
            }
            append(list,frbf,fbr);
        } else if(ferror(fp)) {
            assert(0);
        } else {
            assert(0);
        }

    }

}

void list2file(FILE *fp, struct node **list) {
    assert(fp);
    assert(list);

    struct node *tmp = head(list);
    size_t l = length(list);

    if(!l) return;
    for(size_t i=1; i<=l; i++) {
        fwrite(tmp->data,1,tmp->size,fp);
        tmp=next(tmp);
    }

}


int send_start_packet(serial_connection_t *serial, struct start_packet start) {
    assert(serial);

    transfer_t tmp = {0};
    tmp.id=START;
    uint8_t i = 0;
    uint8_t d = sizeof(start.md5);
    memcpy(&tmp.payload[i],&start.md5[0],d);
    i += d;
    d = sizeof(start.pc);
    memcpy(&tmp.payload[i],&start.pc,d);
    i += d;
    tmp.size=i;

    return serial_struct_tx(serial, tmp);
}

int send_data_packet(serial_connection_t *serial, struct data_packet data) {
    assert(serial);

    transfer_t tmp = {0};
    tmp.id=DATA;
    uint16_t i = 0;
    uint8_t d = sizeof(data.pn); 
    memcpy(&tmp.payload[i],&data.pn,d);
    i += d;
    d = sizeof(data.psize);
    memcpy(&tmp.payload[i],&data.psize,d);
    i += d;
    d = data.psize;
    memcpy(&tmp.payload[i],&data.payload[0],d);
    i += d;
    tmp.size=i;

    return serial_struct_tx(serial, tmp);
}


int send_ack_packet(serial_connection_t *serial, struct ack_packet ack) {
    assert(serial);
    
    transfer_t tmp = {0};
    tmp.id=ACK;
    uint16_t i = 0;
    uint8_t d = sizeof(ack.type);
    memcpy(&tmp.payload[i],&ack.type,d);
    i += d;
    d = sizeof(ack.response);
    memcpy(&tmp.payload[i],&ack.response,d);
    i += d;
    d = sizeof(ack.psize);
    memcpy(&tmp.payload[i],&ack.psize,d);
    i += d;
    d = ack.psize;
    memcpy(&tmp.payload[i],&ack.payload[0],d);
    i += d;
    tmp.size=i;
    
    return serial_struct_tx(serial,tmp);
}

int send_rack_packet(serial_connection_t *serial, struct rack_packet rack) {
    assert(serial);

    transfer_t tmp = {0};
    tmp.id = RACK;
    uint8_t i = 0;
    uint8_t d = sizeof(rack.type);
    memcpy(&tmp.payload[i],&rack.type,d);
    i += d;
    d = sizeof(rack.request);
    memcpy(&tmp.payload[i],&rack.request,d);
    i += d;
    tmp.size = i;
    
    return serial_struct_tx(serial, tmp);
}

int packet_id(transfer_t trans) {
    switch(trans.id) {
        case START:
            return START;
        case DATA:
            return DATA;
        case RACK:
            return RACK;
        case ACK:
            return ACK;
        default:
            assert(0);
            break;
    }
    return 0;
}

// data2 node is a flawed premise... That would require dynamic allocation...
void node2data(struct node *record, struct data_packet *data) {
    assert(record);
    assert(data);
    
    assert(record->size == sizeof(struct data_packet));
    memcpy(data,record->data,record->size);
}

void transfer2data(transfer_t *trans,struct data_packet *data) {
    assert(trans);
    assert(data);

    struct data_packet tmp = {0};

    assert(trans->size >= (sizeof(tmp.pn) + sizeof(tmp.psize)));
    size_t i = 0;
    size_t d = sizeof(tmp.pn);
    memcpy(&tmp.pn,&trans->payload[i],d);
    i += d;
    d = sizeof(tmp.psize);
    memcpy(&tmp.psize,&trans->payload[i],d);
    i += d;
    d = tmp.psize;
    memcpy(tmp.payload,&trans->payload[i],d);
    i += d;
    assert(i==trans->size);
    *data=tmp;

}

void transfer2start(transfer_t *trans, struct start_packet *start) {
    assert(trans);
    assert(start);

    struct start_packet tmp = {0};
    
    assert(trans->id == START);
    assert(trans->size == sizeof(start->md5) + sizeof(start->pc));

    uint8_t i = 0;
    uint8_t d = sizeof(tmp.md5);
    memcpy(&tmp.md5[0],&trans->payload[i],d);
    i += d;
    d = sizeof(tmp.pc);
    memcpy(&tmp.pc,&trans->payload[i],d);
    i += d;
    assert(i == trans->size);
    *start=tmp;
}

void transfer2rack(transfer_t *trans, struct rack_packet *rack) {
    assert(trans);
    assert(rack);

    struct rack_packet tmp = {0};
    assert(trans->id == RACK);
    assert(trans->size == sizeof(tmp.type) + sizeof(tmp.request));

    uint8_t i = 0;
    uint8_t d = sizeof(tmp.type);
    memcpy(&rack->type,&trans->payload[i],d);
    i+=d;
    d = sizeof(tmp.request);
    memcpy(&rack->request,&trans->payload[i],d);
    i += d;
    assert(i==trans->size);
    *rack=tmp;
}

void transfer2ack(transfer_t *trans, struct ack_packet *ack) {
    assert(trans);
    assert(ack);

    struct ack_packet tmp = {0};
    assert(trans->id == ACK);
    assert(trans->size >= sizeof(tmp.type) + sizeof(tmp.response) + sizeof(tmp.psize));

    uint8_t i = 0;
    uint8_t d = sizeof(tmp.type);
    memcpy(&tmp.type,&trans->payload[i],d);
    i += d;
    d = sizeof(tmp.response);
    memcpy(&tmp.response,&trans->payload[i],d);
    i += d;
    d = sizeof(tmp.psize);
    memcpy(&tmp.psize,&trans->payload[i],d);
    i += d;
    d = tmp.psize;
    memcpy(&tmp.payload[0],&trans->payload[i],d);
    i += d;
    assert(i == trans->size);
    *ack=tmp;
}


int compare_start(struct start_packet s1, struct start_packet s2) {
    if(s1.pc!=s2.pc) return 0;
    if(strncmp((const char *) s1.md5, (const char *) s2.md5,sizeof(s1.md5))!=0) return 0;
    return 1;
}