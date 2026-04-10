# Simulating a Server

## Goal
The goal of the Power_Hyperviosr is to always be listening for socket connections and then once accetped perform SSL/TLS checks to ensure it is a trusted device before it can perform any actions on the server. 

## Simulate
This server simulates a hypervisor authentication interface. This means as a hypervisor authentication it should always be running providing connection checks. Therefore this file "server" will continuously be in a while loop and uses a sigaction. So when you Ctrl+C in the terminal instead of killing the program and not cleaning it up properly. It will flip a flag value which will end the loop next time it gets to the top and then perform proper clean up and closing of the sockets.

## Accomplish
This will be my own personal version mTLS made from scratch 

## TLS Certificate Validation
The server will check these areas before determining if the device can be trusted

- Verify trusted CA signature  
- Check expiration  
- Check CN/SN against DB (This check is already done by the CA)
