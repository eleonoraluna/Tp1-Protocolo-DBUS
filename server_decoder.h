#ifndef SERVER_DECODER_H_
#define SERVER_DECODER_H_
#include "common_TCPsocket.h"

typedef struct{
	sockTCP_t socket;
} decoder_t;

//recibe los primeros 16 bytes de cada linea
 //hasta que cierren el socket
void server_decoder_rcvMessages(decoder_t *self);

//crea el socket y lo conecta
void server_decoder_create(decoder_t *self,char* argv[]);


#endif /* SERVER_DECODER_H_ */
