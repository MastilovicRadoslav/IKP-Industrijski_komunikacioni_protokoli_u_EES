#ifndef TCP_HEADER
#define TCP_HEADER "TCPLib.h"

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "../Common/Measurment.h";

// Function for sending measurements via TCP connection
// Funkcija za slanje merenja putem TCP konekcije
bool TCPSend(SOCKET connectSocket, Measurment measurment);

// A function in which the user type is sent (subscriber (b) or publisher (p))
// Funkcija u kojoj se šalje tip korisnika (subscriber (b) ili publisher (p))
bool TCPSend(SOCKET connectSocket, char* key);

// A function that serves only to show whether the measurement is accepted or not
// Funkcija koja služi samo da pokaže da li je prihvaæeno merenje ili nije
bool TCPReceive(SOCKET connectSocket, char* recvbuf, size_t len);

#endif