# Tp1-Protocolo DBUS

**Apellido y nombre:** Eleonora Luna 
**Padron:** 96444

**Repositorio de Github:** https://github.com/eleonoraluna/tp1.git

## Informe

### 1. Arquitectura General

Para la solución del trabajo práctico se implementaron varios TDA's que son usados por el cliente y 
el servidor desarrollados en sus respectivos main.

### Client

Debe recibir un host, un puerto al cual conectarse y un archivo, ya sea por parámetro o por stdin.
Este último se utilizará para leer los mensajes que deberán codificarse.

```
	Ej: ./client localhost 8080 archivo.txt
	    ./client localhost 8080 < archivo.txt
```

El programa cliente hace uso de un encoder_t para codificar los mensajes y enviárselos al servidor.

### Server

El servidor recibe un puerto al cual debe conectarse.
```
	Ej: ./server 8080
```
Hace uso de un decoder_t a través del cual recibe las llamadas codificadas por el cliente y las decodifica
implementando el protocolo establecido para luego imprimir por pantalla los datos pedidos.El servidor termina 
cuando el cliente cierra el canal.

### sockTCP_t

Este TDA representa un socket para protocolo TCP. Se encarga de recibir y enviar los 
buffers que le pasan por parámetros. También se encarga de asegurarse que se envíen y 
reciban todos los bytes correspondientes.

### encoder_t

Este TDA es owner del socket que será usado para comunicarse con el servidor. Se encarga de codificar los
mensajes que se leen desde el archivo o stdin utilizando el protocolo establecido y enviarlos a través 
del socket hacia el servidor. Para leer los mensajes utiliza el TDA reader_t. Se ocupa de crear y 
destruir el socket.

### reader_t

Recibe los parámetros de entrada del programa cliente y a partir de ello decide si va a leer desde un
archivo o desde stdin. Lee de a 32 bytes y una vez que encuentra un "\n" devuelve la línea. Lee hasta
que se termine el archivo o en su defecto stdin. Se encarga de abrir y cerrar el archivo.

### decoder_t

Owner de un sockTCP_t que se usará para comunicarse con el client. Recibe las llamadas codificadas
por el cliente y las decodifica implementando el protocolo establecido. Por cada linea recibida imprime 
por pantalla los datos pedidos y le envía un "OK"al cliente. Se ocupa de crear y destruir el socket.

### 2. Diagrama de la solución

A continuación se muestra un diagrama de secuencia de la solución que se implementó.
Si bien no estamos utilizando objetos lo que se intenta mostrar es cómo interactúan los TDAs entre si.


<p align="center">
<img src="diagrama_secuencia.png">
</p>




