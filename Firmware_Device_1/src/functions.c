#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h> // library to check existing files and for read, write, and close functions for socket programming
#include <openssl/pem.h> 
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/conf.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "declaration.h"

// ==== Returns Error and message of where it went wrong for socket section ====
void error(const char *msg) {
    perror(msg);
    // Include closed socket connection
    exit(1);
}

// ==== Check if key is already created with file name ====
bool is_Priv_Key_Exist(const char *key_path) {
    if (access(key_path, F_OK) == 0) {
        return true;
    } else {
        return false;
    }
}

// ==== Generate RSA key (PKEY) and check OpenSSL function creation ====
EVP_PKEY *generate_EVP_PKEY(const int size) {
    // Prepare area in memory for key generation
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

        // Check for OpenSSL memory prepartion failure
        if (!ctx) {
            ERR_print_errors_fp(stderr);
            return NULL;
        }

        // Initilize key generation at ctx location
        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            ERR_print_errors_fp(stderr);
            EVP_PKEY_CTX_free(ctx);
            return NULL;
        }
        // Set size of RSA key
        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, size) <= 0) {
            ERR_print_errors_fp(stderr);
            EVP_PKEY_CTX_free(ctx);
            return NULL;
        }
        // Generate the key at pkey location
        EVP_PKEY *pkey = NULL;

        if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
            ERR_print_errors_fp(stderr);
            EVP_PKEY_CTX_free(ctx);
            return NULL;
        }
        // Free memory used for key generation
        EVP_PKEY_CTX_free(ctx);
        return pkey;
}

// ==== Create file to store private key and write to it ====
int create_PivKey_File_write(const char priv_key_fp, EVP_PKEY *pkey) {
    FILE *file_storing_Privkey = fopen(priv_key_fp, "wb");
    // Make sure file opens smoothly
    if (!file_storing_Privkey) {
        fprintf(stderr, "Error opening private key file\n");
        return ERROR;
    }
    PEM_write_PrivateKey(file_storing_Privkey, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(file_storing_Privkey);
    return SAFE_OK;
}