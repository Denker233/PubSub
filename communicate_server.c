/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "communicate.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_CLIENTS 100
#define MAX_SUBSCRIPTIONS 10
#define MAXSTRING 120

struct ClientInfo
{
    char IP[16];
    int Port;
    int numSubscriptions;                             // track the number of subscriptions for the client
    char subscriptions[MAX_SUBSCRIPTIONS][MAXSTRING]; // array of subscriptions for the client
};

struct ClientInfo clients[MAX_CLIENTS];
int numClients = 0;

/* send UDP message to the specified client */
void send_message(char *IP, int Port, char *message)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    char port_str[6];
    sprintf(port_str, "%d", Port);

    if ((rv = getaddrinfo(IP, port_str, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }

    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "talker: failed to create socket\n");
        return;
    }

    if ((numbytes = sendto(sockfd, message, strlen(message), 0,
                           p->ai_addr, p->ai_addrlen)) == -1)
    {
        perror("talker: sendto");
        exit(1);
    }

    freeaddrinfo(servinfo);

    printf("talker: sent %d bytes to %s\n", numbytes, IP);
    close(sockfd);
}

/* Check if the client has already joined */
bool_t clientJoined(char *IP, int Port)
{
    for (int i = 0; i < numClients; i++)
    {
        if (strcmp(clients[i].IP, IP) == 0 && clients[i].Port == Port)
        {
            return 1;
        }
    }
    return 0;
}

/* Parse an article string into fields */
int split(const char *str, char ***arr)
{
    int count = 1; // number of tokens
    int token_len = 1;
    int i = 0;
    char *p;
    char *t;

    // first counts the number of substrings that will be produced
    p = str;
    while (*p != '\0')
    {
        if (*p == ';')
            count++;
        p++;
    }

    // then dynamically allocates an array of pointers to characters that will hold each of the substrings.
    *arr = (char **)calloc(count, sizeof(char *));
    if (*arr == NULL)
        exit(1);

    p = str;
    while (*p != '\0')
    {
        if (*p == ';' && *(p + 1) == '\0') {
            // If semicolon is followed by null terminator, skip creating a new substring
            break;
        }
        if (*p == ';')
        {
            (*arr)[i] = (char *)malloc(sizeof(char) * token_len);
            if ((*arr)[i] == NULL)
                exit(1);

            token_len = 0;
            i++;
        }
        p++;
        token_len++;
    }
    (*arr)[i] = (char *)malloc(sizeof(char) * token_len);
    if ((*arr)[i] == NULL)
        exit(1);

    // copies the characters from the original string into the new substring.
    i = 0;
    p = str;
    t = ((*arr)[i]);
    while (*p != '\0')
    {
        if (*p != ';' && *p != '\0')
        {
            *t = *p;
            t++;
        }
        else
        {
            *t = '\0';
            i++;
            t = ((*arr)[i]);
        }
        p++;
    }

    // return the number of tokens
    return count;
}

bool_t isArticleValid(char *article, int flag) //flag = 1, check for subscription; otherwise, check for publication
{
    char ** fields;
    fields = NULL;
    int count = split(article,&fields); 

    if (count != 4 || strlen(article) > MAXSTRING)
    {
        // Invalid article string
        return 0;
    }

    int j;
    int zeros_count = 0;
    for (j = 0; j < count; j++){
        //printf("string #%d: %s\n", j, fields[j]);
        if(j==0){
            if(strlen(fields[0]) == 0){
                zeros_count++;
            }else{
                    if (strcmp(fields[0], "Sports") != 0 &&
                    strcmp(fields[0], "Lifestyle") != 0 &&
                    strcmp(fields[0], "Entertainment") != 0 &&
                    strcmp(fields[0], "Business") != 0 &&
                    strcmp(fields[0], "Technology") != 0 &&
                    strcmp(fields[0], "Science") != 0 &&
                    strcmp(fields[0], "Politics") != 0 &&
                    strcmp(fields[0], "Health") != 0)
                    {
                        // Invalid type field
                        printf("Invalid Article type; ");
                        return 0;
                    }

            }
        }else if(j==1){
            if(strlen(fields[1]) == 0){
                zeros_count++;
            }
        }else if(j==2){
            if(strlen(fields[2]) == 0){
                zeros_count++;
            }
        }else if(j==3){
            if(flag){
                if(fields[3] != NULL){
                    printf("Contents must be empty; ");
                    return 0;
                }
            }else{
                if(fields[3] == NULL){
                    printf("No contents; ");
                    return 0;
                }
            }
        }
    }

    if(zeros_count == 3){
        printf("None of the first 3 fields present; ");
        return 0;
    }
    // Free the memory allocated by split
    for (j = 0; j < count; j++) {
        free(fields[j]);
    }
    free(fields);

    return 1;
}

/* Check if the article is already subscribed */
bool_t alreadySubscribed(int clientIndex, char *Article)
{
    for (int i = 0; i < clients[clientIndex].numSubscriptions; i++)
    {
        char subscribedType[MAXSTRING], subscribedOriginator[MAXSTRING], subscribedOrg[MAXSTRING];
        sscanf(clients[clientIndex].subscriptions[i], "%[^;];%[^;];%[^;];", subscribedType, subscribedOriginator, subscribedOrg);
        char articleType[MAXSTRING], articleOriginator[MAXSTRING], articleOrg[MAXSTRING];
        sscanf(Article, "%[^;];%[^;];%[^;];", articleType, articleOriginator, articleOrg);
        if (strcmp(subscribedType, articleType) == 0 && strcmp(subscribedOriginator, articleOriginator) == 0 && strcmp(subscribedOrg, articleOrg) == 0)
        {
            return 1;
        }
    }

    return 0;
}

/* Subscribe the article */
bool_t subscribeArticle(int clientIndex, char *Article)
{
    if (clients[clientIndex].numSubscriptions < MAX_SUBSCRIPTIONS)
    {
        strcpy(clients[clientIndex].subscriptions[clients[clientIndex].numSubscriptions], Article);
        clients[clientIndex].numSubscriptions++;
        printf("Client subscribed to article '%s' successfully\n", Article);
        return 1;
    }
    else
    {
        return 0;
    }
}

bool_t *
join_1_svc(char *IP, int Port, struct svc_req *rqstp)
{
    static bool_t result;

    /* Check if the client has already joined */
    for (int i = 0; i < numClients; i++)
    {
        if (strcmp(clients[i].IP, IP) == 0 && clients[i].Port == Port)
        {
            result = 0;
            printf("Join Failed, Client with IP: %s and Port: %d has already joined the server\n", IP, Port);
            return &result;
        }
    }

    /* Add the new client to the list of clients */
    if (numClients < MAX_CLIENTS)
    {
        strcpy(clients[numClients].IP, IP);
        clients[numClients].Port = Port;
        numClients++;
        result = 1;
    }
    else
    {
        result = 0;
    }

    // send_message(IP, Port, "hello wulala"); //test send_message in join

    printf("Client joined with IP: %s and Port: %d\n", IP, Port);

    return &result;
}

bool_t *
leave_1_svc(char *IP, int Port, struct svc_req *rqstp)
{
    static bool_t result;

    /* Find the client in the list of clients */
    int clientIndex = -1;
    for (int i = 0; i < numClients; i++)
    {
        if (strcmp(clients[i].IP, IP) == 0 && clients[i].Port == Port)
        {
            clientIndex = i;
            break;
        }
    }

    if (clientIndex == -1)
    {
        result = 0;
        printf("Leave Failed, Client with IP: %s and Port: %d has not joined the server\n", IP, Port);
        return &result;
    }

    /* Remove the client from the list of clients */
    for (int i = clientIndex; i < numClients - 1; i++)
    {
        clients[i] = clients[i + 1];
    }
    numClients--;

    result = 1;
    printf("Client with IP: %s and Port: %d has left the server\n", IP, Port);

    return &result;
}

bool_t *subscribe_1_svc(char *IP, int Port, char *Article, struct svc_req *rqstp)
{
    static bool_t result;

    if (!clientJoined(IP, Port))
    {
        result = 0;
        printf("Subscribe Failed, Client with IP: %s and Port: %d has not joined the server\n", IP, Port);
        return &result;
    }

    int clientIndex = -1;
    for (int i = 0; i < numClients; i++)
    {
        if (strcmp(clients[i].IP, IP) == 0 && clients[i].Port == Port)
        {
            clientIndex = i;
            break;
        }
    }
     
    int flag = 1;
    if (!isArticleValid(Article, flag))
    {
        result = 0;
        printf("Subscribe Failed, Article '%s' is not valid for subscription.\n", Article);
        return &result;
    }

    if (alreadySubscribed(clientIndex, Article))
    {
        result = 0;
        printf("Subscribe Failed, Article '%s' is already subscribed by client with IP: %s and Port: %d\n", Article, IP, Port);
        return &result;
    }

    if (subscribeArticle(clientIndex, Article))
    {
        result = 1;
    }
    else
    {
        result = 0;
    }

    return &result;
}

bool_t *
unsubscribe_1_svc(char *IP, int Port, char *Article, struct svc_req *rqstp)
{
    static bool_t result;

    if (!clientJoined(IP, Port))
    {
        result = 0;
        printf("Unsubscribe Failed, Client with IP: %s and Port: %d has not joined the server\n", IP, Port);
        return &result;
    }

    int flag = 1;
    if (!isArticleValid(Article, flag))
    {
        result = 0;
        printf("Unsubscribe Failed, Article '%s' is not valid for subscription.\n", Article);
        return &result;
    }

    int clientIndex = -1;
    for (int i = 0; i < numClients; i++)
    {
        if (strcmp(clients[i].IP, IP) == 0 && clients[i].Port == Port)
        {
            clientIndex = i;
            break;
        }
    }

    char ** fields;
    fields = NULL;
    int count = split(Article,&fields); 

    bool_t unsubscribed = 0;
    for (int j = 0; j < clients[clientIndex].numSubscriptions; j++)
    {

            char **subFields;
            subFields = NULL;
            int subCount = split(clients[clientIndex].subscriptions[j],&subFields);

            // Check if the client has subscribed to the article being published
            if (strcmp(fields[0], subFields[0]) == 0 &&
                (strcmp(subFields[1], fields[1]) == 0) &&
                (strcmp(subFields[2], fields[2]) == 0))
            {
            // Remove the subscription
                for (int k = j; k < clients[clientIndex].numSubscriptions - 1; k++)
                {
                    strcpy(clients[clientIndex].subscriptions[k], clients[clientIndex].subscriptions[k + 1]);
                }
                clients[clientIndex].numSubscriptions--;
                unsubscribed = 1;
                break;
            }
    }

    if (unsubscribed)
    {
        result = 1;
        printf("Client unsubscribed to article '%s' successfully\n", Article);
    }
    else
    {
        result = 0;
        printf("Unsubscribe Failed, Client with IP: %s and Port: %d has not subscribed to article '%s'\n", IP, Port, Article);
    }

    return &result;
}

bool_t *
publish_1_svc(char *Article, char *IP, int Port, struct svc_req *rqstp)
{
    static bool_t result;

    // Check if the article is valid for publishing
    int flag = 0;
    if (!isArticleValid(Article, flag))
    {
        result = 0;
        printf("Publish Failed, Article '%s' is not valid for publishing.\n", Article);
        return &result;
    }

    char ** fields;
    fields = NULL;
    int count = split(Article,&fields); 

    // Find all clients who have subscribed to the publishing article
    for (int i = 0; i < numClients; i++)
    {
        if (clients[i].numSubscriptions == 0)
        {
            // Skip clients without any subscriptions
            continue;
        }

        for (int j = 0; j < clients[i].numSubscriptions; j++)
        {
            char **subFields;
            subFields = NULL;
            int subCount = split(clients[i].subscriptions[j],&subFields);

            // Check if the client has subscribed to the article being published
            if (strcmp(fields[0], subFields[0]) == 0 &&
                (subFields[1][0] == '\0' || strcmp(subFields[1], fields[1]) == 0) &&
                (subFields[2][0] == '\0' || strcmp(subFields[2], fields[2]) == 0))
            {
                // Send the matching article to the client
                printf("Article : %s matched", Article);
                send_message(clients[i].IP, clients[i].Port, Article);
                break;
            }
        }
    }

    result = 1;
    printf("Article Published: %s\n", Article);

    return &result;
}

bool_t *
ping_1_svc(struct svc_req *rqstp)
{
    static bool_t result;

    /*
     * insert server code heres
     */
    return &result;
}
