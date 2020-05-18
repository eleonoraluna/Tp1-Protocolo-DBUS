#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include "server_decoder.h"

const int MAX_DES=24;
const int MAX_PATH=30;
const int MAX_INT=32;
const int MAX_MET=126;
const int MAX_PARAM=72;
const int SIZE_DESCRIPTION_HEADER=16;
//cant de bytes de la descripcion del parametro sin el byte del tipo
const int SIZE_DESCRIPTION_PARAMETER=3;
const int SIZE_TYPE=1;

static int _multiple8(int length){
	int l=length;
	while((l%8) != 0){
		l=l+1;
	}
	return l;
}

static void _read_parameter(decoder_t *self,char *parameter){
	uint32_t lengthparam;
	//hago un rcv de 4 para el tamaño del param
	TCPsocket_recieve(&self->socket,(char*)&lengthparam,sizeof(lengthparam));
	//hago un rcv del parametro +0
	TCPsocket_recieve(&self->socket,parameter,lengthparam+1);
}

static void _print(decoder_t *self,uint32_t id,char *dest,char *path,
					  char *interface,char *method,uint32_t lengthbody,int count){
	char parameter[MAX_PARAM];
	printf("* Id: 0x%08x\n",id);
	printf("* Destino: %s\n",dest);
	printf("* Ruta: %s\n",path);
	printf("* Interfaz: %s\n",interface);
	printf("* Metodo: %s\n",method);
	if (lengthbody!=0){
		printf("* Parametros:\n");
		for (int i=0; i<count; i++){
		_read_parameter(self,parameter);
		printf("    * %s\n",parameter);
		}
	}
	printf("\n");
}

static void _decode_parameter(decoder_t *self,int* pos, char *buffer){
	char tmp[SIZE_DESCRIPTION_PARAMETER];
	uint32_t lengthparam;
	//hago un rcv de 3 que no me sirve para nada
	TCPsocket_recieve(&self->socket,tmp,sizeof(tmp));
	//hago un rcv de 4 que me da el tamaño
	TCPsocket_recieve(&self->socket,(char*)&lengthparam,sizeof(lengthparam));
	*pos=*pos+7; //ya recibi 7 bytes de mi array
	lengthparam=lengthparam+1; //el /0
	lengthparam=_multiple8(lengthparam); //me devuelve el tamaño incluyendo padding
	TCPsocket_recieve(&self->socket,buffer,lengthparam);
	*pos=*pos+lengthparam;//recibi lengthparam cant de bytes
}

static uint8_t _count_parameters(decoder_t *self,int* pos){
	uint8_t count_param,offset,lengthfirm,padding;
	char tmp[SIZE_DESCRIPTION_PARAMETER],tmp2[MAX_PARAM];
	//hago un rcv de 3 que no me sirve para nada
	TCPsocket_recieve(&self->socket,tmp,sizeof(tmp));
	//hago un rcv de 1 para la cant de param
	TCPsocket_recieve(&self->socket,(char*)&count_param,sizeof(count_param));
	*pos=*pos+4;//recibi 4 bytes
	lengthfirm=count_param+1+1+4;//mas el barra 0 + los 4 primeros
	//me devuelve el tamaño de la firma con padding, si es que lo tiene
	offset=_multiple8(lengthfirm);
	padding=offset-lengthfirm;
	//hago un rcv de cantparam+1+padding
	TCPsocket_recieve(&self->socket,tmp2,count_param+1+padding);
	*pos=*pos+count_param+1+padding;
	return count_param;
}

static void _decode_array(decoder_t *self,uint32_t lengtharray,
							   uint32_t lengthbody,uint32_t id){
	char interface[MAX_INT];
	char dest[MAX_DES];
	char path[MAX_PATH];
	char method[MAX_MET];
	char type[SIZE_TYPE];
	int pos=0,c=0;
	while (pos<lengtharray){
		//hago un recieve de 1 byte para ver el tipo
		TCPsocket_recieve(&self->socket,type,1);
		switch(type[0]){
		case 1: pos=pos+1;
				_decode_parameter(self,&pos,path);
				break;
		case 2: pos=pos+1;
			    _decode_parameter(self,&pos,interface);
				break;
		case 6: pos=pos+1;
			    _decode_parameter(self,&pos,dest);
			    break;
		case 3: pos=pos+1;
			    _decode_parameter(self,&pos,method);
				break;
		case 8: pos=pos+1;
				c=_count_parameters(self,&pos);
				break;
		}
	}
	_print(self,id,dest,path,interface,method,lengthbody,c);
}

static void _decode(decoder_t *self,char* buffer){
	uint32_t lengthbody,id,lengtharray;
	buffer=buffer+4; //salteo los primeros 4 que no me sirven
	memcpy(&lengthbody,buffer,4);
	buffer=buffer+4; //salto al id
	memcpy(&id,buffer,4);
	buffer=buffer+4; //salto a la long del array
	memcpy(&lengtharray,buffer,4);
	_decode_array(self,lengtharray,lengthbody,id);
}

static void _sendok(decoder_t *self){
	char buffer[3]="OK\n";
	TCPsocket_send(&self->socket,buffer,sizeof(buffer));
}

static void _close(decoder_t *self){
	TCPsocket_destroy(&self->socket);
}

void server_decoder_rcvMessages(decoder_t *self){
	char buffer[SIZE_DESCRIPTION_HEADER];
	while((TCPsocket_recieve(&self->socket,buffer,sizeof(buffer))) !=-1){
		_decode(self,buffer);
		_sendok(self);
	}
	_close(self);
}

void server_decoder_create(decoder_t *self,char* argv[]){
	TCPsocket_create(&self->socket);
	TCPsocket_bind(&self->socket,argv[1]);
	TCPsocket_listen(&self->socket);
	TCPsocket_accept(&self->socket);
}
