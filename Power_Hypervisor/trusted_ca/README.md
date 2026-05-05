# Stores Trusted CA Cert

This folder stores the trusted CA's cert to extract the public key and use it in the TLS handshake between each device that tries to connect. 

The cert must manually be copied from the CA's personal cert into this directory. This is not typically how the Power Hypervisor would obtain the CA's cert but it is normally done by the CA cert getting preloaded preloaded by the OS or firmware. So because I do not have the ability to simulate that it must be manually inputted.

Because of this if you wanted to insert a different cert from a different CA that you want to trust instead of the custom created CA you could manfully input any CA cert and the program would then check each devices cert to make sure it is signed by that specific CA.

NOTE: it's important that if you manually input a CA cert the file name must be
`ca_cert.crt`