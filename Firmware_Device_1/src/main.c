#include <stdio.h>
#include <stddef.h>
#include <unistd.h> // library to check existing files
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>


// Possible later note look into making it so only device_1 can open the keys and certs file within 
// it's on directory and no one else can as that is a security threat


int main() {

    // In a real situation Steps 1 and 2 already have the info stored on a secure flash (created at manufacturing) 
    // and step 2 is a presigned cert and not a csr. 

    // Note keys are NEVER regenerated each time power is on 

    // 1. Upon "Power On" Check if there are keys created if not Create Private and Public key (RSA or EC)
        // Store Key in the keys file
        // Note Keys persist across reboots (try to emulate that here as well so only create the keys once)

    // Store within the key files for security
    char key_name_priv[35] = "keys/firmware1_priv_rsa_key.pem";
    char key_name_pub[35] = "keys/firmware1_pub_rsa_key.pem";


    // If keys doesn't exisit
    if (access(key_name_priv, F_OK) != 0 && access(key_name_pub, F_OK) != 0) {
        // Prepare area in memory for key generation
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

        // Check for OpenSSL memory prepartion failure
        if (ctx == NULL) {
            fprintf(stderr, "Error preparing area in memory for Firmware Device 1 key creation\n");
            ERR_print_errors_fp(stderr);
            return 1;
        }

        // Initilize key generation at ctx location
        EVP_PKEY_keygen_init(ctx);
        // Set size of RSA key
        EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048);
        // Generate the key at pkey location
        EVP_PKEY *pkey = NULL;
        EVP_PKEY_keygen(ctx, &pkey);
        // Free memory used for key generation
        EVP_PKEY_CTX_free(ctx);

        // Create file to store private key
        FILE *file_storing_Privkey = fopen(key_name_priv, "wb");
        // Make sure file opens smoothly
        if (file_storing_Privkey == NULL) {
            fprintf(stderr, "Error opening private key file\n");
            return 1;
        }
        // There is a password arguement here could look into
        PEM_write_PrivateKey(file_storing_Privkey, pkey, NULL, NULL, 0, NULL, NULL);
        fclose(file_storing_Privkey);

        // Write public key
        FILE *file_storing_Pubkey = fopen(key_name_pub, "wb");
        // Make sure file opens smoothly
        if (file_storing_Pubkey == NULL) {
            fprintf(stderr, "Error opening public key file\n");
            return 1;
        }
        PEM_write_PUBKEY(file_storing_Pubkey, pkey);        
        fclose(file_storing_Pubkey);


        // Free memory storing key data structure as it is now written to a file a shouldn't be store to memory
        EVP_PKEY_free(pkey);
 
    }

    // 2. Create CSR and request user for CSR info plus exstention
        // Store CSR in certs file
        // Input all CSR info required / could also pull info from a config file
        // Real world the device read this info from a NVRAM/secure storage

    // 3. Send CSR to CA to see if it will get signed
        // If it does store cert in certs file and delete CSR
        // If reject display message / echo "Device number could not get signed as it does not meet requirements therefore it can not try and connect to the Power Hypervisor"

    // 4. If it has a signed cert send it to the Power Hypervisor to try and connect

    // 5. If cert is trusted then perform a task or communicate with the Power Hypervisor
    

    return 0;
}