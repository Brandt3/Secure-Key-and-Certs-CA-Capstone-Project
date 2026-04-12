# Firmware Device development process

In a real situation Steps 1 and 2 already have the info stored on a secure flash (created at manufacturing) 
    and step 2 is a presigned cert and not a csr. 

***Note keys are NEVER regenerated each time power is on***

1. Upon "Power On" Check if there are keys created if not Create Private and Public key (RSA or EC)
    * Store Key in the keys file
    * Note Keys persist across reboots (try to emulate that here as well so only create the keys once)

2. Create CSR and request user for CSR info plus exstention
    * Store CSR in certs file
    * Input all CSR info required / could also pull info from a config file
    * Real world the device read this info from a NVRAM/secure storage

3. Send CSR to CA to see if it will get signed
    * If it does store cert in certs file and delete CSR
    * If reject display message / echo "Device number could not get signed as it does not meet requirements therefore it can not try and connect to the Power Hypervisor"

4. If it has a signed cert send it to the Power Hypervisor to try and connect

5. If cert is trusted then perform a task or communicate with the Power Hypervisor
    