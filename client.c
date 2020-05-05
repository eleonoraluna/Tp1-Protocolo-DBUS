#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include "client.h"

const int BUFF_LECTURA=32;
const int MAX_LINEA=176;
const int MAX_DES=19;
const int MAX_PATH=17;
const int MAX_INT=25;
const int MAX_MET=128;
const int MAX_ARG=64;
const int TIPO_PARAM=4;
const int TAM_DESCRIPCION=16;

int main(int argc, char *argv[]) {
   if (argc >= 3) {
	   _seleccionarInput(argc,argv);
   } else {
	   printf("modo no soportado");
   }
   return 0;
}

static int _seleccionarInput(int argc, char *argv[]){
	FILE *input;
	if (argc==3){
		input = stdin;
		client_t client;
		_create(&client,argv);
		_leerArchivo(&client,input);
	} else {
		input=fopen(argv[3],"rt");
		if (!input) {
			printf("No se pudo abrir el archivo\n");
			return 1;
		} else {
			client_t client;
			_create(&client,argv);
			_leerArchivo(&client,input);
			fclose(input);
		}
	}
	return 0;
}

static void _create(client_t *self,char* argv[]){
	TCPsocket_create(&self->socket);
	TCPsocket_connect(&self->socket,argv[1],argv[2]);
}

static void _leerArchivo(client_t *self,FILE* file){
	int fin=1,largo_linea=0;
	uint32_t id=1;
	char tmp[32];
	memset(tmp,0,sizeof(tmp));
	while(fin!=0){
		char *linea=(char*)calloc(MAX_LINEA,sizeof(char));
		fin=_leerLinea(file,linea,&largo_linea,tmp,strlen(tmp));
		 _codificar_linea(self,linea,id);
		 _recibirok(self,id);
		 largo_linea=0;
		free(linea);
		id=id+1;
	}
	_closeclient(self);
}

static void _recibirok(client_t *self,uint32_t id){
	char msje[3];
	TCPsocket_recieve(&self->socket,msje,sizeof(msje));
	printf("0x%08x: %s",id,msje);
}

static int _leerLinea(FILE* file,char *linea,int *largo,char *tmp, int tamtmp){
	 char buffer[BUFF_LECTURA+1];
	 uint32_t dif,diftmp;
	 int linea_completa=0,offset=0,elementos_leidos=0;
	 memcpy(linea,tmp,tamtmp);
	 memset(tmp,0,tamtmp);
	 while (linea_completa==0){
		 memset(buffer,0,sizeof(buffer));
		 elementos_leidos=fread(buffer,BUFF_LECTURA, 1, file);
		 if (strchr(buffer,'\n') != NULL) {
			 char* ptr=strchr(buffer,'\n');
			dif=&*ptr-buffer;
			diftmp=sizeof(buffer)-dif;
			memcpy(tmp,ptr+1,diftmp);//lo que quedo despues del\n
			//lo que quedo hasta el \n
			memcpy(linea+tamtmp+(offset*BUFF_LECTURA),buffer,dif);
			*largo=*largo+tamtmp+dif;
			linea_completa=1;
		 }else{
			 memcpy(linea+tamtmp+(offset*BUFF_LECTURA),buffer,sizeof(buffer));
			*largo=*largo+tamtmp+BUFF_LECTURA;
			offset=offset+1;
		 }
	 }
	 return elementos_leidos;
}

static void _codificar_linea(client_t *self,char *linea,uint32_t id){
	char destino[MAX_DES],path[MAX_PATH],interfaz[MAX_INT],metodo[MAX_MET],
			nombremetodo[MAX_MET],argumentos[MAX_ARG];
	int tamarraysinpad=0,tamdest,tampath,tamint,tammet,tamfirma,
		tamfirmapad,tambody,cantargs;
	char* d; char* p; char* i; char* m; char* f; char* b;
	sscanf(linea,"%s %s %s %s",destino,path,interfaz,metodo);
	_convertir_parametro(destino,&d,6,1,'s',0,&tamdest);
	_convertir_parametro(path,&p,1,1,'o',0,&tampath);
	_convertir_parametro(interfaz,&i,2,1,'s',0,&tamint);
	sscanf(metodo," %[^(] ",nombremetodo);
	_convertir_parametro(nombremetodo,&m,3,1,'s',0,&tammet);
	if(_tiene_firma(metodo,strlen(nombremetodo),argumentos,&cantargs)==0){
		_convertir_firma(&f,8,1,'g',0,&tamfirma,&tamfirmapad,cantargs);
		_convertir_body(argumentos,&b,&tambody,cantargs);
	}
	tamarraysinpad=tamdest+tampath+tamint+tammet+tamfirma;
	_enviardescripcion(self,tamarraysinpad,tambody,"l",1,id);
	_enviarparametro(self,d,tamdest);
	_enviarparametro(self,p,tampath);
	_enviarparametro(self,i,tamint);
	_enviarparametro(self,m,tammet);
	if(_tiene_firma(metodo,strlen(nombremetodo),argumentos,&cantargs)==0){
		_enviarparametro(self,f,tamfirmapad);
		_enviarparametro(self,b,tambody);
	}
}

static void _convertir_parametro(char *parametro,char **linea,uint8_t tipoparam
		,uint8_t cantstring,char tipodato, uint8_t barracero,int *tamparam){
	uint32_t longconpadding,padding;
	uint32_t longitud=strlen(parametro);
	longconpadding=_multiplo8(longitud+1);
	padding=longconpadding-longitud;
	*linea=(char*)malloc(4+4+longitud+1+padding);
	char *offset=*linea;
	memcpy(offset,&tipoparam,sizeof(tipoparam));
	memcpy(offset+1,&cantstring,sizeof(cantstring));
	memcpy(offset+2,&tipodato,sizeof(tipodato));
	memcpy(offset+3,&barracero,1);
	memcpy(offset+4,&longitud,sizeof(longitud));
	memcpy(offset+8,parametro,longitud);
	for (int i=0; i<=padding; i++){//agrega los ceros de padding +\0
		memcpy(offset+8+longitud,&barracero,1);
		offset=offset+1;
	}
	*tamparam=8+longitud+padding;
}

static int _multiplo8(int longitud){
	int l=longitud;
	while((l%8) != 0){
		l=l+1;
	}
	return l;
}

static void _convertir_firma(char **linea,uint8_t tipoparam,uint8_t cantstring
		,char tipodato,uint8_t barracero,int *tamfirma,int *tamfirmaconpad,
		uint8_t cantargumentos){
	uint32_t longconpadding,padding;
	longconpadding=_multiplo8(4+1+cantargumentos+1);
	padding=longconpadding-4-1-cantargumentos-1;
	*tamfirmaconpad=longconpadding;
	*tamfirma=longconpadding-padding;
	*linea=(char*)malloc(longconpadding);
	char *offset=*linea;
	memcpy(offset,&tipoparam,sizeof(tipoparam));
	memcpy(offset+1,&cantstring,sizeof(cantstring));
	memcpy(offset+2,&tipodato,sizeof(tipodato));
	memcpy(offset+3,&barracero,1);
	memcpy(offset+4,&cantargumentos,1);
	for (int i=0; i<cantargumentos; i++){//agrega una s por cada arg
		memcpy(offset+5+i,"s",1);
	}
	for (int i=0; i<=padding; i++){//agrega los ceros de padding +\0
		memcpy(offset+5+cantargumentos+i,&barracero,1);
	}
}

static int _tiene_firma(char *metodo,int tammetodo,char* argumentos,int *cant){
	metodo=metodo+tammetodo;//avanzo la firma del metodo
	memset(argumentos,0,MAX_ARG);
	sscanf(metodo,"(%[^)])",argumentos);//lo de adentro de ()
	if ((strlen(argumentos))==0){
		return 1;
	}
	if(strchr(argumentos,',') != NULL){
		*cant=_contarargumentos(argumentos);
	} else {
		*cant=1;
	}
	return 0;
}

static int _contarargumentos(char *argumentos){
	const char *p=argumentos;
	int cant=0;
	char c=',';
	do{
		if(*p==c)
		 cant++;
	} while (*(p++));

	return cant+1;
}

static void _convertir_body(char *argumentos,char **body,int *tambody,
		                    uint8_t cantargumentos){
	uint8_t barracero=0;
	uint32_t longitud;
	if(cantargumentos!=1){
		longitud=1;
	} else {
		longitud=strlen(argumentos);
	}
	uint32_t totalbody=cantargumentos*(longitud+4+1);//param+tam+\0
	*tambody=totalbody;
	*body=(char*)malloc(totalbody);
	char *offset=*body;
	for(int i=0; i<cantargumentos; i++){
	memcpy(offset+i*6,&longitud,sizeof(longitud));
	memcpy(offset+i*6+4,argumentos,longitud);
	memcpy(offset+i*6+4+longitud,&barracero,1);
	argumentos=argumentos+2;
	}
}

static void _enviardescripcion(client_t *self,uint32_t tamarray,
								uint32_t tambody,char* endian,
								uint8_t tipo, uint32_t id){
	uint8_t barracero=0;
	char *linea=malloc(16);
	char *offset=linea;
	memcpy(offset,endian,1);
	memcpy(offset+1,&tipo,1);
	memcpy(offset+2,&barracero,1);
	memcpy(offset+3,&tipo,1);
	memcpy(offset+4,&tambody,sizeof(tambody));
	memcpy(offset+8,&id,sizeof(id));
	memcpy(offset+12,&tamarray,sizeof(tamarray));
	//TCP send 16 bytes
	TCPsocket_send(&self->socket,linea,TAM_DESCRIPCION);
	free(linea);
}

static void _enviarparametro(client_t *self,char *parametro,
							 uint32_t tamparametro){
	char *linea=malloc(tamparametro);
	memcpy(linea,parametro,tamparametro);
	//TCP send tamparametro bytes
	TCPsocket_send(&self->socket,linea,tamparametro);
	free(parametro);
	free(linea);
}

static void _closeclient(client_t *self){
	TCPsocket_destroy(&self->socket);
}
