#ifndef COMMON_TCPSOCKET_H_
#define COMMON_TCPSOCKET_H_

typedef struct{
	int fd;
}sockTCP_t;

void TCPsocket_create(sockTCP_t *self);
int TCPsocket_bind(sockTCP_t *self,char *puerto);
int TCPsocket_listen(sockTCP_t *self);
int TCPsocket_accept(sockTCP_t *self);
int TCPsocket_connect(sockTCP_t *self, char *name, char *p);
int TCPsocket_recieve(sockTCP_t *self, char *buffer, int length);
int TCPsocket_send(sockTCP_t *self, const char *buffer, int length);
int TCPsocket_destroy(sockTCP_t *self);

#endif /* COMMON_TCPSOCKET_H_ */
