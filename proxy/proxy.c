#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/socket.h>
#include  <netdb.h>
#include  <string.h>
#include  <unistd.h>
#include  <stdbool.h>
#include "./simpleSocketAPI.h"

#define SERVADDR "127.0.0.1"        // Définition de l'adresse IP d'écoute
#define SERVPORT "0"                // Définition du port d'écoute, si 0 port choisi dynamiquement
#define LISTENLEN 1                 // Taille de la file des demandes de connexion
#define MAXBUFFERLEN 1024           // Taille du tampon pour les échanges de données
#define MAXHOSTLEN 64               // Taille d'un nom de machine
#define MAXPORTLEN 64               // Taille d'un numéro de port
#define PORTFTP "21"
#define MAXBUFFERDATALEN 8192

char* lireMessageServeur();
char* lireMessageClient();
void envoyerMessageServeur();
void envoyerMessageClient();

char* lireMessageServeur(int *socketServeurCMD){
    int ecode;
    char *buffer = (char *) calloc(MAXBUFFERLEN,sizeof(char));
    ecode = read(*socketServeurCMD,buffer,MAXBUFFERLEN-1);
    if (ecode == -1){perror("Erreur lecture du message envoyé par le serveur");exit(2);}
    buffer[ecode] = '\0';
    printf("Reçu Serveur : %s\n",buffer);
    return buffer;
}

char* lireData(int *socketData){
    int ecode;
    char *buffer = (char *) calloc(MAXBUFFERLEN,sizeof(char));
    char *bufferData = (char *) calloc(MAXBUFFERDATALEN,sizeof(char));
    ecode = read(*socketData,bufferData,MAXBUFFERLEN-1);
    strcat(bufferData,buffer);
    memset(buffer,0,MAXBUFFERLEN);
    while (ecode != 0){
        ecode = read(*socketData,buffer,MAXBUFFERLEN-1);
        strcat(bufferData,buffer);
        memset(buffer,0,MAXBUFFERLEN);
    }
    if (ecode == -1){perror("Erreur lecture du message envoyé par le serveur");exit(2);}
    return bufferData;
}

char* lireMessageClient(int *descSockCOM){
    int ecode;
    char *buffer = (char *) calloc(MAXBUFFERLEN,sizeof(char));
    ecode = read(*descSockCOM,buffer,MAXBUFFERLEN-1);
    if (ecode == -1){perror("Erreur lecture du message envoyé par le client");exit(2);}
    buffer[ecode] = '\0';
    printf("Reçu Client : %s\n",buffer);
    return buffer;
 }

void envoyerMessageServeur(int socketServeurCMD, char *buffer){
    int ecode;
    ecode = write(socketServeurCMD,buffer,strlen(buffer));
    if (ecode == -1){perror("Erreur envoi du message vers le serveur");exit(2);}
}

void envoyerMessageClient(int descSockCOM, char *buffer){
    int ecode;
    ecode = write(descSockCOM,buffer,strlen(buffer));
    if (ecode == -1){perror("Erreur envoi du message vers le client");exit(2);}
}


int main(){
    int ecode;                       // Code retour des fonctions
    char serverAddr[MAXHOSTLEN];     // Adresse du serveur
    char serverPort[MAXPORTLEN];     // Port du server
    int descSockRDV;                 // Descripteur de socket de rendez-vous
    int descSockCOM;                 // Descripteur de socket de communication
    struct addrinfo hints;           // Contrôle la fonction getaddrinfo
    struct addrinfo *res;            // Contient le résultat de la fonction getaddrinfo
    struct sockaddr_storage myinfo;  // Informations sur la connexion de RDV
    struct sockaddr_storage from;    // Informations sur le client connecté
    socklen_t len;                   // Variable utilisée pour stocker les 
				                     // longueurs des structures de socket
    char buffer[MAXBUFFERLEN];       // Tampon de communication entre le client et le serveur
    char bufferData[MAXBUFFERDATALEN];

    // Initialisation de la socket de RDV IPv4/TCP
    descSockRDV = socket(AF_INET, SOCK_STREAM, 0);
    if (descSockRDV == -1) {
         perror("Erreur création socket RDV\n");
         exit(2);
    }
    // Publication de la socket au niveau du système
    // Assignation d'une adresse IP et un numéro de port
    // Mise à zéro de hints
    memset(&hints, 0, sizeof(hints));
    // Initialisation de hints
    hints.ai_flags = AI_PASSIVE;      // mode serveur, nous allons utiliser la fonction bind
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_family = AF_INET;        // seules les adresses IPv4 seront présentées par 
				                      // la fonction getaddrinfo

     // Récupération des informations du serveur
     ecode = getaddrinfo(SERVADDR, SERVPORT, &hints, &res);
     if (ecode) {
         fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
         exit(1);
     }
     // Publication de la socket
     ecode = bind(descSockRDV, res->ai_addr, res->ai_addrlen);
     if (ecode == -1) {
         perror("Erreur liaison de la socket de RDV");
         exit(3);
     }
    // Nous n'avons plus besoin de cette liste chainée addrinfo
    freeaddrinfo(res);

     // Récuppération du nom de la machine et du numéro de port pour affichage à l'écran
     len=sizeof(struct sockaddr_storage);
     ecode=getsockname(descSockRDV, (struct sockaddr *) &myinfo, &len);
     if (ecode == -1)
     {
         perror("SERVEUR: getsockname");
         exit(4);
     }
     ecode = getnameinfo((struct sockaddr*)&myinfo, sizeof(myinfo), serverAddr,MAXHOSTLEN, 
                         serverPort, MAXPORTLEN, NI_NUMERICHOST | NI_NUMERICSERV);
     if (ecode != 0) {
             fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(ecode));
             exit(4);
     }
     printf("L'adresse d'ecoute est: %s\n", serverAddr);
     printf("Le port d'ecoute est: %s\n", serverPort);

     // Definition de la taille du tampon contenant les demandes de connexion
     ecode = listen(descSockRDV, LISTENLEN);
     if (ecode == -1) {
         perror("Erreur initialisation buffer d'écoute");
         exit(5);
     }

	len = sizeof(struct sockaddr_storage);
     // Attente connexion du client
     // Lorsque demande de connexion, creation d'une socket de communication avec le client
    while (1){
        descSockCOM = accept(descSockRDV, (struct sockaddr *) &from, &len);
        if (descSockCOM == -1){
            perror("Erreur accept\n");
            exit(6);
        }
        pid_t idProc = fork();
        if (idProc == 0) {
            // Echange de données avec le client connecté

            strcpy(buffer, "220 Bienvenue sur le proxy\n");
            write(descSockCOM, buffer, strlen(buffer));
            
            ecode = read(descSockCOM,buffer,MAXBUFFERLEN-1);
            if (ecode == -1 ){perror("pb lecture");exit(2);}
            buffer[ecode]= '\0';
            printf("Reçu du client : %s\n",buffer);

            char login[30];
            sscanf(buffer,"%[^@]@%s",login,serverAddr);
            printf("Login : %s\nServeur : %s\n",login,serverAddr);

            int socketServeurCMD;
            ecode=connect2Server(serverAddr,PORTFTP,&socketServeurCMD); 

            memset(buffer,0,MAXBUFFERLEN);
            strcpy(buffer,lireMessageServeur(&socketServeurCMD)); // 220 (Bienvenue Serveur) S -> P
            
            memset(buffer,0,MAXBUFFERLEN);
            sprintf(buffer,"%s\r\n",login);
            envoyerMessageServeur(socketServeurCMD,buffer); // USER login P -> S

            memset(buffer,0,MAXBUFFERLEN);
            strcpy(buffer,lireMessageServeur(&socketServeurCMD)); // 331 (Login Ok) S -> P

            envoyerMessageClient(descSockCOM,buffer); // 331 P -> C

            memset(buffer,0,MAXBUFFERLEN);
            strcpy(buffer,lireMessageClient(&descSockCOM)); // PASS C -> P

            envoyerMessageServeur(socketServeurCMD, buffer); // PASS P -> S

            memset(buffer,0,MAXBUFFERLEN);
            strcpy(buffer,lireMessageServeur(&socketServeurCMD)); // 230 (Acces granted) S -> P

            envoyerMessageClient(descSockCOM,buffer); // 230 P -> C

            memset(buffer,0,MAXBUFFERLEN);
            strcpy(buffer,lireMessageClient(&descSockCOM)); // SYST C -> P

            envoyerMessageServeur(socketServeurCMD,buffer); // SYST P -> S
            
            memset(buffer,0,MAXBUFFERLEN);
            strcpy(buffer,lireMessageServeur(&socketServeurCMD)); // 215 S ->  P

            envoyerMessageClient(descSockCOM,buffer); // 215 P -> C

            while (1) {
                memset(buffer,0,MAXBUFFERLEN);
                strcpy(buffer,lireMessageClient(&descSockCOM));
                printf("Requête client : %s", buffer);

                //Gestion du LS
                if (strstr(buffer,"PORT") != NULL){
                    printf("Test ls\n");
                    char adresseIp[16];
                    char ip1[4], ip2[4],ip3[4],ip4[5];
                    char firstPort[4], secondPort[4];
                    sscanf(buffer, "PORT %[^,],%[^,],%[^,],%[^,],%[^,],%s", ip1, ip2, ip3, ip4, firstPort, secondPort);
                    int portTemp = atol(firstPort) * 256 + atol(secondPort);
                    char port[6];
                    sprintf(port,"%d",portTemp);
                    sprintf(adresseIp,"%s.%s.%s.%s", ip1,ip2,ip3,ip4);
                    printf("Adresse Ip : %s\n",adresseIp);
                    printf("N° de port : %s\n",port);
                    
                    int socketDataClient;
                    connect2Server(adresseIp,port,&socketDataClient);

                    memset(buffer,0,MAXBUFFERLEN);
                    strcpy(buffer,"PASV\r\n");
                    envoyerMessageServeur(socketServeurCMD,buffer);

                    memset(buffer,0,MAXBUFFERLEN);
                    strcpy(buffer,lireMessageServeur(&socketServeurCMD));

                    sscanf(buffer, "227 Entering Passive Mode (%[^,],%[^,],%[^,],%[^,],%[^,],%[^)].",ip1,ip2,ip3,ip4, firstPort, secondPort);
                    portTemp = atol(firstPort) * 256 + atol(secondPort);
                    sprintf(port,"%d",portTemp);
                    sprintf(adresseIp,"%s.%s.%s.%s",ip1,ip2,ip3,ip4);
                    
                    int socketDataServeur;
                    connect2Server(adresseIp,port,&socketDataServeur);

                    memset(buffer,0,MAXBUFFERLEN);
                    strcpy(buffer,"220 Port Command Succesful\n");
                    envoyerMessageClient(descSockCOM,buffer);

                    memset(buffer,0,MAXBUFFERLEN);
                    strcpy(buffer,lireMessageClient(&descSockCOM)); // LIST C -> P
                    envoyerMessageServeur(socketServeurCMD,buffer);

                    memset(buffer,0,MAXBUFFERLEN);
                    strcpy(buffer,lireMessageServeur(&socketServeurCMD)); // 150 (Opening ASCII)
                    envoyerMessageClient(descSockCOM,buffer);
                    printf("%s",buffer);

                    memset(bufferData,0,MAXBUFFERDATALEN);
                    strcpy(bufferData,lireData(&socketDataServeur)); // Lecture du resultat de LIST S -> P
                    close(socketDataServeur);
                    envoyerMessageClient(socketDataClient,bufferData);
                    close(socketDataClient);

                    memset(buffer,0,MAXBUFFERLEN);
                    strcpy(buffer,lireMessageServeur(&socketServeurCMD));
                    envoyerMessageClient(descSockCOM,buffer);

                } else {
                    envoyerMessageServeur(socketServeurCMD,buffer);
                    memset(buffer,0,MAXBUFFERLEN);
                    strcpy(buffer,lireMessageServeur(&socketServeurCMD));

                    envoyerMessageClient(descSockCOM,buffer);

                    char code[4];
                    strncpy(code, buffer, 3);
                    code[3] = '\0';
                    // Gestion de la commande close
                    if (strcmp(code,"221") == 0){
                        close(descSockCOM);
                        close(descSockRDV);
                        break;
                    }
                }
            }
        }
    }
}

