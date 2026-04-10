# Simulating a Server

## Goal
The goal of the Power_Hyperviosr is to always be listening for socket connections and then once accetped perform SSL/TLS checks to ensure it is a trusted device before it can perform any actions on the server. 

## Accomplish
This will be my own personal version mTLS made from scratch 

## Validation
The server will check these areas before determining if the device can be trusted

- Verify CA signature  
- Check expiration  
- Check CN/SN against DB  
- Verify public key matches