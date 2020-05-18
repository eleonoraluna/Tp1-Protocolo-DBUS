#ifndef CLIENT_ENCODER_H_
#define CLIENT_ENCODER_H_
#include <stdint.h>
#include "common_TCPsocket.h"

typedef struct{
	sockTCP_t socket;
} encoder_t;

//crea el socket y lo conecta
void client_encoder_create(encoder_t *self,char* argv[]);

//si se le pasa un archivo lo abre y sino toma por input stdin
int client_encoder_selectInput(int argc, char *argv[],encoder_t *self);


#endif /* CLIENT_ENCODER_H_ */
