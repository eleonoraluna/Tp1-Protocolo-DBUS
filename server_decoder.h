#ifndef SERVER_DECODER_H_
#define SERVER_DECODER_H_
#include "common_TCPsocket.h"

typedef struct{
	sockTCP_t socket;
} decoder_t;

//crea el socket y lo conecta
//si tiene algun problema devuelve 1
int server_decoder_create(decoder_t *self,char* argv[]);

//recibe datos mediante el socket,
//los decodifica y luego
//imprime por pantalla los datos
void server_decoder_run(decoder_t *self);

//cierra el socket
void server_decoder_destroy(decoder_t *self);


#endif /* SERVER_DECODER_H_ */
