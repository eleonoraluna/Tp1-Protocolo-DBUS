#ifndef SERVER_H_
#define SERVER_H_
#include "common_TCPsocket.h"

typedef struct{
	sockTCP_t socket;
} server_t;

int main(int argc, char *argv[]);

//crea el socket y lo conecta
static void _create(server_t *self,char* argv[]);

//recibe los primeros 16 bytes de cada linea
 //hasta que cierren el socket
static void _recibirMensajes(server_t *self);

//decodifica los primeros 16 bytes
static void _decodificar(server_t *self,char* buffer);

//hace un rcv del tipo de parametro y decodifica cada uno
// dependiendo del tipo
static void _decodificar_array(server_t *self,uint32_t longarray,
									uint32_t longbody,uint32_t id);

//el destino,path,interfaz y metodo se decodifican con esta funcion
static void _decodificar_parametro(server_t *self,int* pos,char* buffer);

//devuelve la cantidad de parametros que tiene la firma
static uint8_t _cant_parametros(server_t *self,int* pos);

//auxiliar para calcular el padding
static int _multiplo8(int longitud);

//output pedido
static void _imprimir(server_t *self,uint32_t id,char *destino,char *path,
		char *interfaz,char *metodo,uint32_t longbody, int cantparametros);

//hace rcv de los parametros de la firma
static void _leer_parametro(server_t *self,char *parametro);
static void _close(server_t *self);
static void _enviarok(server_t *self);


#endif /* SERVER_H_ */
