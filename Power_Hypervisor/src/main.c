#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h> // library to check existing files and for read, write, and close functions for socket programming
#include "file_utils.h"

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

// Step 2: Loop a listen function to listen for connection request to the server "port"

    // Eventually loop this so it connitnuously is listening and checking clients connection
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        printf("Error on Accepting socket connections");
        return ERROR;
    }

    // Recieve size of Cert that is expected to be received
// ===== ENDED HERE =====
    size_t expect_file_size = 0;
    char *buffer = NULL;

    int val = recv(newsockfd, &expect_file_size, sizeof(expect_file_size), 0);
    if (val <= 0) {
        printf("Failed to recieve size of expected data for the validation\n");
        return ERROR;
    }


    buffer = malloc(expect_file_size);
    size_t total = 0;
    while (total < expect_file_size) {
        ssize_t n = recv(newsockfd, buffer + total, expect_file_size - total, 0);
        if (n <= 0) {
            printf("recieve failed for the Power Hypervisor\n");
            return ERROR;
        }
        total += n;
    }



    free(buffer);
    close(newsockfd);
    close(sockfd);



    return 0;
}