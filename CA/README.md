# CA Structure

Set the CA folder connected to a port to allow it to act as a server that is "always running"
Then this can run sperately and always be running if wanted

Step 1: On powering up connect to a port

Step 2: Loop a listen function to listen for csr request to the CA "port"

Step 3: Once reecieeved a CSR reads and Validate CSR and checks these fields before it can get signed
    * private key used to sign matches the public key for authenticity
    * When you hash the body of the csr it should match the hash in the signature this checks the integrity 
    * Check serial number that it's in the databse and unused
    * Device name that it's in the database and unused 
    * Organization Name that it is the proper company

Step 4: If valid step 3 then read CSR's CN (Common Name) SN (serial number) ensuring they are presenet, it matches with listed possiblitlies, and that it isn't already deployed or signed.
    could look to add key strength Ex. if key bit < 2048 reject "Not secure enough key"

Step 5: If all steps are valid sign csr with private key and send new cert back to device. Then store signed cert and SN and CN name which it can use later to see if dupilcate csr's are sent

Step 6: If at any point something isn't valid reject it print a statement of the device name and why it's rejected, adn store it to certs/rejected which can be helpful to anaylize what device were trying to connect
    Ex. "Device 1 SN CN was rejected by CA because it was not a valid SN option"

Database storing valid CN and SN could be SQL, noSQL, or simple text file.
    For better flow I will start with a text file storing valid CN and SN
    Each device within the database could have also been a struct stored to an array but I wanted a database that would last outside of the programs lifecycle