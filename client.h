#ifndef CLIENT_H_
#define CLIENT_H_
#include "common_TCPsocket.h"

typedef struct{
	sockTCP_t socket;
} client_t;

int main(int argc, char **argv);
static void _create(client_t *self,char* argv[]);
static int _seleccionarInput(int argc, char *argv[]);
static int _leerLinea(FILE* file,char *linea,int *largo_linea,
		             char *tmp, int tamtmp);
static void _leerArchivo(client_t *self,FILE* file);
static void _codificar_linea(client_t *self,char *linea,uint32_t id);
//destino,interfaz,metodo y path se codifican con esta funcion
static void _convertir_parametro(char *parametro,char **linea,
								uint8_t tipoparam,uint8_t cantstring,
								char tipodato,uint8_t barracero,int *tamparam);
static int _multiplo8(int longitud);
static int _contarargumentos(char *argumentos);
static void _convertir_firma(char **linea,uint8_t tipoparam,uint8_t cantstring
							,char tipodato, uint8_t barracero,int *tamfirma,
							int *tamfirmaconpad,uint8_t cantargumentos);
static int _tiene_firma(char *metodo,int tammetodo,char *argumentos,int *cant);
static void _convertir_body(char *metodo,char **body,int *tambody,
		                    uint8_t cantargumentos);
//envia los primeros 16 bytes de la descripcion de la llamada
static void _enviardescripcion(client_t *self,uint32_t tamarray,
								uint32_t tambody,char* endian,
								uint8_t tipo, uint32_t id);
//envia cada parametro (destino,interfaz,metodo,path,firma,body)
static void _enviarparametro(client_t *self,char *parametro
							  ,uint32_t tamparametro);
static void _closeclient(client_t *self);
static void _recibirok(client_t *self,uint32_t id);

#endif /* CLIENT_H_ */
