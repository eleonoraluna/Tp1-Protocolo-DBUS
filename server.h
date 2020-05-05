#ifndef SERVER_H_
#define SERVER_H_
#include "common_TCPsocket.h"

typedef struct{
	sockTCP_t socket;
} server_t;

int main(int argc, char *argv[]);
static void _recibirMensajes(server_t *self);
static void _decodificar(server_t *self,char* buffer);
static void _create(server_t *self,char* argv[]);
static void _decodificar_array(server_t *self,uint32_t longarray,
									uint32_t longbody,uint32_t id);
static void _decodificar_parametro(server_t *self,int* pos,char* buffer);
static uint8_t _cant_parametros(server_t *self,int* pos);
static int _multiplo8(int longitud);
static void _imprimir(server_t *self,uint32_t id,char *destino,char *path,
		char *interfaz,char *metodo,uint32_t longbody, int cantparametros);
static void _leer_parametro(server_t *self,char *parametro);
static void _close(server_t *self);
static void _enviarok(server_t *self);


#endif /* SERVER_H_ */
