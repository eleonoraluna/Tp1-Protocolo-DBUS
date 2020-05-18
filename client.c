#include <stdio.h>
#include "client_encoder.h"

int main(int argc, char *argv[]) {
   if (argc >= 3) {
	   encoder_t encoder;
	   client_encoder_create(&encoder,argv);
	   client_encoder_selectInput(argc,argv,&encoder);
   } else {
	   printf("modo no soportado");
   }
   return 0;
}


