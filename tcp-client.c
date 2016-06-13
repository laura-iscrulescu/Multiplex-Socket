#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

/**
 * The entrance of the server application.
 * 
 * @param  argc the number of arguments
 * @param  argv a pointer to a char array that stores arguments
 * @return 0 if the application exited normally
 */
int main(int argc, char *argv[]) {
    if ( argc != 3 ) {
        fprintf(stderr," Usage: %s Host PortNumber\n",argv[0]);
        exit(1);
    }
    
    struct hostent* pHost = gethostbyname(argv[1]);
    if ( pHost == NULL ) {
        fprintf(stderr, "Usage: %s Host PortNumber\n", argv[0]);
        exit(1);
    }
    int portNumber = atoi(argv[2]);
    if ( portNumber <= 0 ) {
        fprintf(stderr, "Usage: %s Host PortNumber\n", argv[0]);
        exit(1);
    }

    /*
     * Create socket file descriptor.
     * Function Prototype: int socket(int domain, int type,int protocol)
     * Defined in sys/socket.h
     *
     * @param domain:   AF_INET stands for Internet, AF_UNIX can only communicate between UNIX systems.
     * @param type      the prototype to use, SOCK_STREAM stands for TCP and SOCK_DGRAM stands for UDP
     * @param protocol  if type is specified, this parameter can be assigned to 0.
     * @return -1 if socket is failed to create
     */
    int tcpSocketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if ( tcpSocketFileDescriptor == -1 ) {
        fprintf(stderr, "[ERROR] Failed to create socket: %s\n", strerror(errno));
        exit(127);
    }

    /*
     * Initialize sockaddr struct.
     *
     * The structure of sockaddr:
     * 
     * struct sockaddr{
     *     unisgned short  as_family;
     *     char            sa_data[14];
     * };
     *
     * To keep compatibility in different OS, sockaddr_in is used:
     *
     * struct sockaddr_in{
     *     unsigned short          sin_family;     // assigned to AF_INET generally
     *     unsigned short int      sin_port;       // the port number listens to
     *     struct in_addr          sin_addr;       // assigned to INADDR_ANY for communicating with any hosts
     *     unsigned char           sin_zero[8];    // stuffing bits
     * };
     *
     * Both of them are defined in netinet/in.h
     */
    int sockaddrSize = sizeof(struct sockaddr);
    struct sockaddr_in serverSocketAddress;
    bzero(&serverSocketAddress, sockaddrSize);
    serverSocketAddress.sin_family = AF_INET;
    serverSocketAddress.sin_addr=*((struct in_addr *)pHost->h_addr);
    serverSocketAddress.sin_port = htons(portNumber);

    /*
     * Connect to server.
     * Function prototype: int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
     * Defined in sys/socket.h and sys/types.h
     *
     * @param sockfd  the socket file descriptor
     * @param my_addr the specified address of server
     * @param addrlen the size of the struct sockaddr
     * @return -1 if the operation failed
     */
    if ( connect(tcpSocketFileDescriptor, (struct sockaddr *)(&serverSocketAddress), sizeof(struct sockaddr_in)) == -1 ) {
        fprintf(stderr, "[ERROR] Failed to connect to server: %s\n", strerror(errno));
        exit(127);
    }

    char inputBuffer[BUFFER_SIZE] = {0};
    char outputBuffer[BUFFER_SIZE] = {0};
    fprintf(stderr, "[INFO] Congratulations! Connection established with server.\nType \'BYE\' to disconnect.\n");
    do {
        fprintf(stderr, "> ");
        scanf("%s", outputBuffer, BUFFER_SIZE);

        // Send a message to client
        if ( send(tcpSocketFileDescriptor, outputBuffer, strlen(outputBuffer) + 1, 0) == -1 ) {
            fprintf(stderr, "[ERROR] An error occurred while sending message to the server: %s\nThe connection is going to close.\n", strerror(errno));
            break;
        }

        // Stop sending message to server
        if ( strcmp("BYE", outputBuffer) == 0 ) {
            break;
        }
        
        // Receive a message from client
        int readBytes = recv(tcpSocketFileDescriptor, inputBuffer, BUFFER_SIZE, 0);
        if ( readBytes < 0 ) {
            fprintf(stderr, "[ERROR] An error occurred while receiving message from the server: %s\nThe connection is going to close.\n", strerror(errno));
            break;
        }
        fprintf(stderr, "[INFO] Received a message from server: %s\n", inputBuffer);
    } while ( 1 );

    /*
     * Close socket for client.
     */
    close(tcpSocketFileDescriptor);

    return 0;
}