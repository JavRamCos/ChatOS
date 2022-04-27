# ChatOS

Proyecto de Chat para curso de Sistemas Operativos, 2022. Implementación de Servidor y Cliente, utilizando protocolo de protobuff.

## Integrantes:

 - Javier Ramirez Cospin, carne No. 18099
 - Cesar Vinicio Rodas, carne No. 16776

## Instrucciones:

### 1. Compilar protocolo

Compilar el protocolo con el siguiente comando en un sistema Linux

```
$ protoc -I=<Directorio source> --cpp_out=<Directorio destino> <Directorio source>/protocol.proto
```

En donde:

- **Directorio source** : Directorio en donde se encuentra el archivo protocolo.proto
- **Directorio destino** : Directorio en donde se va a compilar el archivo protocolo.proto

### 2. Servidor

Compilar el servidor con el siguiente comando en un sistema Linux

```
$ g++ -std=c++11 -I <Directorio include> -L <Directorio lib> Server.cpp protocol.pb.cc -lprotobuf -pthread -o Server
```

En donde:

- **Directorio include** : Directorio en donde se encuentra el directorio /include
- **Directorio lib** : Directorio en donde se encuentra el directorio /lib

NOTA : Este comando asume que se está realizando en el mismo directorio en donde se encuentra el protocolo compilado y el archivo Server.cpp

Una vez compilado el servidor, ejecutar con

```
$ ./Server <puerto>
```

En donde:

- **puerto** : Número de puerto de elección

