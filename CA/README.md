# CA — Certificate Authority

The CA folder connects to a port to act as a server that is "always running," allowing it to run separately and remain active indefinitely.

---

## Flow

### Step 1 — Power On
Connect to a port and begin listening.

### Step 2 — Listen
Loop a listen function to wait for incoming CSR requests on the CA port.

### Step 3 — Receive & Validate CSR
Once a CSR is received, validate the following fields before it can be signed:

- Private key used to sign matches the public key *(authenticity)*
- Hashing the body of the CSR matches the hash in the signature *(integrity)*
- Serial number is in the database and unused
- Device name is in the database and unused
- Organization name matches the expected company

### Step 4 — Verify CSR Fields
If Step 3 passes, read the CSR's CN (Common Name) and SN (Serial Number), ensuring:

- Both fields are present
- They match a listed valid option
- The device has not already been deployed or signed

> 💡 **Possible addition:** Check key strength — e.g., if key bit length < 2048, reject with "Not secure enough key."

### Step 5 — Sign & Store
If all steps are valid, sign the CSR with the CA private key and send the new cert back to the device. Store the signed cert along with the SN and CN to detect any duplicate CSRs in the future.

### Step 6 — Reject & Log
If at any point validation fails, reject the CSR, print a statement explaining why, and store it to `certs/revoked/` for later analysis.

**Example:**
Device1 [SN: ABC123, CN: firmware-device-1] rejected by CA — invalid serial number.


## Database

Valid CN and SN entries can be stored using SQL, NoSQL, or a simple text file.

- **Current approach:** A text file storing valid CN and SN values — simple and easy to get started with, especially since my focus was not on database structure.
- Each device entry could alternatively be stored as a struct in an array, but a file-based database was chosen so the data persists outside of the program's lifecycle.