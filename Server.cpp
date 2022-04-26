#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include "protocol.pb.h"

/* Struct to store user information */
struct USER {
    std::string username;
    std::string ip;
    std::string status;
}

/* Server class */
class Server {
    private:
        int port;
        int sockt;
        struct sockaddr_in srvr_address;
        struct sockaddr_in clnt_address;
        int buff_size;
        int max_users;
        std::vector<USER> users;
    public:
        /* Constructor to define initial class params */
        Server(int s_port) {
            this->port = s_port;
            this->buff_size = 2048;
            this->max_users = 10;
        }

        /* Deconstructor to handle file and libraries closing */
        ~Server() {
            google::protobuf::ShutdownProtobufLibrary();
        }

        /* Function to define server's initial parameters */
        int SetInitParams() {
            /* Creating socket connection */
            this->sockt = socket(AF_INET,SOCK_STREAM,0);
            if(this->sockt < 0) {
                printf("> Error creating socket\n");
                return -1;
            }
            /* Address format */
            this->srvr_address.sin_family = AF_INET;
            this->srvr_address.sin_port = htons(this->port);
            this->srvr_address.sin_addr.s_addr = INADDR_ANY;
            /* Socket binding */
            int bind_flag = bind(this->sockt,(struct sockaddr*)&this->srvr_address,sizeof(this->srvr_address));
            if(bind_flag < 0) {
                printf("> Error binding socket\n");
                return -1;
            }
            /* Socket listening */
            int lstn_flag = listen(this->sockt,this->max_users);
            if(lstn_flag < 0) {
                printf("> Error socket listen\n");
                return -1;
            }
            return 0;
        }

        /* Function to check and accept socket connections */
        void CheckConnections() {
            while(1) {
                /* Accept sockets requests */
                int n_sockt = accept(this->sockt,(struct sockaddr*)&this->clnt_address,(socklen_t*)sizeof(this->clnt_address));
                if(n_sockt < 0) {
                    printf("> Error accepting requests\n");
                }
                /* Read socket information */
                char buff[this->buff_size] = {0};
                int vread = recv(n_sockt,buff,this->buff_size,0);
                if(vread < 0) {
                    printf("> Error reading socket information\n");
                }
                buff[vread] = '\0';
                std::string ser_str = buff;
                /* Parse new user petition */
                chat::ClientRequest clnt_rqst;
                clnt_rqst.ParseFromString(ser_str);
                /* Server response */
                chat::ServerResponse response;
                /* Handle new user registration */
                if(clnt_rqst.option() == 0) {
                    printf("> New user registration ...\n");
                    /* Check if new username is already registered */
                    int flag = 1;
                    for(auto user = users.begin();user != users.end();user++) {
                        if(user->username == clnt_rqst.newuser().username()) {
                            /* Username is already registered */
                            printf("> Username is already registered\n");
                            flag = 0;
                            break;
                        }
                    }
                    if(flag) {
                        /* Username is not registered */
                        struct USER new_user;
                        new_user.username = clnt_rqst.newuser().username();
                        new_user.ip = clnt_rqst.newuser().ip();
                        new_user.status = "ACTIVE";
                        /* Append new user to user list */
                        users.push_back(new_user);
                        /* Set correct server response */
                        response.set_option(chat::ServerResponse_Option_USER_LOGIN);
                        response.set_code(chat::ServerResponse_Code_SUCCESSFUL_OPERATION);
                        response.set_response("Successful User Registration");
                    } else {
                        response.set_option(chat::ServerResponse_Option_USER_LOGIN);
                        response.set_code(chat::ServerResponse_Code_FAILED_OPERATION);
                        response.set_response("Unsuccessful User Registration");
                    }
                } else {
                    response.set_option(chat::ServerResponse_Option_USER_LOGIN);
                    response.set_code(chat::ServerResponse_Code_FAILED_OPERATION);
                    response.set_response("Unsuccessful User Registration");
                }
                /* Send server response to socket */
                response.SerializeToString(&ser_str);
                char msg[ser_str.size() + 1];
                strcpy(msg,ser_str.c_str());
                send(n_sockt,msg,strlen(msg),0);
            }
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
    /* Setting server initial parameters */
    srvr.SetInitParams();
    return EXIT_SUCCESS;
}
