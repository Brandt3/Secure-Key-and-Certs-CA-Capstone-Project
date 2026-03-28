#ifndef DECLARATION_H
#define DECLARATION_H
#include <openssl/evp.h>


#define SAFE_OK 0;
#define ERROR -1;

void error(const char *msg);

bool is_Priv_Key_Exist(const char *key_path);

EVP_PKEY *generate_EVP_PKEY(const int size);

int create_PivKey_File_write(const char priv_key_fp, EVP_PKEY *pkey);

#endif