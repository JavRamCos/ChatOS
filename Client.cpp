#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "protocol.pb.h"

#define BUFFSIZE 4096

int main(int argc,char* argv[]) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    /* Parameter verification */
    if(argc != 4) {
        printf("> Incorrect number of parameters\n");
        return  EXIT_FAILURE;
    }
    /* Initial parameters */
    std::string user_name = argv[1];
    char* srvr_name = argv[2];
    int srvr_port = atoi(argv[3]);
    /* Getting server */
    struct hostent* srvr = gethostbyname(srvr_name);
    if(srvr == NULL) {
        printf("> Error getting server\n");
        return  EXIT_FAILURE;
    }
    /* Socket creation */
    int sockt = socket(AF_INET,SOCK_STREAM,0);
    if(sockt < 0) {
        printf("> Error creating socket\n");
        return  EXIT_FAILURE;
    }
    /* Address format */
    struct sockaddr_in srvr_address;
    srvr_address.sin_family = AF_INET;
    srvr_address.sin_port = htons(srvr_port);
    srvr_address.sin_addr = *((struct in_addr*)srvr->h_addr);
    /* Connecting to server */
    int conn = connect(sockt,(struct sockaddr*)&srvr_address,sizeof(srvr_address));
    if(conn < 0) {
        printf("> Error connecting to server\n");
        return EXIT_FAILURE;
    }
    /* User Registration message */
    chat::UserRegistration* user_reg(new chat::UserRegistration);
    user_reg->set_username(user_name);
    user_reg->set_ip(inet_ntoa(srvr_address.sin_addr));
    /* Request user registration */
    chat::ClientRequest clnt_rqst;
    clnt_rqst.set_option(chat::ClientRequest_Option_USER_LOGIN);
    clnt_rqst.set_allocated_newuser(user_reg);
    /* Send request to server */
    std::string srvr_msg;
    clnt_rqst.SerializeToString(&srvr_msg);
    char msg[srvr_msg.size() + 1];
    strcpy(msg,srvr_msg.c_str());
    send(sockt,msg,strlen(msg),0);
    /* Receive Server response */
    char buff[BUFFSIZE] = {0};
    int resp_size = recv(sockt,buff,BUFFSIZE,0);
    if(resp_size < 0) {
        printf("> Error receiving server response\n");
        return EXIT_FAILURE;
    }
    buff[resp_size] = '\0';
    std::string resp = buff;
    /* Handle Server response */
    chat::ServerResponse response;
    response.ParseFromString(resp);
    if(response.code() == 1 && response.option() == 0) {
        printf("> Login successfull\n");
    } else {
        printf("> Login unsuccessfull\n");
        return EXIT_FAILURE;
    }
    /* Client petition variables initiation */
    chat::ClientRequest clnt_pet;
    std::string rqst_str;
    std::string text;
    std::string receiver;
    pthread_t thd;
    return EXIT_SUCCESS;
}