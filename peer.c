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
char ** searchFunction(int s);
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


char** searchFunction(int s){
        char ** sub_str = malloc(3 * sizeof(char*));
        printf("Enter a file name: ");
        char str[100];
        int flag=0;
        scanf("%s",str);
        int size = strlen(str) + 1;
        char buf[1200];
        buf[0] = 2;
        memcpy(buf+1, str, size);
        size = size + 1;
        buf[size+1] = '\0';
        if((sendall(s, buf, &size)) == -1)
        {
                printf("error\n");
                exit(1);
        }
        char response[10];
        recv(s, response, 10, 0);        
        uint32_t peerID;
        uint32_t ipAddr;
        uint16_t port;
        memcpy(&peerID, response, 4);
        memcpy(&ipAddr, response+4, 4);
        memcpy(&port, response+8, 2);
        peerID = ntohl(peerID);
        if(peerID ==0){
                printf("File not indexed by registry\n");
                flag++;
        } 
        if(flag==0){
                port = ntohs(port);
                char realIP[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ipAddr, realIP, sizeof(realIP));
                printf("File found at\n");
                printf(" Peer %u\n", peerID);
                printf(" %s:%u\n", realIP, port);
                char port_char[2];
                // port = ntohs(port);
                sprintf(port_char,"%u", port);

                sub_str[0] = str;
                sub_str[1] = realIP;
                sub_str[2] = port_char;
                
        }else{
             sub_str[0] = 0;   
        }
        return sub_str;
        
}


void actionFunction(int s, char* command, int peerID){
        if(strcmp(command,"JOIN") == 0){
                joinFunction(s, peerID);
        }
        else if(strcmp(command,"PUBLISH") == 0){
                publishFunction(s);
        }
        else if(strcmp(command,"SEARCH") == 0){
                // searchFunction(s);
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
        char ** rvalue = malloc(3 * sizeof(char*));
        rvalue = searchFunction(s);
        if(rvalue[0] ==0) exit(1);
        int _s;
        if ( ( _s = lookup_and_connect(rvalue[1], rvalue[2]) ) < 0 ) {
                exit( 1 );
        }
        // printf("Downloading...\n");
        int size = strlen(rvalue[0]) + 1;
        char buf[1200];
        buf[0]=0x03;
        memcpy(buf+1, rvalue[0], size);
        size = size + 1;
        buf[size+1] = '\0';
        if((send(_s, buf, size,0)) == -1)
        {
                printf("error\n");
                exit(1);
        }
        int total_len = 0;
        char response[10];
        FILE *file = NULL;
        file = fopen(rvalue[0], "ab");
        if(file == NULL){
                printf("File could not open");
        }else{
                // printf("File succesfully opened");      
        }
        int count =0;
        while(1)
        {
                if(count==0){
                        int flag = recv(_s, response, 2, 0);
                        // printf("flag is %d\n", flag);
                        if(flag==1) break;
                        else {
                                char content[1];
                                memcpy(&content, response+1, 1);
                                fwrite(content , 1 , 1, file);
                                total_len++;
                        }
                }
                int _rec = recv(_s, response, 10, 0);
                if( _rec < 0 ){
                        puts("recv failed");
                        break;
                }
                // if(_rec==1){break;}
                total_len += _rec;      
                fwrite(response , _rec , 1, file);
                // printf("Received byte size = %d\nTotal length = %d\n", _rec, total_len);

                if( _rec == 0 ){
                        break;
                }  
                count++;
                
        }
        // puts("\nReply received\n");
        // printf("Total length = %d\n",total_len);
        fclose(file);
 
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