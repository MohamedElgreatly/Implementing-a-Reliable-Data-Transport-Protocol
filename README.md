# Implementing-a-Reliable-Data-Transport-Protocol
Implementing a Reliable Data Transport Protocol  (MohamedElgreatly) - (Eng-Ayman-Mohamed).

* In this project we implement Reliable-Data-Transport-Protocol over UDP protocols to transfer data.

Implementation will run in a simulated environment, this means that we will simulate the packet loss probability (PLP) since packet loss is infrequent in a localhost or LAN environment.
* PLP ranges from 0 to 1. For PLP=0 your implementation should behave with no packet loss. For nonzero values of PLP, you should simulate packet loss (by dropping datagrams given as parameters to the send() method) with the corresponding probability - i.e. a datagram given as a parameter to the send() method is transmitted only 100*(1 - PLP)% of the time (A value of 0.1 would mean that one in ten packets (on average) are lost).
