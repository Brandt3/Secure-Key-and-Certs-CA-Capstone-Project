#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


void error(const char *msg) {
    perror(msg);
    exit(1); 
}

// Run program with 2 arguments required
int main(int argc, char *argv[]) {

    // if user doens't provide 2 arguements
    if (argc < 2) {
        fprintf(stderr, "Port not provided. Program terminated\n");
        exit(1);
    }

    int newsockfd, n;
    char buffer[255];

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

    /*  Set the CA folder connected to a port to allow it to act as a server that is "always running" simulating a network with device, CA, and powerHypervisor
            Then this can run sperately and always be running if wanted

        Step 1: On powering up connect to a port

        Step 2: Loop a listen function to listen for csr request to the CA "port"

        Step 3: Once reecieeved a CSR reads and Validate CSR was signed by private key of matching public key (integrity)

        Step 4: If valid step 3 then read CSR's CN (Common Name) SN (serial number) ensuring they are presenet, it matches with listed possiblitlies, and that it isn't already deployed or signed.
            could look to add key strength Ex. if key bit < 2048 reject "Not secure enough key"

        Step 5: If all steps are valid sign csr with private key and send new cert back to device. Then store signed cert and SN and CN name which it can use later to see if dupilcate csr's are sent

        Step 6: If at any point something isn't valid reject it print a statement of the device name and why it's rejected, adn store it to certs/rejected which can be helpful to anaylize what device were trying to connect
            Ex. "Device 1 SN CN was rejected by CA because it was not a valid SN option"

        

    */




    return 0;
}