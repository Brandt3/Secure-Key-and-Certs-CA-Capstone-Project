# Big Picture Project Goal

Each Folder is a seperate "device"

Each "device" contains it's own public and private key

Firmware Devices will each start with a (Device like) CSR and submit it to the CA

The CA will sign one, reject one, and the final one will sign it's own (Device like) CSR

Then each device will try and connect to the Power hyperviosr 

The Power hyperviosr will only allow a connection if the trusted CA public key(s) it has stored can verfiy the signature on the cert of each device

## Big Learning Focuses

1. Working on creating my own CA within a system and only signing certs that meet customized requirements. This allows for creating a personallized CA that can be used for only signing specific requirments and then on your network only allow conenction to device with your personal CA signature on certs. **For CA currenlty this project will check the serial number, model, and firmware Version from the CSR and that the device sending the CSR has the private key to it based off the public key**

2. Showing the SSL layer and how a connection between Firmware devices is not allowed until a trust relationship is ecured through a cert that is signed by a trusted CA. For the project this is only allowing device to connect to the Power Hypervisor that are signed by my custome CA. This allows for a secure system where only device matches personal criteria can be trusted and create a connections on the SSL layer to communicate.

3. Proper storage of keys and certs keeping them seperate and not hard coding secerts but calling from a secure location and return them after using them. **Possibly later on also focusing on using an emulated TPM (Trusted Platform Module) to store the CA trusted cert/public key and the private key for the Power Hypervisor**



NOTE: Certs for device are differnet then basic certs as they will have optional extension fields to show Firmware version, Hardware model, date manufactured using (using config file or -addext)

## Lessons Learned
* Understanding concepts of what you want you project to do before starting to code helps tremndously and saves you a lot of time
* Laying out a file structure visually on a white board or paper allows for a better understanding of the flow of the project going forward. And helps keep structure and organization once you start coding
* For each file taking time to break down the functions of the file into smaller steps allows for a smoothier more orgnaized coding process and makes it easier to know the next step
* Taking breaks such as a walk is a good way to step back and anaylize what need to be accomplished next. Especially helpful when stuck on a bug for awhile
* Creating functions of code that is redudant early on will save you lots of time down the road (I did not do this at first and left myself with a lot of time spent cleaning up code)
* With OpenSSL and C there are a lot of simple checks that can save you a lot of debugging time by including checking if a value is NULL after a function returns to it
* Clear debugging statements was very helpful and saved me a lot of time when bugs appeared (For me this was clear print statements for specifc checks) 

# File Structure

```text
project-root/
├── ca/
├── hypervisor/
└── firmware_devices/
    ├── nic/            ← possible device types
    ├── storage/
    └── bmc/


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
├── LIKELY NOT ADDING THIS database/              ←  Track issued certs
│   └── authorized_devices.txt  ← Valid serial numbers
│
├── src/
│   ├── main.c
│   ├── ca_functions.c     ← Sign, validate, reject logic
│   └── ca_functions.h
│
└── README.md              ← How CA works

Firmware_Device/
├── keys/                    ← Upon powering up these device will create the keys (Normally would have them prestored)
│   └── device_private.pem   ← Device's private key
│   
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
│   └── ca_cert.pem          ← CA public cert (trust anchor) / contains CA public key for verification
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



