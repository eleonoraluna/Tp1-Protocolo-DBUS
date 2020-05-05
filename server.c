#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include "server.h"

const int MAX_DES=24;
const int MAX_PATH=30;
const int MAX_INT=32;
const int MAX_MET=126;
const int MAX_PARAM=72;

int main(int argc, char *argv[]) {
   if (argc == 2) {
	   server_t server;
	   _create(&server,argv);
	   _recibirMensajes(&server);
   } else {
	   printf("modo no soportado\n");
   }
   return 0;
}

static void _create(server_t *self,char* argv[]){
	TCPsocket_create(&self->socket);
	TCPsocket_bind(&self->socket,argv[1]);
	TCPsocket_listen(&self->socket);
	TCPsocket_accept(&self->socket);
}

static void _recibirMensajes(server_t *self){
	char buffer[16];
	while((TCPsocket_recieve(&self->socket,buffer,sizeof(buffer))) !=-1){
		_decodificar(self,buffer);
		_enviarok(self);
	}
	_close(self);
}

static void _decodificar(server_t *self,char* buffer){
	uint32_t longbody,id,longarray;
	buffer=buffer+4; //salteo los primeros 4 que no me sirven
	memcpy(&longbody,buffer,4);
	buffer=buffer+4; //salto al id
	memcpy(&id,buffer,4);
	buffer=buffer+4; //salto a la long del array
	memcpy(&longarray,buffer,4);
	_decodificar_array(self,longarray,longbody,id);
}

static void _decodificar_array(server_t *self,uint32_t longarray,
							   uint32_t longbody,uint32_t id){
	char *interfaz=malloc(MAX_INT);
	char *destino=malloc(MAX_DES);
	char *path=malloc(MAX_PATH);
	char *metodo=malloc(MAX_MET);
	char tipo[1];
	int pos=0,cantidad=0;
	while (pos<longarray){
		//hago un recieve de 1 byte para ver el tipo
		TCPsocket_recieve(&self->socket,tipo,1);
		switch(tipo[0]){
		case 1: pos=pos+1;
				_decodificar_parametro(self,&pos,path);
				break;
		case 2: pos=pos+1;
			    _decodificar_parametro(self,&pos,interfaz);
				break;
		case 6: pos=pos+1;
			    _decodificar_parametro(self,&pos,destino);
			    break;
		case 3: pos=pos+1;
			    _decodificar_parametro(self,&pos,metodo);
				break;
		case 8: pos=pos+1;
				cantidad=_cant_parametros(self,&pos);
				break;
		}
	}
	_imprimir(self,id,destino,path,interfaz,metodo,longbody,cantidad);
	free(destino);
	free(path);
	free(interfaz);
	free(metodo);
}

static void _decodificar_parametro(server_t *self,int* pos, char *buffer){
	char tmp[3];
	uint32_t longparam;
	//hago un rcv de 3 que no me sirve para nada
	TCPsocket_recieve(&self->socket,tmp,sizeof(tmp));
	//hago un rcv de 4 que me da el tamaño
	TCPsocket_recieve(&self->socket,(char*)&longparam,sizeof(longparam));
	*pos=*pos+7; //ya recibi 7 bytes de mi array
	longparam=longparam+1; //el /0
	longparam=_multiplo8(longparam); //me devuelve el tamaño incluyendo padding
	TCPsocket_recieve(&self->socket,buffer,longparam);
	*pos=*pos+longparam;//recibi longparam cant de bytes
}

static uint8_t _cant_parametros(server_t *self,int* pos){
	uint8_t cantparam,offset,tam_firma,padding;
	char tmp[3],tmp2[MAX_PARAM];
	//hago un rcv de 3 que no me sirve para nada
	TCPsocket_recieve(&self->socket,tmp,sizeof(tmp));
	//hago un rcv de 1 para la cant de param
	TCPsocket_recieve(&self->socket,(char*)&cantparam,sizeof(cantparam));
	*pos=*pos+4;//recibi 4 bytes
	tam_firma=cantparam+1+1+4;//mas el barra 0 + los 4 primeros
	//me devuelve el tamaño de la firma con padding, si es que lo tiene
	offset=_multiplo8(tam_firma);
	padding=offset-tam_firma;
	//hago un rcv de cantparam+1+padding
	TCPsocket_recieve(&self->socket,tmp2,cantparam+1+padding);
	*pos=*pos+cantparam+1+padding;
	return cantparam;
}

static int _multiplo8(int longitud){
	int l=longitud;
	while((l%8) != 0){
		l=l+1;
	}
	return l;
}

static void _imprimir(server_t *self,uint32_t id,char *destino,char *path,
					  char *interfaz,char *metodo,uint32_t longbody,int cant){
	printf("* Id: 0x%08x\n",id);
	printf("* Destino: %s\n",destino);
	printf("* Ruta: %s\n",path);
	printf("* Interfaz: %s\n",interfaz);
	printf("* Metodo: %s\n",metodo);
	if (longbody!=0){
		printf("* Parametros:\n");
		for (int i=0; i<cant; i++){
		char *parametro=malloc(MAX_PARAM);
		_leer_parametro(self,parametro);
		printf("    * %s\n",parametro);
		free(parametro);
		}
	}
	printf("\n");
}

static void _leer_parametro(server_t *self,char *parametro){
	uint32_t tamparam;
	//hago un rcv de 4 para el tamaño del param
	TCPsocket_recieve(&self->socket,(char*)&tamparam,sizeof(tamparam));
	//hago un rcv del parametro +0
	TCPsocket_recieve(&self->socket,parametro,tamparam+1);
}

static void _enviarok(server_t *self){
	char buffer[3]="OK\n";
	TCPsocket_send(&self->socket,buffer,sizeof(buffer));
}

static void _close(server_t *self){
	TCPsocket_destroy(&self->socket);
}
