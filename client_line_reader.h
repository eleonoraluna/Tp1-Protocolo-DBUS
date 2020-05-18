#ifndef CLIENT_LINE_READER_H_
#define CLIENT_LINE_READER_H_
#include <stdio.h>

typedef struct{
	FILE *input;
} reader_t;

//selecciona el input, si se le pasa un archivo lo abre
//sino, selecciona como input el stdin
//devuelve 1 si no pudo abrir el archivo
int client_reader_create(int argc, char *argv[],reader_t *self);

//copia en la linea lo que va leyendo en buffers de 32 bytes
//si encuentra un /n en un buffer de 32 bytes, guarda lo
//que queda luego del /n en el buffer temporal tmp
//y termina de leer la linea
int _read_line(reader_t *self,char *line,char *tmp, int sizetmp);

//cierra el archivo
int client_reader_close(reader_t *self);

#endif /* CLIENT_LINE_READER_H_ */
