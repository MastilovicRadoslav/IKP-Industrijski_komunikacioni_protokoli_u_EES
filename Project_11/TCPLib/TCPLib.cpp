#pragma once

#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "pch.h"
#include "framework.h"

#include "../Common/Measurment.h";
#include "../TCPLib/TCPLib.h";

#pragma comment(lib,"WS2_32")

/// <summary>
/// Sends a measurment packet trough the socket using TCP protocol.
/// </summary>
// Funkcija za slanje merenja putem TCP konekcije
bool TCPSend(SOCKET connectSocket, Measurment measurment) {
    char* data = (char*)malloc(sizeof(Measurment));
    memcpy(data, (const void *)&measurment, sizeof(Measurment));
    int iResult = send(connectSocket, data, sizeof(Measurment), 0);
    free(data);

    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return false;
    }
    return true;
}

/// <summary>
/// Used for sending keys of introduction or any kind of string data trough the socket using TCP protocol.
/// </summary>
// Funkcija u kojoj se šalje tip korisnika (subscriber (b) ili publisher (p))
bool TCPSend(SOCKET connectSocket, char *key) {
    int iResult = send(connectSocket, (const char*)key, sizeof(char), 0);

    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return false;
    }
    return true;
}

/// <summary>
/// Function for receiving packets from a specified socket.
/// </summary>
// Funkcija koja služi samo da pokaže da li je prihvaæeno merenje ili nije
bool TCPReceive(SOCKET connectSocket, char* recvbuf, size_t len) {
    int iResult = recv(connectSocket, (char*)recvbuf, len, 0);

    if (iResult > 0)
    {
    }
    else
    {
        return false;
    }
    return true;
}