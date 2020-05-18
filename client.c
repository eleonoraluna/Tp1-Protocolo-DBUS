#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "client_encoder.h"

int main(int argc, char *argv[]) {
   if (argc >= 3) {
	   encoder_t encoder;
	   if(client_encoder_create(&encoder,argv)==0){
	   client_encoder_run(&encoder,argc,argv);
	   client_encoder_destroy(&encoder);
	   }
   } else {
	   printf("modo no soportado\n");
   }
   return 0;
}
