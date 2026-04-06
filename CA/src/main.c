#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/pem.h> 
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include "file_utils.h"


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
// Step 0: Create pubulic key, private key, and cert for CA only one time
    const char key_name_priv[] = "CA/keys/ca_priv_rsa_key.pem";
    const char key_name_pub[] = "CA/keys/ca_pub_rsa_key.pem";
    const int key_size = 2048;
    static long serial = 2; // Unique value set to certs serial number for future this should be from a file so it is stored


    // If private key doesn't exist it is created and also Power Sever CA public key must be updated
        // But the CA public key should already be created and stored in the TPM of the Power Sever
    if (!is_Key_Exist(key_name_priv)) {
    // Create pkey structure which stores public and private key along with other attirbutes
        EVP_PKEY *pkey = generate_EVP_PKEY(key_size);

    // Write private key to secure .pem file
        if (file_Create_PrivKey_Write(key_name_priv, pkey) == ERROR) {
            fprintf(stderr, "Error opening private key file\n");
            // FUNCTION STOP PROGRAM
            exit(1);
        }
        // Write public key
        if (file_Create_PubKey_Write(key_name_pub, pkey) == ERROR) {
            fprintf(stderr, "Error opening public key file\n");
            free_Pkey(pkey);
            return -1;
        }

        // Once I move this to a function this is what it will return  X509 *create_ca_cert(EVP_PKEY *ca_key) 
            X509 *cert = X509_new();
            X509_set_version(cert, 2); //  version (v3)
            ASN1_INTEGER_set(X509_get_serialNumber(cert), 1); // Set cert serial number
            X509_gmtime_adj(X509_get_notBefore(cert), 0);   // Set validity to now
            X509_gmtime_adj(X509_get_notAfter(cert), 31536000L); // 1 year

            // subject name (CA identity)
            X509_NAME *name = X509_get_subject_name(cert);
    // FIXME instead of static values loop through config file for subject data device 1 does this
            X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC,
                                    (unsigned char *)"US", -1, -1, 0);
            X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC,
                                    (unsigned char *)"MyOrg", -1, -1, 0);
            X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                                    (unsigned char *)"MyRootCA", -1, -1, 0);

            // issuer = subject (self-signed)
            X509_set_issuer_name(cert, name);

            // public key
            X509_set_pubkey(cert, ca_key);

            // 🔥 IMPORTANT EXTENSION: mark as CA
            X509_EXTENSION *ext;
            ext = X509V3_EXT_conf_nid(NULL, NULL, NID_basic_constraints, "CA:TRUE");
            X509_add_ext(cert, ext, -1);
            X509_EXTENSION_free(ext);

            // key usage (optional but good)
            ext = X509V3_EXT_conf_nid(NULL, NULL, NID_key_usage,
                                    "keyCertSign, cRLSign");
            X509_add_ext(cert, ext, -1);
            X509_EXTENSION_free(ext);

            // sign with CA private key (self-sign)
            X509_sign(cert, ca_key, EVP_sha256());
        

    
        // Free memory storing key data structure as it is now written to a file a shouldn't be store to memory
        free_Pkey(pkey);
    }




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
        * private key used to sign matches the public key for authenticity
        * When you hash the body of the csr it should match the hash in the signature this checks the integrity 
        * Check serial number that it's in the databse and unused
        * Device name that it's in the database and unused
*/

    // Writing csr to file to then conveert file to openSSL req struct to perform functions on
    FILE *csr_fp = fopen("CA/certs/signed/received.csr", "wb");
    if (!csr_fp) {
        fprintf(stderr, "Error in CA trying to open csr file to write\n");
        return 1;
    }
    fwrite(buffer, 1, total, csr_fp);
    fclose(csr_fp);         // Reset the file pointer to the beginning if further reading is needed

    // Reading csr and putting it into a openSSL struct 
    FILE *read_csr_fp = fopen("CA/certs/signed/received.csr", "r");
    X509_REQ *req = PEM_read_X509_REQ(read_csr_fp, NULL, NULL, NULL);
    fclose(read_csr_fp);

    // Extract public key from the csr
    EVP_PKEY *pubkey = X509_REQ_get_pubkey(req);
    // Checking Authenticity and Integrity 
    printf("Checking CSR's authenticity and integrity...\n");
    int check = X509_REQ_verify(req, pubkey); // Hashes the body and decrypts the signature using the public key and compares the two
    if (check < 0) {
        printf("Error during verification\n");
        EVP_PKEY_free(pubkey);
        X509_REQ_free(req); 
        return -1;    
    } else if (check == 0) {
        printf("CSR signature is INVALID\n");
        EVP_PKEY_free(pubkey);
        X509_REQ_free(req);
        return -1;
    }

    printf("CSR's authenticity and integrity verified moving on to check fields... \n");
    EVP_PKEY_free(pubkey);


// Checking CSR fields compared to database
    X509_NAME *subject = X509_REQ_get_subject_name(req); //Pulls all fields into a structured object
    char csr_common_name[256];
    char csr_serial_num[256];
    char csr_organization[256];
    X509_NAME_get_text_by_NID(subject, NID_commonName, csr_common_name, sizeof(csr_common_name));
    X509_NAME_get_text_by_NID(subject, NID_serialNumber, csr_serial_num, sizeof(csr_serial_num));
    X509_NAME_get_text_by_NID(subject, NID_organizationName, csr_organization, sizeof(csr_organization));

    printf("Device info after reading CSR Org -> %s Com Name -> %s Ser Num -> %s\n", csr_organization, csr_common_name, csr_serial_num);

    // Normally this would be a change to a SQL database instead of writing to a file
    FILE *db_fp = fopen("CA/database/approved_devices.txt", "r");
    if (!db_fp) {
        fprintf(stderr, "Error trying to open Approved Devices Data base\n");
        close(newsockfd);
        close(sockfd);
        return -1;
    }
    FILE *temp_db_fp = fopen("CA/database/temp_approved_devices.txt", "wb");
    if (!temp_db_fp) {
        fprintf(stderr, "Error trying to open TEMP Approved Devices Data base\n");
        close(newsockfd);
        close(sockfd);
        return -1;
    }
    char db_buffer[256];
    bool is_device_approved = false;

    while(fgets(db_buffer, sizeof(db_buffer), db_fp) != NULL) {
        int wordcount = 0;
        char org[25] = "";
        char com_n[25] = "";
        char sn[25] = "";
        int index_org = 0;
        int index_com_n = 0;
        int index_sn = 0;

        // Check if device already been signed off then skip to next device in database
        if (db_buffer[0] == '1') {
            fwrite(db_buffer, sizeof(db_buffer), 1, temp_db_fp);
            continue;
        }
        // Extracting each key word from the database for comparison
        for(size_t i  = 2; i < strlen(db_buffer) + 1; ++i) {
            if (db_buffer[i] == '|') {
                ++wordcount;
                continue;
            }
        // Important to replace \n with \0 so the string compare is perfect
            if (wordcount == 0) {
                if (db_buffer[i] == '\n') {
                    org[index_org++] = '\0';
                    continue;
                }
                org[index_org++] = db_buffer[i];
            } else if (wordcount == 1) {
                if (db_buffer[i] == '\n') {
                    com_n[index_com_n++] = '\0';
                    continue;
                }
                com_n[index_com_n++] = db_buffer[i];
            } else {
                if (db_buffer[i] == '\n') {
                    sn[index_sn++] = '\0';
                    continue;
                }
                sn[index_sn++] = db_buffer[i];
            }
        }
        // Technically not needed because of how strings were intilaized but good practice
        org[index_org] = '\0';
        com_n[index_com_n] = '\0';
        sn[index_sn] = '\0';

        // See if the csr data matches the database
        if (strcmp(org, csr_organization) + strcmp(com_n, csr_common_name) + strcmp(sn, csr_serial_num) == 0) {
            printf("CSR matched Database attempted to update database...\n");
            fprintf(temp_db_fp, "1|%s|%s|%s\n", org, com_n, sn);
            printf("1|%s|%s|%s\n", org, com_n, sn);
            is_device_approved = true;
            printf("Database updated sending Cert signed by me \"CA\" sending back to device %s...\n", com_n);

        } else {
            fprintf(temp_db_fp, "0|%s|%s|%s\n", org, com_n, sn);
        }
    }
    fclose(temp_db_fp);
    fclose(db_fp);

// This is not ideal method for handling database but that is not the current focus can come back and improve later on
// Replacing old database with updated database
    if (remove("CA/database/approved_devices.txt") != 0) {
        perror("Error updating the old  database file");
    }
    if (rename("CA/database/temp_approved_devices.txt", "CA/database/approved_devices.txt") != 0) {
        perror("Error updating the new database file");
    }

    // based off is_device_approved then sign cert and send it back

    if (is_device_approved) {
        printf("CSR meets all requirements I will now sign the CSR and send a Certificate back to you...\n");
        FILE *priv_key_fp = fopen("CA/keys/ca_priv_rsa_key.pem", "r");
        if (!priv_key_fp) {
            fprintf(stderr, "Error opening private key file for CA for signing the cert\n");
        }
        EVP_PKEY *ca_priv_key = PEM_read_PrivateKey(priv_key_fp, NULL, NULL, NULL); // Get private key from key file


        X509 *cert = X509_new();
        X509_set_version(cert, 2);        // Set version of cert
        ASN1_INTEGER_set(X509_get_serialNumber(cert), serial++);   // Create a serial num for cert
        X509_set_issuer_name(cert, X509_get_subject_name(ca_cert));  // Set name of CA (issuer of the cert)  
        X509_set_subject_name(cert, X509_REQ_get_subject_name(req));  // Set subject from devices csr
        EVP_PKEY *pubkey = X509_REQ_get_pubkey(req);    // Get public key from device csr
        X509_set_pubkey(cert, pubkey);
        X509_gmtime_adj(X509_get_notBefore(cert), 0);   // Set dates for when cert is valid 0 = now
        X509_gmtime_adj(X509_get_notAfter(cert), 31536000L);
        // Signing cert that contains info about the csr, device, and CA
        X509_sign(cert, ca_priv_key, EVP_sha256());

    } else {
        printf("Device is not approved to be signed by the CA\n");
    }




    X509_REQ_free(req);




    printf("\nClosed\n");
    close(newsockfd);
    close(sockfd);

    return 0;
}