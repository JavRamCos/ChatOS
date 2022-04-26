#include <iostream>
#include <stdlib.h>

/* Clase Servidor */
class Server {
    private:
        int port;
    public:
        /* Constructor to define initial params */
        Server(int s_port) {
            this->port = s_port;
        }
};

/* Program for Server, Chat Proyect */
/* Javier Ramirez Cospin 18099 */
/* Cesar Vinicio Rodas 16776 */
int main(int argc,char* argv[]) {
    /* Checking number of parameters */
    if(argc != 2) {
        printf("Not enough arguments ...\n");
        return EXIT_FAILURE;
    }
    /* Server object creation */
    int port = atoi(argv[1]);
    Server srvr(port);
    return EXIT_SUCCESS;
}
