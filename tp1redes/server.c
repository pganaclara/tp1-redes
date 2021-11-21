#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 500

char pokedex[40][11];

char* formatString(char string[11]){
    for(int i = 0; i < 11; i++){
        if(string[i] == '\0' || string[i] == '\n'){
            string[i] = '\0';
        }
    }
    return string;
}

void clearSpaceInFinal(char string[BUFSZ]){
    int i;
    for (i=0;string[i] != '\0';i++){
        i=i;
    }
    if (string[i-1] == ' ')
        string[i-1] = '\0';
}

int existsPokemon (char pokemon[11]){
    for(int i = 0; i < 40; i++){
        if (strcmp(pokemon, pokedex[i]) == 0){
            return i;
        }
    }
    return -1;
}

int fullPokedex(char buf[], char pokemon[11]){
    for (int i = 0; i < 40; i++) {
        if (strcmp(pokedex[i],"") == 0)
            return 0;
    }
    return 1;
}

void clearPokedex (){
    for(int i = 0; i < 40; i++){
        strcpy(pokedex[i], "");
    }
}

int findPlacePokedex (){
    for (int i = 0; i < 40; i++) {
       if (strcmp(pokedex[i],"") == 0){
           return i;
       } 
    } 
    return -1;
}

void handleAddPokemon(char buf[], char pokemon[5][11], int numPokemon){
    char message[500] = "";
    
    for(int i = 0; i < numPokemon; i++){
        if(fullPokedex(buf, pokemon[i]) == 0){
            if (existsPokemon(formatString(pokemon[i])) != -1){
                strcat(message, pokemon[i]);
                strcat(message, " already exists");
            } else {
                strcat(message, pokemon[i]);
                strcat(message, " added ");
                strcpy(pokedex[findPlacePokedex()],pokemon[i]);
            }
        } else {
            strcat(message, "limit exceeded");
        }       
    }
    clearSpaceInFinal(message);
    sprintf(buf,"%s\n", message);
}

void handleRemovePokemon(char buf[], char pokemon[11]){
    formatString(pokemon);
    if (existsPokemon(pokemon) != -1){
        strcpy(pokedex[existsPokemon(pokemon)], "");
        sprintf(buf, "%s removed\n", pokemon);
    }else{
        sprintf(buf, "%s does not exist\n", pokemon);
    }
}

void handleListPokemon(char buf[]){
    char message[500] = "";
    int cont = 0;
    for(int i = 0; i < 40; i++){
        if(strcmp(pokedex[i],"") != 0){
            strcat(message, pokedex[i]);
            strcat(message, " ");
            cont++;
        }
    }
    if (cont == 0)
        strcat(message, "none");
    clearSpaceInFinal(message);
    sprintf(buf, "%s\n", message);
}

void handleExchangePokemon(char buf[], char pokemon1[11], char pokemon2[11]){
    formatString(pokemon2);
    int index = existsPokemon(pokemon1), index2 = existsPokemon(pokemon2);
    if (index == -1){
        sprintf(buf, "%s does not exist\n", pokemon1);
    }else if (index2 > -1){
        sprintf(buf, "%s already exists\n", pokemon2);
    }else{
        strcpy(pokedex[index], pokemon2);
        sprintf(buf, "%s exchanged\n", pokemon1);
    }
}

void handle(char buf[]){
    char msg[BUFSZ];
    strcpy(msg, buf); //creates a copy of the buffer for manipulation
    memset(buf,0, BUFSZ); //clears buffer

    //string declaration for each command known:
    const char add[] = "add"; 
    const char remove[] = "remove"; 
    const char exchange[] = "exchange"; 
    const char list[] = "list\n";
    
    // Extract the first token
   char space[] = " ";
   char pokemon[5][11];
   char *pokemonTemp[5];
   int totalPokemon = 0;
   pokemonTemp[0] = strtok(msg, space);
   // loop through the string to extract all other tokens
    for(totalPokemon = 1; pokemonTemp[totalPokemon-1] != NULL ; totalPokemon++) {
      pokemonTemp[totalPokemon] = strtok(NULL, space);
      if (pokemonTemp[totalPokemon] != NULL){
        strcpy(pokemon[totalPokemon-1], pokemonTemp[totalPokemon]);
        for (int i = 0;pokemon[totalPokemon-1][i] != '\0' && pokemon[totalPokemon-1][i] != '\n';i++){
            if (pokemon[totalPokemon-1][i] < 97 || pokemon[totalPokemon-1][i] > 122){
                if (pokemon[totalPokemon-1][i] < 48 || pokemon[totalPokemon-1][i] > 57){
                    sprintf(buf, "invalid message\n");
                    return;
                }
            }
        }
      }
   }
   totalPokemon = totalPokemon - 2;
   if (strcmp(msg,add) == 0){
       handleAddPokemon(buf,pokemon,totalPokemon);
   } else if(strcmp(msg, remove) == 0){
       handleRemovePokemon(buf, pokemon[0]);
   } else if(strcmp(msg, list) == 0){
       handleListPokemon(buf);
   } else if(strcmp(msg, exchange) == 0){
       handleExchangePokemon(buf, pokemon[0], pokemon[1]);
   } else{
       exit(EXIT_SUCCESS);  
   }
}

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    clearPokedex();

    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        size_t count;

        while(1){
            memset(buf, 0, BUFSZ);  
            count = 0;

            while (buf[strlen(buf) - 1] != '\n') {
                count += recv(csock, buf + count, BUFSZ - count, 0);
            }

            printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);    

            handle(buf);
            count = send(csock, buf, strlen(buf), 0);
            if (count != strlen(buf)) {
                logexit("send");
            }
        }
        close(csock);   
    }

    exit(EXIT_SUCCESS);
}
