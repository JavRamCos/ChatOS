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

void* MessageHandler(void* args);
int user_count = 0;
std::vector<USER> users;
pthread_mutex_t public_chat = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t private_chat = PTHREAD_MUTEX_INITIALIZER;

/* Server class */
class Server {
    private:
        int port;
        int sockt;
        struct sockaddr_in srvr_address;
        struct sockaddr_in clnt_address;
        pthread_t pool[MAXUSERS];
    public:
        /* Constructor to define initial class params */
        Server(int s_port) {
            this->port = s_port;
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
            while(user_count < MAXUSERS) {
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
                    continue;
                }
                buff[vread] = '\0';
                std::string clnt_msg = buff;
                /* Parse new user petition */
                chat::ClientRequest clnt_rqst;
                clnt_rqst.ParseFromString(clnt_msg);
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
                        new_user.sockt = n_sockt;
                        /* Append new user to user list */
                        users.push_back(new_user);
                        /* Set correct server response */
                        response.set_option(chat::ServerResponse_Option_USER_LOGIN);
                        response.set_code(chat::ServerResponse_Code_SUCCESSFUL_OPERATION);
                        response.set_response("Successful User Registration");
                        /* Create new user thread */
                        pthread_create(&this->pool[user_count],NULL,MessageHandler,(void*)&new_user);
                        user_count += 1;
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
                response.SerializeToString(&clnt_msg);
                char msg[clnt_msg.size() + 1];
                strcpy(msg,clnt_msg.c_str());
                send(n_sockt,msg,strlen(msg),0);
            }
        } 
};

/* Function to create new thread with new user */
void* MessageHandler(void* args) {
    int open_conn = 1;
    /* Handle user incoming messages */
    struct USER *n_user = (struct USER*)args;
    int u_sockt = n_user->sockt;
    while(open_conn) {
        /* Read information from socket */
        char buff[BUFFSIZE] = {0};
        int vread = recv(u_sockt,buff,BUFFSIZE,0);
        if(vread < 0) {
            printf("> Error reading socket information\n");
            continue;
        } else {
            buff[vread] = '\0';
            /* Read client request */
            std::string clnt_msg = buff;
            chat::ClientRequest clnt_rqst;
            clnt_rqst.ParseFromString(clnt_msg);
            /* Handle client requests */
            int opt = clnt_rqst.option();
            switch(opt) {
                case 0:
                    /* User registration */
                    printf("> User already registered\n");
                    break;
                case 1: {
                    /* Connected users */
                    chat::ConnectedUsers* conn_users(new chat::ConnectedUsers);
                    for(struct USER& user: users) {
                        /* Obtain connected users information */
                        chat::UserInformation* user_info = conn_users>add_users();
                        user_info->set_username(user.username);
                        user_info->set_ip(user.ip);
                        user_info->set_status(user.status);
                    }
                    /* Set Server response */
                    chat::ServerResponse response;
                    response.set_option(chat::ServerResponse_Option_USER_LOGIN);
                    response.set_code(chat::ServerResponse_Code_SUCCESSFUL_OPERATION);
                    response.set_allocated_users(conn_users);
                    /* Send Server response */
                    std::string clnt_msg;
                    response.SerializeToString(&clnt_msg);
                    char msg[clnt_msg.size() + 1];
                    strcpy(msg,clnt_msg.c_str());
                    send(u_sockt,msg,strlen(msg),0);
                    break;
                }
                case 2: {
                    /* User information */
                    std::string clnt_name = clnt_rqst.user().user();
                    chat::ServerResponse response;
                    if(clnt_name == "all") {
                        /* If client wants all users information */
                        printf("> Functionality not implemented, use Connected Users instead\n");
                        /* Set Server response */
                        response.set_option(chat::ServerResponse_Option_USER_INFORMATION);
                        response.set_code(chat::ServerResponse_Code_FAILED_OPERATION);
                        chat::UserInformation* users_info(new chat::UserInformation);
                        response.set_allocated_user(users_info);
                    } else {
                        /* If client wants specific user information */
                        chat::UserInformation* user_info(new chat::UserInformation);
                        int flag = 0;
                        for(struct USER& user: users) {
                            /* Check if username exists */
                            if(user.username == clnt_name) {
                                user_info.set_username(user.username);
                                user_info.set_ip(user.ip);
                                user_info.set_status(user.status);
                                flag = 1;
                            }
                        }
                        if(flag) {
                            /* Username found */
                            response.set_option(chat::ServerResponse_Option_USER_INFORMATION);
                            response.set_code(chat::ServerResponse_Code_SUCCESSFUL_OPERATION);
                            response.set_allocated_user(user_info);
                        } else {
                            /* Username not found */
                            response.set_option(chat::ServerResponse_Option_USER_INFORMATION);
                            response.set_code(chat::ServerResponse_Code_FAILED_OPERATION);
                            response.set_allocated_user(user_info);
                        }
                    }
                    /* Send Server response */
                    std::string clnt_msg;
                    response.SerializeToString(&clnt_msg);
                    char msg[clnt_msg.size() + 1];
                    strcpy(msg,clnt_msg.c_str());
                    send(u_sockt,msg,strlen(msg),0);
                    break;
                }
                case 3: {
                    /* Change status */
                    chat::ChangeStatus* new_status(new chat::ChangeStatus);
                    chat::ServerResponse response;
                    int flag = 0;
                    for(struct USER& user: users) {
                        /* Check if username exists */
                        if(user.username = clnt_rqst.status().username()) {
                            user.status = clnt_rqst.status().status();
                            new_status->set_username(clnt_rqst.status().username());
                            new_status->set_status(clnt_rqst.status().status());
                            flag = 1;
                        }
                    }
                    if(flag) {
                        /* Username found */
                        response.set_option(chat::ServerResponse_Option_STATUS_CHANGE);
                        response.set_code(chat::ServerResponse_Code_SUCCESSFUL_OPERATION);
                        response.set_allocated_status(new_status);
                    } else {
                        /* Username not found */
                        response.set_option(chat::ServerResponse_Option_STATUS_CHANGE);
                        response.set_code(chat::ServerResponse_Code_FAILED_OPERATION);
                        response.set_allocated_status(new_status);
                    }
                    /* Send Server response */
                    std::string clnt_msg;
                    response.SerializeToString(&clnt_msg);
                    char msg[clnt_msg.size() + 1];
                    strcpy(msg,clnt_msg.c_str());
                    send(u_sockt,msg,strlen(msg),0);
                    break;   
                }
                case 4: {
                    /* Send message (Chat) */
                    std::string mode = clnt_rqst.messg().receiver();
                    if(mode == "all") {
                        /* Public chat */
                        pthread_mutex_lock(&public_chat);
                        /* Set message object */
                        chat::Message* messsage(new chat::Message);
                        messsage->set_receiver(clnt_rqst.messg().receiver());
                        messsage->set_sender(clnt_rqst.messg().sender());
                        messsage->set_text(clnt_rqst.messg().text());
                        /* Set Server response */
                        chat::ServerResponse response;
                        response.set_option(chat::ServerResponse_Option_SEND_MESSAGE);
                        response.set_code(chat::ServerResponse_Code_SUCCESSFUL_OPERATION);
                        response.set_allocated_messg(messsage);
                        std::string clnt_msg;
                        response.SerializeToString(&clnt_msg);
                        char msg[clnt_msg.size() + 1];
                        strcpy(msg,clnt_msg.c_str());
                        /* Send message to all users */
                        for(struct USER& user: users) {
                            send(user.sockt,msg,strlen(msg),0);
                        }
                        pthread_mutex_unlock(&public_chat);
                    } else {
                        /* Private chat */
                    }
                    break;
                }
                default:
                    /* Unknown operation (Error) */
                    printf("> Unknown operation, try again...\n");
                    break;
            }
        }
    }
    /* Close user socket connection */
    close(u_sockt);
    user_count -= 1;
    if(user_count < 0) { user_count = 0; }
    /* Close user thread */
    pthread_exit(NULL);
}

/* Program for Server, Chat Proyect */
/* Javier Ramirez Cospin 18099 */
/* Cesar Vinicio Rodas 16776 */
int main(int argc,char* argv[]) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
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
