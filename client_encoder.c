#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "client_encoder.h"
#include "client_line_reader.h"

const int READ_BUFFER=32;
const int MAX_LINE=176;
const int MAX_DES=19;
const int MAX_PATH=17;
const int MAX_INT=25;
const int MAX_MET=128;
const int MAX_ARG=64;
const int TYPE_PARAM=4;
const int DESCRIPTION_SIZE=16;
const int MAX_SIZE_ARG=70;
const int SIZE_MESSAGE_RCV=3;


int client_encoder_create(encoder_t *self,char* argv[]){
	int connect;
	TCPsocket_create(&self->socket);
	connect=TCPsocket_connect(&self->socket,argv[1],argv[2]);
	if (connect==1){
		return 1;
	}
	return 0;
}

static void _rcv_message(encoder_t *self,uint32_t id){
	char message[SIZE_MESSAGE_RCV];
	TCPsocket_recieve(&self->socket,message,sizeof(message));
	printf("0x%08x: %s",id,message);
}

static int _multiple8(int length){
	int l=length;
	while((l%8) != 0){
		l=l+1;
	}
	return l;
}

static void _encode_parameter(char *parameter,char **line,uint8_t paramtype
		,uint8_t stringcount,char datatype, uint8_t zero,int *paramlength){
	uint32_t lengthpadding,padding;
	uint32_t length=strlen(parameter);
	lengthpadding=_multiple8(length+1);
	padding=lengthpadding-length;
	*line=(char*)malloc(4+4+length+1+padding);
	char *offset=*line;
	memcpy(offset,&paramtype,sizeof(paramtype));
	memcpy(offset+1,&stringcount,sizeof(stringcount));
	memcpy(offset+2,&datatype,sizeof(datatype));
	memcpy(offset+3,&zero,1);
	memcpy(offset+4,&length,sizeof(length));
	memcpy(offset+8,parameter,length);
	for (int i=0; i<=padding; i++){//agrega los ceros de padding +\0
		memcpy(offset+8+length,&zero,1);
		offset=offset+1;
	}
	*paramlength=8+length+padding;
}

static void _encode_firm(char **line,uint8_t paramtype,uint8_t stringcount
		,char datatype,uint8_t zero,int *firmlength,int *firmlengthpadding,
		uint8_t argumentscount){
	uint32_t lengthpadding,padding;
	lengthpadding=_multiple8(4+1+argumentscount+1);
	padding=lengthpadding-4-1-argumentscount-1;
	*firmlengthpadding=lengthpadding;
	*firmlength=lengthpadding-padding;
	*line=(char*)malloc(lengthpadding);
	char *offset=*line;
	memcpy(offset,&paramtype,sizeof(paramtype));
	memcpy(offset+1,&stringcount,sizeof(stringcount));
	memcpy(offset+2,&datatype,sizeof(datatype));
	memcpy(offset+3,&zero,1);
	memcpy(offset+4,&argumentscount,1);
	for (int i=0; i<argumentscount; i++){//agrega una s por cada arg
		memcpy(offset+5+i,"s",1);
	}
	for (int i=0; i<=padding; i++){//agrega los ceros de padding +\0
		memcpy(offset+5+argumentscount+i,&zero,1);
	}
}

static int _count_arguments(char *arguments){
	const char *p=arguments;
	int count=0;
	char c=',';
	do{
		if(*p==c)
		 count++;
	} while (*(p++));

	return count+1;
}

static int _firm_exists(char *method,int lengthmethod,char* arguments,
		                int *count){
	method=method+lengthmethod;//avanzo la firma del metodo
	memset(arguments,0,MAX_ARG);
	sscanf(method,"(%[^)])",arguments);//lo de adentro de ()
	if ((strlen(arguments))==0){
		return 1;
	}
	if(strchr(arguments,',') != NULL){
		*count=_count_arguments(arguments);
	} else {
		*count=1;
	}
	return 0;
}

static void _encode_body(char *arguments,char **body,int *bodylength,
		                    uint8_t argumentscount){
	char argument[MAX_SIZE_ARG];
	uint8_t zero=0;
	uint32_t argumentsize;
	*body=(char*)malloc(argumentscount*(MAX_SIZE_ARG+4+1));
	char *offset=*body;
	for (int i=0; i<argumentscount; i++){
		sscanf(arguments,"%[^,],",argument);
		argumentsize=strlen(argument);
		memcpy(offset,&argumentsize,sizeof(argumentsize));//copio el tamanio
		memcpy(offset+4,argument,argumentsize);//copio el argumento
		memcpy(offset+4+argumentsize,&zero,1);//el /0
		offset=offset+argumentsize+4+1;//sig pos donde quiero copiar
		arguments=arguments+argumentsize+1;//apunto al sig arg
		*bodylength=*bodylength+4+1+argumentsize;
	}
}

static void _send_description(encoder_t *self,uint32_t arraysize,
								uint32_t bodylength,char* endian,
								uint8_t type, uint32_t id){
	uint8_t zero=0;
	char *line=calloc(4,4);//tamanio descripcion 16 bytes
	char *offset=line;
	memcpy(offset,endian,1);
	memcpy(offset+1,&type,1);
	memcpy(offset+2,&zero,1);
	memcpy(offset+3,&type,1);
	memcpy(offset+4,&bodylength,sizeof(bodylength));
	memcpy(offset+8,&id,sizeof(id));
	memcpy(offset+12,&arraysize,sizeof(arraysize));
	TCPsocket_send(&self->socket,line,DESCRIPTION_SIZE);
	free(line);
}

static void _send_parameter(encoder_t *self,char *parameter,
							 uint32_t parametersize){
	char *line=malloc(parametersize);
	memcpy(line,parameter,parametersize);
	TCPsocket_send(&self->socket,line,parametersize);
	free(parameter);
	free(line);
}

static void _encode_line(encoder_t *self,char *line,uint32_t id){
	char dest[MAX_DES],path[MAX_PATH],interface[MAX_INT],method[MAX_MET],
			methodname[MAX_MET],arguments[MAX_ARG];
	int arraysize=0,destsize,pathsize,intsize,metsize,firmsize=0,
		firmsizepad,bodysize=0,argcount;
	char* d; char* p; char* i; char* m; char* f; char* b;
	sscanf(line,"%s %s %s %[^\n]",dest,path,interface,method);
	_encode_parameter(dest,&d,6,1,'s',0,&destsize);
	_encode_parameter(path,&p,1,1,'o',0,&pathsize);
	_encode_parameter(interface,&i,2,1,'s',0,&intsize);
	sscanf(method," %[^(] ",methodname);
	_encode_parameter(methodname,&m,3,1,'s',0,&metsize);
	if(_firm_exists(method,strlen(methodname),arguments,&argcount)==0){
		_encode_firm(&f,8,1,'g',0,&firmsize,&firmsizepad,argcount);
		_encode_body(arguments,&b,&bodysize,argcount);
	}
	arraysize=destsize+pathsize+intsize+metsize+firmsize;
	_send_description(self,arraysize,bodysize,"l",1,id);
	_send_parameter(self,d,destsize);
	_send_parameter(self,p,pathsize);
	_send_parameter(self,i,intsize);
	_send_parameter(self,m,metsize);
	if(_firm_exists(method,strlen(methodname),arguments,&argcount)==0){
		_send_parameter(self,f,firmsizepad);
		_send_parameter(self,b,bodysize);
	}
}
void client_encoder_run(encoder_t *self,int argc, char *argv[]){
	   reader_t reader;
	   if (client_reader_create(argc,argv,&reader)==0){
	   	int end=1;
	   	uint32_t id=1;
	   	char tmp[READ_BUFFER+1];
	   	char line[MAX_LINE];
	   	memset(tmp,0,sizeof(tmp));
	   	memset(line,0,MAX_LINE);
	   	while(end!=0){
	   		end=_read_line(&reader,line,tmp,strlen(tmp));
	   		 _encode_line(self,line,id);
	   		 _rcv_message(self,id);
	   		memset(line,0,MAX_LINE);
	   		id=id+1;
	   	}
	   	client_reader_close(&reader);
	   }
}

void client_encoder_destroy(encoder_t *self){
	int destroy;
	destroy=TCPsocket_destroy(&self->socket);
	if(destroy==1){
		printf("Error al destruir socket \n");
	}
}
