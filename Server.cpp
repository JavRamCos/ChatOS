#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

/* Clase Servidor */
class Server {
    private:
        int port;
        int sockt;
    public:
        /* Constructor to define initial params */
        Server(int s_port) {
            this->port = s_port;
        }

        int SetInitParams() {
            /* Creating socket connection */
            this->sockt = socket(AF_INET,SOCK_STREAM,0);
            if(this->sockt < 0) {
                printf("> Error creating socket\n");
                return -1;
            }
            return 0;
        }
};

/* Program for Server, Chat Proyect */
/* Javier Ramirez Cospin 18099 */
/* Cesar Vinicio Rodas 16776 */
int main(int argc,char* argv[]) {
    /* Checking number of parameters */
    if(argc != 2) {
        printf("> Not enough arguments ...\n");
        return EXIT_FAILURE;
    }
    /* Server object creation */
    int port = atoi(argv[1]);
    Server srvr(port);
    return EXIT_SUCCESS;
}
