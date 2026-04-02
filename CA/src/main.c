#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/pem.h> 
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/x509.h>


void error(const char *msg) {
    perror(msg);
    exit(1); 
}


/*  Set the CA folder connected to a port to allow it to act as a server that is "always running" simulating a network with device, CA, and powerHypervisor
        Then this can run sperately and always be running if wanted

    

    Step 4: If valid step 3 then read CSR's CN (Common Name) SN (serial number) ensuring they are presenet, it matches with listed possiblitlies, and that it isn't already deployed or signed.
        could look to add key strength Ex. if key bit < 2048 reject "Not secure enough key"

    Step 5: If all steps are valid sign csr with private key and send new cert back to device. Then store signed cert and SN and CN name which it can use later to see if dupilcate csr's are sent

    Step 6: If at any point something isn't valid reject it print a statement of the device name and why it's rejected, adn store it to certs/rejected which can be helpful to anaylize what device were trying to connect
        Ex. "Device 1 SN CN was rejected by CA because it was not a valid SN option"

    

*/

// Run program with 2 arguments required (File name and port) Ex. ./test_program 9090
int main(int argc, char *argv[]) {

// Step 1: On powering up connect to a port
    // if user doens't provide 2 arguements
    if (argc < 2) {
        fprintf(stderr, "Port not provided. Program terminated\n");
        exit(1);
    }


    struct sockaddr_in serv_addr, cli_addr;

    // Data type 32 bit
    socklen_t clilen;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error opening Socket.");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    int portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("Binding Fialed.");
    }

    // 5 is the number of clients that can connect to the server at a time
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

// Step 2: Loop a listen function to listen for csr request to the CA "port"

// Eventually loop this so it connitnuously is listening and accepting clients connection and csr's

    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        error("Error on Accept");
    }

    size_t expect_file_size = 0;

    int val = recv(newsockfd, &expect_file_size, sizeof(expect_file_size), 0);
    if (val <= 0) error("Failed to recieve size of expected data for the CA\n");


    char buffer[expect_file_size];

    size_t total = 0;
    while (total < expect_file_size) {
        ssize_t n = recv(newsockfd, buffer + total, expect_file_size - total, 0);
        if (n <= 0) error("recieve failed for the CA\n");
        total += n;
    }

    

/* Step 3: Once reecieved a CSR reads and Validate CSR was signed by private key of matching public key (integrity)
    
    Things needed to be checked by CA before the csr can get signed
        * private key used to sign matches the public key a for authenticity
        * The hash of the data to check for integrity 
        * Check serial number that it's in the databse and unused
        * Device name that it's in the database and unused
*/


// Validation of received data
    FILE *csr_fp = fopen("certs/signed/received.csr", "wb");
    if (!csr_fp) {
        fprintf(stderr, "Error in CA trying to open csr file to write\n");
        return 1;
    }
    fwrite(buffer, 1, total, csr_fp);

    fclose(csr_fp);


    printf("\nClosed\n");
    close(newsockfd);
    close(sockfd);







    return 0;
}