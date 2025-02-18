#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main() {

   char fn_in[] = "input.jpg";
   char fn_out[] = "output.jpg";

   FILE* fiptr = fopen(fn_in,"rb");
   FILE* foptr = fopen(fn_out,"wb");

   if(fiptr==NULL) return -1;
   if(foptr==NULL) return -1;

   uint16_t robjects = 0;
   uint8_t rdata[256];

   uint16_t wobjects = 0;
   uint8_t wdata[256];

   while(!feof(fiptr) && !ferror(fiptr)) {
      memset(&rdata[0],0x00,256);
      memset(&wdata[0],0x00,256);
      robjects=fread(&rdata[0],1,256,fiptr);

      printf("Objects Read=%d : %s\n\n", robjects, rdata);
      memcpy(&wdata,&rdata,robjects);

      wobjects=fwrite(&wdata[0],1,robjects,foptr);
      printf("Objects Written=%d : %s\n\n",wobjects,&wdata);
   }


   fclose(fiptr);
   fclose(foptr);

}
