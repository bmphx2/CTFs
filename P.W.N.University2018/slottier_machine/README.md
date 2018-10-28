Disclaimer: I wasn't able to recover the flag from the target server, however the exploit worked locally and that matters to me. 

It's a x64 binary file:

slottiermachine: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 3.2.0, BuildID[sha1]=9c102a48d0082f6109f20a8015bb9eab04c82e8c, not stripped

The application already leaks the system() address on the function.

You have 7 coins for creating(malloc)/deleting(free) and editing using read().



