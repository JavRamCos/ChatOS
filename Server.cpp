#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <pthread.h>
#include "protocol.pb.h"

#define MAXUSERS 10
#define BUFFSIZE 2048

/* Struct to store user information */
struct USER {
    std::string username;
    std::string ip;
    std::string status;
    int sockt;
}

/* Server class */
class Server {
    private:
        int port;
        int sockt;
        struct sockaddr_in srvr_address;
        struct sockaddr_in clnt_address;
        int user_count;
        std::vector<USER> users;
        pthread_t pool[MAXUSERS];
    public:
        /* Constructor to define initial class params */
        Server(int s_port) {
            this->port = s_port;
            this->user_count = 0;
        }

        /* Deconstructor to handle file and libraries closing */
        ~Server() {
            close(this->sockt);
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
            int lstn_flag = listen(this->sockt,MAXUSERS);
            if(lstn_flag < 0) {
                printf("> Error socket listen\n");
                return -1;
            }
            return 0;
        }

        /* Function to check and accept socket connections */
        void CheckConnections() {
            while(this->user_count < MAXUSERS) {
                /* Accept sockets requests */
                int n_sockt = accept(this->sockt,(struct sockaddr*)&this->clnt_address,(socklen_t*)sizeof(this->clnt_address));
                if(n_sockt < 0) {
                    printf("> Error accepting requests\n");
                }
                /* Read socket information */
                char buff[BUFFSIZE] = {0};
                int vread = recv(n_sockt,buff,BUFFSIZE,0);
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
                    for(auto user = this->users.begin();user != this->users.end();user++) {
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
                        new_user.sockt = n_sockt;
                        /* Append new user to user list */
                        this->users.push_back(new_user);
                        /* Set correct server response */
                        response.set_option(chat::ServerResponse_Option_USER_LOGIN);
                        response.set_code(chat::ServerResponse_Code_SUCCESSFUL_OPERATION);
                        response.set_response("Successful User Registration");
                        pthread_create(&this->pool[this->user_count],NULL,MessageHandler,(void*)&new_user);
                        this->user_count += 1;
                    } else {
                        /* Set error server response (Username already registered) */
                        response.set_option(chat::ServerResponse_Option_USER_LOGIN);
                        response.set_code(chat::ServerResponse_Code_FAILED_OPERATION);
                        response.set_response("Username already registered");
                    }
                } else {
                    /* Set error server response (Unsuccessful user registration) */
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

        /* Function to create new thread with new user */
        static void* MessageHandler(void* args) {
            char buff[BUFFSIZE];
            struct USER *n_user = (struct USER*)args;
            int open_conn = 1;
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
    printf("> Creating Server ...\n");
    int port = atoi(argv[1]);
    Server srvr(port);
    printf("> Server created successfully\n");
    /* Setting server initial parameters */
    printf("> Setting up Server ...\n");
    int init_flag = srvr.SetInitParams();
    if(init_flag < 0) {
        return EXIT_FAILURE;
    }
    printf("> Server set up successfully\n");
    return EXIT_SUCCESS;
}
