#ifndef CLIENT_ENCODER_H_
#define CLIENT_ENCODER_H_
#include "common_TCPsocket.h"

typedef struct{
	sockTCP_t socket;
} encoder_t;

//crea el socket y lo conecta
//si tuvo algun problema devuelve 1
int client_encoder_create(encoder_t *self,char* argv[]);

//si se le pasa un archivo lo abre y sino toma por input stdin
void client_encoder_run(encoder_t *self,int argc, char *argv[]);

//cierra el socket
void client_encoder_destroy(encoder_t *self);


#endif /* CLIENT_ENCODER_H_ */
