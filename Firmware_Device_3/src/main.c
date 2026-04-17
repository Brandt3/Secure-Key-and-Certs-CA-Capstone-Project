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
#include "file_utils.h"


// NOTE: good check after program is competed is to make sure each pointer have a functions return as it's value should be checked to make sure it's not NULL
    // Look to create efficeint debugging functions to prevemt redundant code here

/* common practice
EVP_PKEY *pkey = NULL;
X509_REQ *req = NULL;

pkey = load_key();
if (!pkey) goto cleanup;

req = create_csr();
if (!req) goto cleanup;

cleanup:
EVP_PKEY_free(pkey);
X509_REQ_free(req);
*/


// Possible later note look into making it so only device_3 can open the keys and certs file within 
// it's on directory and no one else can as that is a security threat


/*
Test Cases:
    Previously Nothing -> Create everything GOOD
    Previously Pubkey -> Create Private key, CSR, and CSR contains public key matching one derived from the private key (public key must get rewritten) GOOD
    Previously Privkey -> Create Public key, CSR, and public key matches csr public key GOOD
    Previously csr -> Didn't run csr creation, creeated both keys, public key matches csr public key GOOD
    Previously both keys -> Didn't create keys, created csr, public key matches public key in csr GOOD
    Previously pubkey and csr -> created private key, public key matches public key in csr GOOD
    Previously privkey and csr -> did not create extra csr, created public key, public key matches csr public key GOOD
    Previously everything -> Nothing extra created GOOD
*/

// Needs file, CA server IP address (loopback for internal), CA port number, Power Hyperviosr IP address, Power Hyperviosr port number
int main(int argc, char *argv[]) {

    // In a real situation Steps 1 and 2 already have the info stored on a secure flash (created at manufacturing) 
    // and step 2 is a presigned cert and not a csr. 

    // Note keys are NEVER regenerated each time power is on 

// 1. Upon "Power On" Check if there are keys created if not Create Private and Public key (RSA or EC)
        // Store Key in the keys file
        // Note Keys persist across reboots (try to emulate that here as well so only create the keys once)

    // Store keys within the key directory for security
    const char key_name_priv[] = "Firmware_Device_3/keys/firmware3_priv_rsa_key.pem";
    const char key_name_pub[] = "Firmware_Device_3/keys/firmware3_pub_rsa_key.pem";
    const int key_size = 2048;

    bool is_privkey_created = false;

// If private key doesn't exist
    if (!is_fp_Exist(key_name_priv)) {
        is_privkey_created = true;

    // Create pkey structure which stores public and private key along with other attirbutes
        EVP_PKEY *pkey = generate_EVP_PKEY(key_size);

    // Write private key to secure .pem file
        if (file_Create_PrivKey_Write(key_name_priv, pkey) == ERROR) {
            fprintf(stderr, "Error opening private key file\n");
            free_Pkey(pkey);            
            return ERROR;
        }

    // Write public key
        if (file_Create_PubKey_Write(key_name_pub, pkey) == ERROR) {
            fprintf(stderr, "Error opening public key file\n");
            free_Pkey(pkey);            
            return ERROR;
        }
    
        // Free memory storing key data structure as it is now written to a file a shouldn't be store to memory
        free_Pkey(pkey);
        printf("Created Public and Private RSA key\n");
 
//  Check if public key fp exist / is created
    } else if(!is_fp_Exist(key_name_pub)) { //If there is a private key but not public key create public key from already created private key
        FILE *priv_key_fp = fopen("Firmware_Device_3/keys/firmware3_priv_rsa_key.pem", "r");
        if (!priv_key_fp) {
            fprintf(stderr, "Error opening private key file for device3 for public key creation\n");
        }

        EVP_PKEY *pkey = PEM_read_PrivateKey(priv_key_fp, NULL, NULL, NULL);

    // Write public key
        if (file_Create_PubKey_Write(key_name_pub, pkey) == ERROR) {
            fprintf(stderr, "Error opening public key file\n");
            free(pkey);
            return ERROR;
        }

    // Free memory storing key data structure as it is now written to a file a shouldn't be store to memory
        free_Pkey(pkey);
        printf("Retrieved Public key from Private key file\n");
        


    } else {
        printf("Didn't run key creation they are already created\n");

    }


// ======= FUNCTION HAVE NOT BEEN CREATED FOR BELOW CODE =======

    /*
    Intersting factor the pkey is only created if key isn't already created
    If it is there is no pkey for csr creation
    SOLUTION: you can still create a csr as long as the private key is saved.
        Therefore the csr creationg will use the private key file instead of the pkey structure so it 
        doesn't have to be recreated in case the keys are created but not the csr
    */



// 2. Create CSR and request user for CSR info plus exstention
    // Store CSR in certs file
    // Input all CSR info required / could also pull info from a config file
    // Real world the device read this info from a NVRAM/secure storage

    // Creating variable to store config file
    const char csr_fp[] = "Firmware_Device_3/certs/device3.csr";

    // If csr file doesn't exist create it
    if (!is_fp_Exist(csr_fp)) {
        FILE *priv_key_fp = fopen("Firmware_Device_3/keys/firmware3_priv_rsa_key.pem", "r");
        if (!priv_key_fp) {
            fprintf(stderr, "Error opening private key file for device3\n");
        }

        EVP_PKEY *pkey = PEM_read_PrivateKey(priv_key_fp, NULL, NULL, NULL);
        if (!pkey) {
            fprintf(stderr, "Creating pkey based of private key file reading failed for device 3\n");
        }

        // Once I turn this into a function this will be the function header
        // int generate_csr_from_config(EVP_PKEY *pkey, const char *config_path)  

    // STEP 1: Load OpenSSL config from file
        // creates a OpenSSL config structure 
        CONF *conf = NCONF_new(NULL); 
        const char config_path[] = "Firmware_Device_3/config/device3_csr.conf";
        if (NCONF_load(conf, config_path, NULL) <= 0) {
            ERR_print_errors_fp(stderr);
            return ERROR;
        }
        
    // STEP 2: Create empty CSR object
        // Allocates memory for CSR structure
        X509_REQ *csr = X509_REQ_new();
        if (!csr) {
            NCONF_free(conf);
            return ERROR;
        }

    // STEP 3: Set CSR version (v1 = 0) 
        X509_REQ_set_version(csr, 0);
        
    // STEP 4: Get the subject name section from config
        // Sets pointer to thee subject name inside the CSR
        // Does not allocate new memory just points to CSR internal structure
        X509_NAME *name = X509_REQ_get_subject_name(csr);
        if(!name) {
            fprintf(stderr, "Error loading config file subject name for firmware device3\n");
            return ERROR;
        }
        
    // STEP 5: Read each field from config and add to subject
        // Get the "req_dn" section and retursn a "stack"/list of key-value pair Ex. C = US
        STACK_OF(CONF_VALUE) *dn_sk = NCONF_get_section(conf, "req_dn");
        if(!dn_sk) {
            fprintf(stderr, "Error reading config file for firmware device3\n");
            return ERROR;   
        }

        // NOTE: look more into what this code is doing
        if (dn_sk) {
            // Loop through each entry in [req_dn] section
            // sk_CONF_VALUE_num reeturns num of entries
            for (int i = 0; i < sk_CONF_VALUE_num(dn_sk); i++) {
                CONF_VALUE *v = sk_CONF_VALUE_value(dn_sk, i);
                
                // v->name = field name (e.g., "C", "O", "CN")
                // v->value = field value (e.g., "US", "IBM", "Device-ABC123")
                
                X509_NAME_add_entry_by_txt(name, v->name, MBSTRING_ASC,
                                            (unsigned char *)v->value, -1, -1, 0);
            }
        }

    // STEP 6: set the public key in CSR 
        /* NOTE: EVP_PKEY *pkey is a key container structure that stores Key type, pointer to key, and reference counter
            So pkey stores both public and private key it just depends on how you extract it Ex. X509_REQ_set_pubkey */
        X509_REQ_set_pubkey(csr, pkey);


    // STEP 7: Sign the CSR with private key
        // This proves you own the key pair
        // Hash all CSR data with SHA-256
        // Sign the hash with private key
        // Attach signature to CSR
        if (!X509_REQ_sign(csr, pkey, EVP_sha256())) {
            fprintf(stderr, "Error signing CSR\n");
            X509_REQ_free(csr);
            NCONF_free(conf);
            return ERROR;
        }
        
    // STEP 8: Write CSR to file
        FILE *csr_fp = fopen("Firmware_Device_3/certs/device3.csr", "wb");
        if (!csr_fp) {
            fprintf(stderr, "Error opening file\n");
            X509_REQ_free(csr);
            NCONF_free(conf);
            return ERROR;
        }
        
        PEM_write_X509_REQ(csr_fp, csr);
        fclose(csr_fp);
        
    // STEP 9: Cleanup all allocated memory
        EVP_PKEY_free(pkey);
        X509_REQ_free(csr);
        NCONF_free(conf);
        printf("Created CSR signing with private key and getting data from config file\n");

        
    } else if (is_privkey_created) { // If there was a previous csr but a pirvate key just got create new csr must be made with private key signature
        if (remove(csr_fp) != 0) {
            perror("Error deleting csr file");
        }

// All this is duplicate code that can be put into a function 
        FILE *priv_key_fp = fopen("Firmware_Device_3/keys/firmware3_priv_rsa_key.pem", "r");
        if (!priv_key_fp) {
            fprintf(stderr, "Error opening private key file for device3\n");
        }

        EVP_PKEY *pkey = PEM_read_PrivateKey(priv_key_fp, NULL, NULL, NULL);
        if (!pkey) {
            fprintf(stderr, "Error creating pkey based of private key file reading failed for device 3\n");
        }

        // Once I turn this into a function this will be the function header
        // int generate_csr_from_config(EVP_PKEY *pkey, const char *config_path)  

    // STEP 1: Load OpenSSL config from file
        // creates a OpenSSL config structure 
        CONF *conf = NCONF_new(NULL); 
        char config_path[] = "Firmware_Device_3/config/device3_csr.conf";
        if (NCONF_load(conf, config_path, NULL) <= 0) {
            ERR_print_errors_fp(stderr);
            return ERROR;
        }
        
    // STEP 2: Create empty CSR object
        // Allocates memory for CSR structure
        X509_REQ *csr = X509_REQ_new();
        if (!csr) {
            NCONF_free(conf);
            fprintf(stderr, "Failed to allocate memory for CSR structure");
            return ERROR;
        }

    // STEP 3: Set CSR version (v1 = 0) 
        X509_REQ_set_version(csr, 0);
        
    // STEP 4: Get the subject name section from config
        // Sets pointer to thee subject name inside the CSR
        // Does not allocate new memory just points to CSR internal structure
        X509_NAME *name = X509_REQ_get_subject_name(csr);
        if(!name) {
            fprintf(stderr, "Error loading config file subject name for firmware device3\n");
            return ERROR;
        }
        
    // STEP 5: Read each field from config and add to subject
        // Get the "req_dn" section and retursn a "stack"/list of key-value pair Ex. C = US
        STACK_OF(CONF_VALUE) *dn_sk = NCONF_get_section(conf, "req_dn");
        if(!dn_sk) {
            fprintf(stderr, "Error reading config file for firmware device3\n");
            return ERROR;   
        }

        // NOTE: look more into what this code is doing
        if (dn_sk) {
            // Loop through each entry in [req_dn] section
            // sk_CONF_VALUE_num reeturns num of entries
            for (int i = 0; i < sk_CONF_VALUE_num(dn_sk); i++) {
                CONF_VALUE *v = sk_CONF_VALUE_value(dn_sk, i);
                
                // v->name = field name (e.g., "C", "O", "CN")
                // v->value = field value (e.g., "US", "IBM", "Device-ABC123")
                
                X509_NAME_add_entry_by_txt(name, v->name, MBSTRING_ASC,
                                            (unsigned char *)v->value, -1, -1, 0);
            }
        }

    // STEP 6: set the public key in CSR 
        /* NOTE: EVP_PKEY *pkey is a key container structure that stores Key type, pointer to key, and reference counter
            So pkey stores both public and private key it just depends on how you extract it Ex. X509_REQ_set_pubkey */
        X509_REQ_set_pubkey(csr, pkey);


    // STEP 7: Sign the CSR with private key
        // This proves you own the key pair
        // Hash all CSR data with SHA-256
        // Sign the hash with private key
        // Attach signature to CSR
        if (!X509_REQ_sign(csr, pkey, EVP_sha256())) {
            fprintf(stderr, "Error signing CSR\n");
            X509_REQ_free(csr);
            NCONF_free(conf);
            return ERROR;
        }
        
    // STEP 8: Write CSR to file
        FILE *csr_fp = fopen("Firmware_Device_3/certs/device3.csr", "wb");
        if (!csr_fp) {
            fprintf(stderr, "Error opening file\n");
            X509_REQ_free(csr);
            NCONF_free(conf);
            return ERROR;
        }
        
        PEM_write_X509_REQ(csr_fp, csr);
        fclose(csr_fp);
        
    // STEP 9: Cleanup all allocated memory
        EVP_PKEY_free(pkey);
        X509_REQ_free(csr);
        NCONF_free(conf);
        printf("Removed old CSR since it didn't match the new private key; Created CSR signing with private key and getting data from config file\n");




    } else {

        printf("Didn't run CSR creation it is already created\n");
    }
    

    

// 3. Send CSR to CA to see if it will get signed
    // If it does store cert in certs file and delete CSR (Privode a status update in the terminal for each device)
    // If reject display message / echo "Device number could not get signed as it does not meet requirements therefore it can not try and connect to the Power Hypervisor"
    // Can look into ssl_connect, ssl_write, ssl_read which is used to authenicate the server trying to connect to
    struct sockaddr_in serv_addr;
    struct hostent *server;

    // Checking to make sure port and host IP was included when ran
    if (argc < 3) {
        fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
        return ERROR;
    }

    int portno = atoi(argv[2]);

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
        return ERROR;
    }

    // Get server address
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return ERROR;
    }

    // Set up address struct
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr_list[0],
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);

    serv_addr.sin_port = htons(portno);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR connecting to server on Device 3\n");    
        close(sockfd);
        return ERROR;
    }

    printf("Connected to CA server\n");

    // Opening csr file to get file size and send
    FILE *fp = fopen(csr_fp, "rb");
    if (!fp) {
        printf("Error opening csr file on device 3\n");
        close(sockfd);
        return ERROR;
    }

    size_t file_size = 0;
    char *buffer = NULL;

    // Go to end of file to get size
    if (fseek(fp, 0, SEEK_END) == 0) {
        file_size = ftell(fp);         // Get the current position (which is the file size in bytes)
        rewind(fp);         // Reset the file pointer to the beginning if further reading is needed
    } else {
        perror("Error accessing the end of file in device3\n");
        close(sockfd);
        return ERROR;
    }
    buffer = malloc(file_size);

    // Getting content of csr file to send to CA
    ssize_t bytes_read = fread(buffer, 1, file_size, fp);

    // Sending size of expected data to CA
    int n = send(sockfd, &bytes_read, sizeof(bytes_read), 0);
    if (n < 0) {
        printf("ERROR sending size to socket\n");
        close(sockfd);
        return ERROR;
    }
    ssize_t total_sent = 0;

    // ---- SEND DATA (CSR SIMULATION) ----
    printf("Sending CSR for signing: \n");

    // Send data/bytes until total size sent = the file size
    while (total_sent < bytes_read) {
        ssize_t n = send(sockfd, buffer + total_sent, bytes_read - total_sent, 0);
        if (n <= 0) {
            printf("Error sending csr to device 3 over the socket\n");
            close(sockfd);
            return ERROR;
        }
        total_sent += n;
    }

// === Receiving Data from CA ===

    // Receiving if csr was signed or not from CA

    const char *expected_cert_fp = "Firmware_Device_3/certs/device3_cert.crt";

    bool is_cert_signed = false;
    size_t expect_file_size = 0;
    int num = recv(sockfd, &is_cert_signed, sizeof(is_cert_signed), 0);
    if (num == -1) {
        printf("Failed to recieve boolean check from CA\n");
        close(sockfd);
        return ERROR;
    }
    // If cert is signed then prepare to recieve cert
    if (is_cert_signed) {
        int val = recv(sockfd, &expect_file_size, sizeof(expect_file_size), 0);
        if (val == -1) {
            perror("Failed to recieve size of expected data for the CA");
            close(sockfd);
            return ERROR;
        }

        char *temp = realloc(buffer, expect_file_size);
        if (!temp) {
            printf("Error Reallocating memory for buffer\n");
            free(buffer);
            close(sockfd);
            return ERROR;
        }
        buffer = temp;

        size_t total = 0;
        while (total < expect_file_size) {
            ssize_t n = recv(sockfd, buffer + total, expect_file_size - total, 0);
            if (n <= 0) {
                printf("recieve failed for the device\n");
                close(sockfd);
                return ERROR;
            }
            total += n;
        }

        FILE *cert_fp = fopen("Firmware_Device_3/certs/device3_cert.crt", "wb");
        if (!cert_fp) {
            printf("Error opening file to write device certificate\n");
            close(sockfd);
            return ERROR;
        }
        fwrite(buffer, 1, total, cert_fp);
        fclose(cert_fp);
    } else if (!is_fp_Exist(expected_cert_fp)) {
        printf("CSR was not signed by CA. I will sign it myself\n");

        FILE *csr_fp = fopen("Firmware_Device_3/certs/device3.csr", "r");
        if (!csr_fp) {
            perror("CSR open failed");
            close(sockfd);
            return ERROR;
        }

        // 1. Load CSR
        X509_REQ *req = PEM_read_X509_REQ(csr_fp, NULL, NULL, NULL);
        fclose(csr_fp);

        if (!req) {
            printf("Failed to read CSR for self signing\n");
            close(sockfd);
            return ERROR;
        }

        // 2. Extract public key from CSR
        EVP_PKEY *pubkey = X509_REQ_get_pubkey(req);

        // 3. Load private key (used to sign cert)
        FILE *key_fp = fopen("Firmware_Device_3/keys/firmware3_priv_rsa_key.pem", "r");
        EVP_PKEY *device_pivkey = PEM_read_PrivateKey(key_fp, NULL, NULL, NULL);
        fclose(key_fp);

        if (!device_pivkey) {
            printf("Failed to load private key for self signing\n");
            close(sockfd);
            return ERROR;
        }

        // 4. Create certificate
        X509 *cert = X509_new();
        X509_set_version(cert, 2);         // Set version (v3 = 2)
        ASN1_INTEGER_set(X509_get_serialNumber(cert), 1);         // Serial number
        X509_gmtime_adj(X509_get_notBefore(cert), -60);         // Validity
        X509_gmtime_adj(X509_get_notAfter(cert), 31536000L); // 1 year

        // 5. Set subject from CSR
        X509_set_subject_name(cert, X509_REQ_get_subject_name(req));

        // 6. Self-signed → issuer = subject
        X509_set_issuer_name(cert, X509_REQ_get_subject_name(req));

        // 7. Set public key
        X509_set_pubkey(cert, pubkey);

        // 8. Sign certificate
        if (!X509_sign(cert, device_pivkey, EVP_sha256())) {
            printf("Signing failed\n");
            close(sockfd);
            return ERROR;
        }

        // 9. Write certificate to file
        FILE *out = fopen("Firmware_Device_3/certs/device3_cert.crt", "w");
        PEM_write_X509(out, cert);
        fclose(out);

        printf("Certificate successfully self signed and created!\n");

        // Cleanup
        X509_REQ_free(req);
        EVP_PKEY_free(pubkey);
        EVP_PKEY_free(device_pivkey);
        X509_free(cert);

    }
    printf("Closed CA Socket Connection\n");
    close(sockfd);



// 4. If it has a signed cert send it to the Power Hypervisor to try and connect

    struct sockaddr_in power_serv_addr;
    struct hostent *power_server;

    // Checking to make sure port and host IP was included when ran
    if (argc < 5) {
        fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
        return ERROR;
    }

    int server_portno = atoi(argv[4]);

    // Create socket
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        error("ERROR opening socket");
        return ERROR;
    }

    // Get server address
    power_server = gethostbyname(argv[3]);
    if (power_server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return ERROR;
    }

    // Set up address struct
    memset(&power_serv_addr, 0, sizeof(power_serv_addr));
    power_serv_addr.sin_family = AF_INET;

    bcopy((char *)power_server->h_addr_list[0],
          (char *)&power_serv_addr.sin_addr.s_addr,
          power_server->h_length);

    power_serv_addr.sin_port = htons(server_portno);

    // Connect to server
    if (connect(server_sockfd, (struct sockaddr *)&power_serv_addr, sizeof(power_serv_addr)) < 0) {
        printf("ERROR connecting to server on Device 3\n");    
        close(server_sockfd);
        return ERROR;
    }
    printf("Connected to Power Hypervisor\n");
    
    // Sending size of expected data to device
    file_size = 0;

    FILE *cert_fp = fopen("Firmware_Device_3/certs/device3_cert.crt", "r");
    if (!cert_fp) {
            printf("Failed opening cert file to send to Power Hypervisor\n");
            close(server_sockfd);
            free(buffer);
            return ERROR;
        }

    if (fseek(cert_fp, 0, SEEK_END) == 0) {
        file_size = ftell(cert_fp);         // Get the current position (which is the file size in bytes)
        rewind(cert_fp);         // Reset the file pointer to the beginning if further reading is needed
    } else {
        perror("Error accessing the end of file in device3\n");
        fclose(cert_fp);
        return ERROR;
    }

    char *temp = realloc(buffer, file_size);
    if (!temp) {
        printf("Error reallocating buffer for Power Hypervisor\n");
        close(server_sockfd);
        free(buffer);
        return ERROR;
    }
    buffer = temp;

    n = send(server_sockfd, &file_size, sizeof(file_size), 0);
    if (n < 0) {
        error("ERROR sending size to Power Hypervisor\n");
        return ERROR;
    }
   
    // Read cert into buffer    
    bytes_read = fread(buffer, 1, file_size, cert_fp); // writting file into buffer
    fclose(cert_fp);

    size_t amount_sent = 0;
    printf("Sending Certifacte to Power Hypervisor: \n");
    // Send Cert to try and make a trusted connection with the Power Hyperviosr
    while (amount_sent < file_size) {
        ssize_t n = send(server_sockfd, buffer + amount_sent, bytes_read - amount_sent, 0);
        if (n <= 0) {
            error("Error sending cert to Power Hypervisor over the socket\n");
            return ERROR;
        }
        amount_sent += n;
    }

    

    printf("closed\n");
    free(buffer);
    close(server_sockfd);


    // 5. If cert is trusted and makes a secure SSL/TLS connection then perform a task or communicate with the Power Hypervisor
    

    return 0;
}