#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "file_utils.h"


// Use mTLS for connections to server from device so it can’t connect unless it has a trusted cert
// Your hypervisor should verify:

// X509_verify(cert, ca_pubkey);

// This will FAIL for self-signed device certs

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Port not provided. Program terminated\n");
        return ERROR;
    }

    struct sockaddr_in serv_addr, cli_addr;
    // Data type 32 bit
    socklen_t clilen;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening Socket.");
        return ERROR;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    int portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Binding Failed.");
        return ERROR;
    }

    // 5 is the number of clients that can connect to the server at a time
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // 192




    return 0;
}