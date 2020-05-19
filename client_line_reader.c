#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "client_line_reader.h"

const int ARGS_CLIENT_WITH_FILE=4;
const int BUFFER=32;

int client_reader_create(int argc, char *argv[],reader_t *self){
	if (argc==ARGS_CLIENT_WITH_FILE){
		self->input=fopen(argv[3],"rt");
		if (!self->input) {
			printf("No se pudo abrir el archivo\n");
			return 1;
		}
	}else{
		self->input= stdin;
	}
	return 0;
}

static void save_nextline(char *line,char *buffer,char *tmp,int sizetmp,
		                  int offset){
	 uint32_t dif,size_newline;
	 char* ptr_newline=strchr(buffer,'\n');
	 dif=&*ptr_newline-buffer;
	 size_newline=BUFFER-dif;
	 //lo que quedo despues del\n
	 memcpy(tmp,ptr_newline+1,size_newline);
	 //lo que quedo hasta el \n
	 memcpy(line+sizetmp+(offset*BUFFER),buffer,dif);
}

int _read_line(reader_t *self,char *line,char *tmp, int sizetmp){
	 char buffer[BUFFER+1];
	 int line_complete=0,offset=0,read_elements=0;
	 memcpy(line,tmp,sizetmp);
	 memset(tmp,0,sizetmp);
	 while (line_complete==0){
		 memset(buffer,0,sizeof(buffer));
		 read_elements=fread(buffer,BUFFER, 1,self->input);
		 if (strchr(buffer,'\n') != NULL) {
			save_nextline(line,buffer,tmp,sizetmp,offset);
			line_complete=1;
		 }else{
			 memcpy(line+sizetmp+(offset*BUFFER),buffer,sizeof(buffer));
			offset=offset+1;
		 }
	 }
	 return read_elements;
}

int client_reader_close(reader_t *self){
	if (self->input!=NULL) {
		fclose(self->input);
	}
	return 0;
}


