#include <stdio.h>
#include "server_decoder.h"

int main(int argc, char *argv[]) {
   if (argc == 2) {
	   decoder_t decoder;
	   server_decoder_create(&decoder,argv);
	   server_decoder_rcvMessages(&decoder);
   } else {
	   printf("modo no soportado\n");
   }
   return 0;
}
