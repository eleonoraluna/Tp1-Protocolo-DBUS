#define _POSIX_C_SOURCE 200112L
#include "common_TCPsocket.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

void TCPsocket_create(sockTCP_t *self){
	self->fd=-1;
}

int TCPsocket_bind(sockTCP_t *self,char *puerto){
	struct addrinfo hints;
	struct addrinfo *ptr;

	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family=AF_INET; //IPv4
	hints.ai_socktype=SOCK_STREAM; //TCP
	hints.ai_flags=AI_PASSIVE; // AI_PASSIVE for server

	if (getaddrinfo(0,puerto,&hints,&ptr)!=0){
		fprintf(stderr, "Error in getaddrinfo: \n");
		return 1;
	}

	if ((self->fd=socket(ptr->ai_family,ptr->ai_socktype,ptr->ai_protocol))==-1){
		fprintf(stderr,"Error:socket creating failed: %s\n",strerror(errno));
		freeaddrinfo(ptr);
		return 1;
	}

	if (bind(self->fd,ptr->ai_addr,ptr->ai_addrlen)==-1){
	    fprintf(stderr,"Error:binding failed: %s\n",strerror(errno));
	    freeaddrinfo(ptr);
	    close(self->fd);
	    return 1;
	}
	freeaddrinfo(ptr);
	return 0;
}

int TCPsocket_listen(sockTCP_t *self){
	if (listen(self->fd,1)==-1){
		fprintf(stderr,"Error:listening failed: %s\n",strerror(errno));
		close(self->fd);
		return 1;
	}
	return 0;
}

int TCPsocket_accept(sockTCP_t *self){
	if((self->fd=accept(self->fd,NULL,NULL))==-1){
		fprintf(stderr,"Error:accepting failed: %s\n",strerror(errno));
	}
	return self->fd;
}

int TCPsocket_connect(sockTCP_t *self, char *name, char *puerto){
	struct addrinfo hints;
	struct addrinfo *ptr;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family=AF_INET; //IPv4
	hints.ai_socktype=SOCK_STREAM; //TCP
	hints.ai_flags=0;

	if (getaddrinfo(name,puerto,&hints,&ptr) !=0){
		fprintf(stderr,"Error in getaddrinfo \n");
		return 1;
	}

	if ((self->fd=socket(ptr->ai_family,ptr->ai_socktype,ptr->ai_protocol))==-1){
		fprintf(stderr,"Error:socket creating failed: %s\n",strerror(errno));
		freeaddrinfo(ptr);
		return 1;
	}

	if (connect(self->fd,ptr->ai_addr,ptr->ai_addrlen)<0){
		fprintf(stderr,"Error: connect failed %s\n",strerror(errno));
		return 1;
	}
	freeaddrinfo(ptr);
	return 0;
}

int TCPsocket_recieve(sockTCP_t *self, char *buffer, int length){
	int pos=0;
	int recieved=0;
	while(pos<length){
		recieved=recv(self->fd,&buffer[pos],length-pos,MSG_NOSIGNAL);
		//sigo recibiendo hasta recibir todos los bytes
		if (recieved>0){
			pos=pos+recieved;
		//recibo 0 bytes. Hicieron un shutdown o llego al EOF
		} else if (recieved==0){
			//tengo que cerrar socket
			return -1;
		} else {
			fprintf(stderr,"Error:recieve failed %s\n",strerror(errno));
			return 1;
		}
	}
	return 0;
}

int TCPsocket_send(sockTCP_t *self, const char *buffer, int length){
	int pos=0;
	int sent=0;
	while(pos<length){
		sent=send(self->fd,&buffer[pos],length-pos,MSG_NOSIGNAL);
		if (sent>=0){ //sigo mandando hasta que se envien todos los bytes
			pos=pos+sent;
		} else {//error en el send
			fprintf(stderr,"Error: send failed %s\n",strerror(errno));
			return 1;
		}
	}
	return 0;
}

int TCPsocket_destroy(sockTCP_t *self){
	if (shutdown(self->fd,SHUT_RDWR)==-1){
		fprintf(stderr,"Error: shutdown failed %s\n",strerror(errno));
		return 1;
	}
	if (close(self->fd)==-1){
		fprintf(stderr,"Error: close failed %s\n",strerror(errno));
		return 1;
	}
	self->fd=-1;
	return 0;
}
