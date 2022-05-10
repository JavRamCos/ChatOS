#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <netdb.h>
#include <pthread.h>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "protocol.pb.h"

#define BUFFSIZE 4096

int sockt;
pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t thdcond = PTHREAD_COND_INITIALIZER;
std::string status = "ACTIVE";
std::vector<chat::Message> gen_chat;
std::map<std::string,std::vector<chat::Message>> prv_chat;

/* Function to create new thread with new client */
void* RequestHandler(void* args) {
    while(1) {
        /* Receive Server response */
        char buff[BUFFSIZE] = {0};
        int resp_size = recv(sockt,buff,BUFFSIZE,0);
        buff[resp_size] = '\0';
        std::string resp = buff;
        /* Handle Server response */
        chat::ServerResponse response;
        response.ParseFromString(resp);
        int opt = response.option();
        switch(opt) {
            case 1: {
                /* Connected Users */
                for(const chat::UserInformation& user : repsonse.users().users()) {
                    std::string usr_name = user.username();
                    std::string user_ip = user.ip();
                    std::string user_status = user.status();
                    printf("Username: %s\n",usr_name.c_str());
                    printf("User IP: %s\n",user_ip.c_str());
                    printf("User Status: %s\n",user_status.c_str);
                }
                pthread_cond_signal(&thdcond);
                break;
            }
            case 2: {
                /* User information */
                std::string usr_name = response.user().username();
                std::string user_ip = response.user().ip();
                std::string user_status = response.user().status();
                pritnf("Username: %s\n",usr_name.c_str());
                printf("User IP: %s\n",user_ip.c_str());
                printf("User Status: %s\n",user_status.c_str());
                pthread_cond_signal(&thdcond);
                break;
            }
            case 3: {
                /* Change status */
                status = response.status().status();
                printf("New status: %s\n",status.c_str());
                pthread_cond_signal(&thdcond);
                break;
            }
            case 4: {
                /* Messages */
                if(response.messg().receiver() == "all") {
                    gen_chat.push_back(response.messg());
                } else {
                    if(prv_chat.find(response.messg().sender()) == prv_chat.end()) {
                        std::vector<chat::Message> n_msg;
                        n_msg.push_back(response.messg());
                        prv_chat[response.messg().sender()] = n_msg;
                    } else {
                        std::vector<chat::Message> msgs = prv_chat[response.messg().sender()];
                        msgs.push_back(response.messg());
                        prv_chat[response.messg().sender()] = msgs;
                    }
                }
                break;
            }
            default: {
                printf("Unknown operation\n");
                break;
            }
        }
    }
}

/* Program for Client, Chat Proyect */
/* Javier Ramirez Cospin 18099 */
/* Cesar Vinicio Rodas 16776 */
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
    sockt = socket(AF_INET,SOCK_STREAM,0);
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
    if(response.code() == chat::ServerResponse_Code_SUCCESSFUL_OPERATION && response.option() == chat::ServerResponse_Option_USER_LOGIN) {
        printf("> Login successfull\n");
    } else if(response.code() == chat::ServerResponse_Code_FAILED_OPERATION && response.option() == chat::ServerResponse_Option_USER_LOGIN) {
        printf("> Login unsuccessfull\n");
        perror("> Login: ");
        return EXIT_FAILURE;
    } else {
        printf("> Error retrieving server response\n");
        perror("> Server response: ");
        return EXIT_FAILURE;
    }
    /* Client petition variables initiation */
    int opt;
    std::string text;
    pthread_t thd;
    pthread_create(&thd,NULL,RequestHandler,NULL);
    int lp_flag = 1;
    /* Client main loop */
    while(lp_flag) {
        /* Display options to user */
        printf("Options: \n");
        printf("1. See connected users\n");
        printf("2. Request User information\n");
        printf("3. Change status\n");
        printf("4. General chat\n");
        printf("5. Private message\n");
        printf("6. Exit\n");
        scanf("%d",&opt);
        /* Handle user option */
        switch(opt) {
            case 1: {
                /* Connected users */
                chat::UserRequest* user_rqst(new chat::UserRequest);
                user_rqst->set_user("all");
                chat::ClientRequest clnt_pet;
                clnt_pet.set_option(chat::ClientRequest_Option_CONNECTED_USERS);
                clnt_pet.set_allocated_user(user_rqst);
                /* Send request to server */
                std::string messg;
                clnt_pet.SerializeToString(&messg);
                char msg[messg,size() + 1];
                strcpy(msg,messg.c_str());
                send(sockt,msg,strlen(msg),0);
                pthread_cond_wait(&thdcond,&locker);
                break;
            } 
            case 2: {  
                /* User information */
                printf("> Enter 'all' for all users information\n");
                printf("> Enter username for specific username\n");
                std::string usr_name;
                getline(std::cin,usr_name);
                /* Set request */
                chat::UserRequest* user_rqst(new chat::UserRequest);
                user_rqst->set_user(usr_name);
                chat::ClientRequest clnt_pet;
                clnt_pet.set_option(chat::ClientRequest_Option_USER_INFORMATION);
                clnt_pet.set_allocated_user(user_rqst);
                /* Send request to server */
                std::string messg;
                clnt_pet.SerializeToString(&messg);
                char msg[messg.size() + 1];
                strcpy(msg,messg.c_str());
                send(sockt,msg,strlen(msg),0);
                pthread_cond_wait(&thdcond,&locker);
                break;
            }
            case 3: {
                /* Change status */
                printf("1. See current status\n");
                printf("2. Change status\n");
                int st_opt;
                scanf("%d",&st_opt);
                switch(st_opt) {
                    case 1: {
                        /* Display current status */
                        printf("Current status: %s\n",status.c_str());
                        break;
                    }
                    case 2: {
                        /* Set new status */
                        printf("Enter new status: ");
                        std::string n_status;
                        getline(std::cin,n_status);
                        transform(n_status.begin(),n_status.end(),n_status.begin(),::toupper);
                        status = n_status;
                        /* Set request */
                        chat::ChangeStatus* sts_rqst(new chat::ChangeStatus);
                        sts_rqst->set_username(user_name);
                        sts_rqst->set_status(status);
                        chat::ClientRequest clnt_pet;
                        clnt_pet.set_option(chat::ClientRequest_Option_CHANGE_STATUS);
                        clnt_pet.set_allocated_status(sts_rqst);
                        /* Send request to server */
                        std::string messg;
                        clnt_pet.SerializeToString(&messg);
                        char msg[messg.size() + 1];
                        strcpy(msg,messg.c_str());
                        send(sockt,msg,strlen(msg),0);
                        pthread_cond_wait(&thdcond,&locker);
                        printf("> Status updated correctly\n");
                        break;
                    }
                    default: 
                        printf("> Invalid operation\n");
                        break;
                }
                break;
            }
            case 4: {
                /* General chat */
                for(chat::Message& message : gen_chat) {
                    /* Display General chat messages */
                    printf("@%s: %s\n",message.sender().c_str(),message.text().c_str());
                }
                printf("1. Send message\n");
                printf("2. Continue\n");
                int gmsg_opt;
                scanf("%d",&gmsg_opt);
                switch(gmsg_opt) {
                    case 1: {
                        /* Send message to general chat */
                        printf("Message: ");
                        std::string txt;
                        getline(std::cin,txt);
                        /* Set request */
                        chat::Message* msg_rqst(new chat::Message);
                        msg_rqst->set_text(txt);
                        msg_rqst->set_receiver("all");
                        msg_rqst->set_sender(user_name);
                        chat::ClientRequest clnt_pet;
                        clnt_pet.set_option(chat::ClientRequest_Option_SEND_MESSAGE);
                        clnt_pet.set_allocated_messg(msg_rqst);
                        /* Send request to server */
                        std::string messg;
                        clnt_pet.SerializeToString(&messg);
                        char msg[messg.size() + 1];
                        strcpy(msg,messg.c_str());
                        send(sockt,msg,strlen(msg),0);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case 5: {
                /* Private chat */
                printf("Enter username: ");
                std::string usr_name;
                getline(std::cin,usr_name);
                if(prv_chat.find(usr_name) != prv_chat.end()) {
                    std::vector<chat::Message> messsages = prv_chat[usr_name];
                    for(chat::Message& msg : messsages) {
                        printf("@%s: %s",msg.sender().c_str(),msg.text().c_str());
                    } 
                }
                printf("1. Send message to %s\n",usr_name.c_str());
                printf("2. Continue\n");
                int pmsg_opt;
                scanf("%d",&pmsg_opt);
                switch(pmsg_opt) {
                    case 1: {
                        /* Send message to specific user */
                        printf("Message: ");
                        std::string txt;
                        getline(std::cin,txt);
                        /* Set request */
                        chat::Message* msg_rqst(new chat::Message);
                        msg_rqst->set_text(txt);
                        msg_rqst->set_receiver(usr_name);
                        msg_rqst->set_sender(user_name);
                        chat::ClientRequest clnt_pet;
                        clnt_pet.set_option(chat::ClientRequest_Option_SEND_MESSAGE);
                        clnt_pet.set_allocated_messg(msg_rqst);
                        /* Set sender message */
                        chat::Message sndr_msg;
                        sndr_msg.set_text(txt);
                        sndr_msg.set_receiver(usr_name);
                        sndr_msg.set_sender(user_name);
                        if(prv_chat.find(sndr_msg.receiver()) == prv_chat.end()) {
                            std::vector<chat::Message> n_msg;
                            n_msg.push_back(sndr_msg);
                            prv_chat[sndr_msg.receiver()] = n_msg;
                        } else {
                            std::vector<chat::Message> messages = prv_chat[sndr_msg.receiver()];
                            messages.push_back(sndr_msg);
                            prv_chat[sndr_msg.receiver()] = messages;
                        }
                        /* Send request to server */
                        std::string messg;
                        clnt_pet.SerializeToString(&messg);
                        char msg[messg.size() + 1];
                        strcpy(msg,messg.c_str());
                        send(sockt,msg,strlen(msg),0);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case 6: {
                printf("> Closing session\n");
                lp_flag = 0;
                break;
            }
            default: {
                printf("> Invalid operation ...\n");
                break;
            }
        }
    }
    close(sockt);
    google::protobuf::ShutdownProtobufLibrary();
    return EXIT_SUCCESS;
}
