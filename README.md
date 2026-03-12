# Big Picture Project Goal

Each Folder is a seperate "device"

Each "device" contains it's own public and private key

Firmware Devices will each start with a (Device like) CSR and submit it to the CA

The CA will sign one, reject one, and the final one will sign it's own (Device like) CSR

Then each device will try and connect to the Power hyperviosr 

The Power hyperviosr will only allow a connection if the trusted CA public key(s) it has stored can verfiy the signature on the cert of each device


NOTE: Certs for device are differnet then basic certs as they will have optional exteension fields to show Firmware version, Hardware model, date manufactured using (using config file or -addext)

# File Structure

CA 
| 
| - - Keys (Public and Private)
|
| - - Signed Certs / Rejected Certs
|
| - - Criteria Config file (Maybe)
|
| - - Program / src
        |
        | - - main.c
        | - - function.c
        | - - declaration.h
|
| - - README.md



