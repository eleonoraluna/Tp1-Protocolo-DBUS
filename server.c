#include <stdio.h>
#include "server_decoder.h"

int main(int argc, char *argv[]) {
   if (argc == 2) {
	   decoder_t decoder;
	   if (server_decoder_create(&decoder,argv)==0){
	   server_decoder_run(&decoder);
	   server_decoder_destroy(&decoder);
	   }
   } else {
	   printf("modo no soportado\n");
   }
   return 0;
}
