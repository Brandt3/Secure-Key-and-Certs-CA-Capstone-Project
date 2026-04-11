#define _POSIX_C_SOURCE 200809L
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
#include <openssl/pem.h> 
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/conf.h>
#include <signal.h> 
#include <openssl/bio.h>
#include "file_utils.h"

volatile sig_atomic_t keep_running = 1; // Global flag for exiting "server loop"

// Handler
void handle_sigint(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(int argc, char *argv[]) {

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // Explicitly no SA_RESTART
    sigaction(SIGINT, &sa, NULL);

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

    memset(&serv_addr, 0, sizeof(serv_addr));
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

    char *buffer = NULL;
    buffer = malloc(10);

    while (keep_running) {
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            if (keep_running == 0) break; //Exit loop after Ctrl+C
            if (errno == EINTR) continue; // interrupted but not a shutdown signal 
            printf("Error on Accepting socket connections");
            continue;
        }

        // Recieve size of Cert that is expected to be received
    // ===== ENDED HERE =====
        size_t expect_file_size = 0;

        int val = recv(newsockfd, &expect_file_size, sizeof(expect_file_size), 0);
        if (val <= 0) {
            printf("Failed to recieve size of expected data for the validation: DISCONNECTING\n");
            close(newsockfd);
            continue;
        }


        char *temp = realloc(buffer, expect_file_size);
        if (!temp) {
            printf("Failed to realloc buffer memory\n");
            close(newsockfd);
            close(sockfd);
            return ERROR;
        }
        buffer = temp;

        size_t total = 0;
        while (total < expect_file_size) {
            ssize_t n = recv(newsockfd, buffer + total, expect_file_size - total, 0);
            if (n <= -1) {
                printf("recieve failed for the Power Hypervisor\n");
                free(buffer);
                close(newsockfd);
                close(sockfd);
                return ERROR;
            }
            total += n;
        }
        printf("Received device cert and checking SSL/TSL validation\n");
        buffer[expect_file_size] = '\0';

        FILE *ca_cert_fp = fopen("Power_Hypervisor/trusted_ca/ca_cert.crt", "r");
        if (!ca_cert_fp) {
            printf("Failed to open trusted CA Cert\n");
            close(newsockfd);
            break;
        }

        X509 *ca_cert = PEM_read_X509(ca_cert_fp, NULL, NULL, NULL);
        fclose(ca_cert_fp);
        EVP_PKEY *ca_pubkey = X509_get_pubkey(ca_cert);
        X509_free(ca_cert);
        if (!ca_pubkey) {
            printf("Failed to get public key from CA cert\n");
            EVP_PKEY_free(ca_pubkey);
            close(newsockfd);
            break;
        }

        // Read Device cert from buffer to OpenSSL cert struct
        BIO *bio = BIO_new_mem_buf(buffer, total);
        X509 *device_cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
        BIO_free(bio);

    // Perform SSL/TLS validations

        /* For Future adition to progress could lookingot using this instead to check chain of trust as well
                    X509_STORE *store = X509_STORE_new();
                    X509_STORE_add_cert(store, ca_cert);

                    X509_STORE_CTX *ctx = X509_STORE_CTX_new();
                    X509_STORE_CTX_init(ctx, store, device_cert, NULL);

                    int result = X509_verify_cert(ctx);
        
        */


        // #1 Verify Cert has the trusted CA's signature
        int n = X509_verify(device_cert, ca_pubkey);
        if (n == 0) {
            printf("CERTIFICATE NOT VALID: Certs signature is not from a trusted CA therefore the device is not trusted: DISCONNECTING socket connection\n");
            close(newsockfd); // Disconnect Deivce that's not trusted
            continue;
        } else if (n < 0) {
            printf("Error verifying certs signature\n");
            close(newsockfd);
            X509_free(device_cert);
            EVP_PKEY_free(ca_pubkey);
            continue;
        }

        // #2 Check cert experation date
        const ASN1_TIME *not_before = X509_get0_notBefore(device_cert);
        const ASN1_TIME *not_after  = X509_get0_notAfter(device_cert);
        int day = 0;
        int sec = 0;

        // Check if cert is not yet valid
        if (ASN1_TIME_diff(&day, &sec, NULL, not_before) == 1) {
            if (day > 0 || sec > 0) {
                printf("CERTIFICATE NOT VALID: Certs is not valid yet (to soon): DISCONNECTING socket connection\n");
                close(newsockfd);
                X509_free(device_cert);
                EVP_PKEY_free(ca_pubkey);
                continue;
            }
        }

        // Check if expired
        if (ASN1_TIME_diff(&day, &sec, NULL, not_after) == 1) {
            if (day < 0 || sec < 0) {
                printf("CERTIFICATE NOT VALID: Certs is expired: DISCONNECTING socket connection\n");
                close(newsockfd);
                X509_free(device_cert);
                EVP_PKEY_free(ca_pubkey);
                continue;
            }
        }
        printf("CERTIFICATE TRUSTED: You can now perform actions with the Power Hyperviosr\n");
    }


    printf("\nServer shutting down: CLOSED\n");
    free(buffer);
    close(sockfd);



    return 0;
}