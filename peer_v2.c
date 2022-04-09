//Maxwell Kuse
//Jesus Ramirez
//EECE 446 Spring 2022

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <dirent.h>
#define rep(i,a,b) for(int i=a; i<b;i++)

int lookup_and_connect( const char *host, const char *service );

int sendall(int s, char* buf, int* len);//***recvall may not be necessary since data being received isnt big enough to be seperaated into chunks like in program 1***

size_t recvall(int s, char* buf, size_t len);

void actionFunction(int s, char* command, int peerID);
void joinFunction(int s, int peerID);
void publishFunction(int s);
void searchFunction(int s);
void fetchFunction(int s);

int main(int argc, char *argv[]) {
        
        int s;
        const char *host = argv[1];
        const char *port = argv[2];
        int peerID = atoi(argv[3]);

        /* Lookup IP and connect to server */
        if ( ( s = lookup_and_connect( host, port ) ) < 0 ) {
                exit( 1 );
        }
        
        char command[8];//***character array big enough to fit the longest command AND null at end***
        printf("Enter a command: ");
        //while loop that asks for action on command line
        while(scanf("%s", command)){
                //if "EXIT" is typed the connection will be closed
                if(strcmp(command,"EXIT") == 0){
                        close(s);
                        return 0;
                }
                actionFunction(s, command, peerID);
                printf("Enter a command: ");
        }
}


void joinFunction(int s, int peerID)
{
  char buf[5];
  buf[0] = 0;
  //convert the peerID into network byte order
  peerID = htonl(peerID);
  memcpy(buf+1, &peerID, 4);//memcpy(DESTINATION, SOURCE, LENGTH)
  int size = sizeof(buf);
  if((sendall(s, buf, &size)) == -1)
  {
    printf("ERROR\n");
    exit(1);
  }
}

void publishFunction(int s){
        char buf[1200];  
        buf[0] = 1;
        int count = 0;
        int size = 0;
        DIR *d;
        struct dirent *dir;
        d = opendir("./SharedFiles");
        if(d){
                while ((dir = readdir(d)) != NULL){
                        if(dir->d_type==DT_REG){
                                count = count + 1;
                        }
                        else{
                                continue;
                        }
                }
                closedir(d);
        }
        char** files = malloc(count * sizeof(*files));
        int i = 0;
        d = opendir("./SharedFiles");
        if(d){
                while ((dir = readdir(d)) != NULL){
                        if(dir->d_type!=DT_DIR){
                                files[i] = malloc(sizeof(dir->d_name));
                                memcpy(files[i], dir->d_name, strlen(dir->d_name) + 1);
                                i++;
                        }
                        else{
                                continue;
                        }
                }
                count = htonl(count);
                memcpy(buf+1, &count, 4);
                memcpy(buf+5, files[0], strlen(files[0]) + 1);
                size = 5 + strlen(files[0]) + 1;
                for(int j = 1; j < i; j++){
                        if(files[j]){
                                memcpy(buf+size, files[j], strlen(files[j]) + 1); 
                                size = size + strlen(files[j]) + 1;
                        }
                        
                }
                if((sendall(s, buf, &size)) == -1){
                        printf("error\n");
                        exit(1);
                }
                closedir(d);
        }
}

void searchFunction(int s){
        printf("Enter a file name: ");
        char str[100];
        int flag=0;
        scanf("%s",str);
        char search_request[1200];
        char buf[10];
        // char s[1200];
        search_request[0] = 0x02;
        int len = strlen(str);
        rep(i,1,len+1){
              search_request[i] = str[i-1];
        } 
        search_request[len+1] = 0x00;
        int bytes_sent = send(s, search_request, len+2, 0);
        if (bytes_sent == -1)
        {
                perror("[<Error> Client] onSend:\n");
                exit(1);
        }
        int _len =0;
        _len = recv(s, buf, 10, 0 );
        if(_len <0){exit(1);}
        // unsigned int result =0x00;
        char peer[100];
        char port[100];
        long peer_n =0;
        // result[0] ='x';
        for (int i = 0; i < 10; i++)
        {
        char str[6];
        unsigned int temp = buf[i] & 0xFF;         
        if(i<4){
                // printf("%d", temp); 
                snprintf(str, sizeof(str), "%02x", temp);
                strcat(peer, str);
                
        }   
        else if (i==4){
                peer_n = strtol(peer, NULL, 16);
                if(peer_n !=0){
                        printf("File found at\n Peer %ld\n", peer_n);
                        peer_n =0;
                }else {
                        printf("File not indexed by registry\n");
                        flag =1;
                        break;
                }
                //  printf("%ld", n);
                // printf(" %d.", temp); 
        }
        }  
        if(flag !=1){
                char realIP[INET_ADDRSTRLEN];
                uint32_t ipAddr;
                uint16_t port;
                char port_char[2];
                memcpy(&ipAddr, buf+4, 4);
                memcpy(&port, buf+8,2);
                port = ntohs(port);
                inet_ntop(AF_INET, &ipAddr, realIP, sizeof(realIP));
                sprintf(port_char,"%u", port);
                printf(" %s:%s\n", realIP, port_char);
        }  
        memset(port, 0, 100);
        memset(peer, 0, 100); 
}

void actionFunction(int s, char* command, int peerID){
        if(strcmp(command,"JOIN") == 0){
                joinFunction(s, peerID);
        }
        else if(strcmp(command,"PUBLISH") == 0){
                publishFunction(s);
        }
        else if(strcmp(command,"SEARCH") == 0){
                searchFunction(s);
        }
        else if (strcmp(command, "FETCH")==0){
                fetchFunction(s);
        }
        else{
                printf("Invalid Action\n");
        }
}

void fetchFunction(int s)
{
        printf("Enter a file name: ");
        char str[100];
        int flag=0;
        scanf("%s",str);
        char search_request[1200];
        char buf[10];
        // char s[1200];
        search_request[0] = 0x02;
        int len = strlen(str);
        rep(i,1,len+1){
              search_request[i] = str[i-1];
        } 
        search_request[len+1] = 0x00;
        int bytes_sent = send(s, search_request, len+2, 0);
        if (bytes_sent == -1)
        {
                perror("[<Error> Client] onSend:\n");
                exit(1);
        }
        int _len =0;
        _len = recv(s, buf, 10, 0 );
        if(_len <0){exit(1);}
        // unsigned int result =0x00;
        char peer[100];
        char port[100];
        long peer_n =0;
        // result[0] ='x';
        for (int i = 0; i < 10; i++)
        {
        char str[6];
        unsigned int temp = buf[i] & 0xFF;         
        if(i<4){
                // printf("%d", temp); 
                snprintf(str, sizeof(str), "%02x", temp);
                strcat(peer, str);
                
        }   
        else if (i==4){
                peer_n = strtol(peer, NULL, 16);
                if(peer_n !=0){
                        printf("File found at\n Peer %ld\n", peer_n);
                        peer_n =0;
                }else {
                        printf("File not indexed by registry\n");
                        flag =1;
                        break;
                }
                //  printf("%ld", n);
                // printf(" %d.", temp); 
        }
        }  
        if(flag !=1){
        char realIP[INET_ADDRSTRLEN];
        uint32_t ipAddr;
        uint16_t port;
        char port_char[2];
        memcpy(&ipAddr, buf+4, 4);
        memcpy(&port, buf+8,2);
        port = ntohs(port);
        inet_ntop(AF_INET, &ipAddr, realIP, sizeof(realIP));
        sprintf(port_char,"%u", port);
        printf(" %s:%s\n", realIP, port_char);

        int _s;
        if ( ( _s = lookup_and_connect(realIP, port_char ) ) < 0 ) {
                exit( 1 );
        }
        // printf("Downloading...\n");
        search_request[0]=0x03;
        if((send(_s, search_request, len+2,0)) == -1)
        {
                printf("error\n");
                exit(1);
        }
        int total_len = 0;
        char response[10];
        FILE *file = NULL;
        file = fopen(str, "ab");
        if(file == NULL){
                printf("File could not opened");
        }else{
                // printf("File succesfully opened");      
        }
        while(1)
        {
                int _rec = recv(_s, response, 10, 0);
                if( _rec < 0 ){
                        puts("recv failed");
                        break;
                }
                total_len += _rec;
                fwrite(response , _rec , 1, file);
                // printf("\nReceived byte size = %d\nTotal lenght = %d", _rec, total_len);

                if( _rec == 0 ){
                        break;
                }  
                
        }
        // puts("\nReply received\n");

        fclose(file);
        }
        
        memset(port, 0, 100);
        memset(peer, 0, 100);
 
}
// Function to deal with partial send --- from Beej's guide on Partial send()/recv()
int sendall(int s, char* buf, int* len)
{
        int total = 0;        // how many bytes we've sent
        int bytesleft = *len; // how many we have left to send
        int n;

        while (total < *len) {
                n = send(s, buf + total, bytesleft, 0);
                if (n == -1) { break;}
                total = total + n;
                bytesleft = bytesleft - n;
        }

        *len = total; // return number actually sent here

        return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

// Function to deal with partial receive--- similar to sendall function from Beej's guide
size_t recvall(int s, char* buf, size_t len)
{
    int total = 0;        // how many bytes we've sent
    size_t bytesleft = len; // how many bytes we have left to send
    int n;

    while ((n = recv(s, buf + total, bytesleft, 0)) > 0) { // While still data to be received call recv()
        if (n <= 0) { break; } // Less than or equal to 0 we have no more bytes to receive
        total += n;
        bytesleft -= n;
    }

    return total;
}
// Use size_t data type to represent size of object in terms of bytes ******important for recv()


int lookup_and_connect( const char *host, const char *service ) {
        struct addrinfo hints;
        struct addrinfo *rp, *result;
        int s;

        /* Translate host name into peer's IP address */
        memset( &hints, 0, sizeof( hints ) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;

        if ( ( s = getaddrinfo( host, service, &hints, &result ) ) != 0 ) {
                fprintf( stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror( s ) );
                return -1;
        }

        /* Iterate through the address list and try to connect */
        for ( rp = result; rp != NULL; rp = rp->ai_next ) {
                if ( ( s = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol ) ) == -1 ) {
                        continue;
                }

                if ( connect( s, rp->ai_addr, rp->ai_addrlen ) != -1 ) {
                        break;
                }

                close( s );
        }
        if ( rp == NULL ) {
                perror( "stream-talk-client: connect" );
                return -1;
        }
        freeaddrinfo( result );

        return s;
}