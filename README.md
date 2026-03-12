# Big Picture Project Goal

Each Folder is a seperate "device"

Each "device" contains it's own public and private key

Firmware Devices will each start with a (Device like) CSR and submit it to the CA

The CA will sign one, reject one, and the final one will sign it's own (Device like) CSR

Then each device will try and connect to the Power hyperviosr 

The Power hyperviosr will only allow a connection if the trusted CA public key(s) it has stored can verfiy the signature on the cert of each device


NOTE: Certs for device are differnet then basic certs as they will have optional exteension fields to show Firmware version, Hardware model, date manufactured using (using config file or -addext)

# File Structure

```text
project-root/
├── ca/
├── hypervisor/
├── firmware_devices/
│   ├── nic/            ← possible device types
│   ├── storage/
│   └── bmc/
└── firmware_device/

CA/
├── keys/
│   ├── ca_private.pem     ← CA's private key
│   └── ca_cert.pem        ← CA's public cert
│
├── certs/
│   ├── issued/            ← Successfully signed certs
│   │   ├── device_nic.pem
│   │   └── device_storage.pem
│   ├── rejected/          ← CSRs that failed validation
│       └── device_bmc.csr
│   
│
├── config/
│   ├── policy.conf        ← Validation rules
│   └── ca_openssl.cnf     ← OpenSSL config for signing / criteria
│
├── database/              ← LIKELY NOT ADDING THIS Track issued certs
│   └── authorized_devices.txt  ← Valid serial numbers
│
├── src/
│   ├── main.c
│   ├── ca_functions.c     ← Sign, validate, reject logic
│   └── ca_functions.h
│
└── README.md              ← How CA works

Firmware_Device/
├── keys/
│   ├── device_private.pem   ← Device's private key
│   └── device_public.pem    ← Extracted public key 
│
├── certs/
│   ├── device.csr           ← CSR sent to CA
│   └── device_cert.pem      ← Cert received from CA
│
├── config/
│   └── device_info.cnf      ← Serial#, model, firmware version
│
├── src/
│   ├── main.c               ← Generate CSR, try connect to hypervisor
│   ├── device_functions.c
│   └── device_functions.h
│
└── README.md       

Hypervisor/
├── trusted_ca/
│   └── ca_cert.pem          ← CA public cert (trust anchor)
│
├── config/
│   └── trust_policy.conf    ← Which CAs public keys to trust
│
├── POSSIBLE IDEA logs/
│   ├── connections.log      ← All connection attempts
│   └── rejections.log       ← Failed validations
│
├── src/
│   ├── main.c               ← Validate certs, allow/deny
│   ├── validator.c          ← Certificate validation logic
│   └── validator.h
│
└── README.md



