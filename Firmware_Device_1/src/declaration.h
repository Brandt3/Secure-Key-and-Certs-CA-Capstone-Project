#ifndef DECLARATION_H
#define DECLARATION_H

#include <openssl/evp.h>
#include <stdbool.h>


#define SAFE_OK 0
#define ERROR -1

void error(const char *msg);

bool is_Key_Exist(const char *key_path);

EVP_PKEY *generate_EVP_PKEY(const int size);

int file_Create_PrivKey_Write(const char *priv_key_fp, EVP_PKEY *pkey);

int file_Create_PubKey_Write(const char *pub_key_fp, EVP_PKEY *pkey);

void free_Pkey(EVP_PKEY *pkey);


#endif