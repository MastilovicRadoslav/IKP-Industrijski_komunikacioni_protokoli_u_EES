#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "../Common/Measurment.h";
#include "../Common/Measurment.cpp";
#include "../TCPLib/TCPLib.h";
#include "../TCPLib/TCPLib.cpp";

#pragma comment(lib,"WS2_32")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016
#define ADDRESS "127.0.0.1"
#define TYPE_STRING_LENGHT 10
#define DEMOTESTCOUNT 20

int Init();
bool InitializeWindowsSockets();
bool CreateSocket();
bool Connect();
bool IntroduceMyself();
void Subscribe();
void StartRecieveThread();
bool Validate(Measurment *);

SOCKET connectSocket = INVALID_SOCKET;
sockaddr_in serverAddress;

/// <summary>
/// Recieves Measurment structure from the PubSubEngine. Uses a function from TCPLib to recieve a packet.
/// </summary>
// Funkcija koja je parametar niti i služi da prihvati poruke od servera
DWORD WINAPI Receive(LPVOID param) {
    char* data = (char*)malloc(sizeof(Measurment));

    while (true) {
        TCPReceive(connectSocket, data, sizeof(Measurment));
        Measurment* newMeasurment = (Measurment*)malloc(sizeof(Measurment));
        memcpy(newMeasurment, data, sizeof(Measurment));

        if (Validate(newMeasurment)) {
            printf("VALID: ");
        }
        else {
            printf("INVALID: ");
        }
        PrintMeasurment(newMeasurment);
        free(newMeasurment);
        Sleep(10);
    }
}

int main()
{
    int result = Init();

    if (result) {
        printf("ERROR CODE %d, press any key to exit\n", result);
        getchar();
        return result;
    }
    printf("Client initialised.\n");
    Subscribe();
    printf("Client subscribed.\n");
    StartRecieveThread();
    getchar();
}

/// <summary>
/// Validates recieved measurment packet. If the signal is Analog, the num can be anything greater than zero. 
/// If the signal is Status, the num can be 0 or 1 (digital).
/// </summary>
// Funkcija koja proverava validnost primljene poruke
bool Validate(Measurment *m) {
    if (m->signal == analog && m->value >= 0) {
        return true;
    }
    else if (m->signal == status && (m->value == 0 || m->value == 1)) {
        return true;
    }
    return false;
}

/// <summary>
/// Creates and starts the thread for the Receive function.
/// </summary>
// Funkcija koja pokreæe nit za prihvatanje poruka
void StartRecieveThread() {
    DWORD id, param = 1;
    HANDLE handle;
    handle = CreateThread(
        NULL, // default security attributes
        0, // use default stack size
        Receive, // thread function
        &param, // argument to thread function
        0, // use default creation flags
        &id); // returns the thread identifier
    int liI = _getch();
    CloseHandle(handle);
}

/// <summary>
/// Gives user the menu for picking subscription topics. Based on users input, uses a sending function from TCPLib to 
/// tell PubSubEngine what signal client has subscribed to.
/// </summary>
// Funkcija za odabir tipa poruke koju klijent želi da primi
void Subscribe() {
    char c;
    char t1[2] = "s";
    char t2[2] = "a";
    printf("1) Status\n2) Analog\n3) Both\nSelect: ");

    while (1) {
        c = getchar();

        if (c == '1' || c == '2' || c == '3') {
            break;
        }
        else {
            printf("\nBad input.\n1) Status\n2) Analog\n3) Both\nSelect: ");
            while (getchar() != '\n');
        }
    }

    if (c == '1') {
        TCPSend(connectSocket, t1);
    }
    else if (c == '2') {
        TCPSend(connectSocket, t2);
    }
    else if (c == '3') {
        TCPSend(connectSocket, t1);
        TCPSend(connectSocket, t2);
    }
}

/// <summary>
/// Initialises the client with functions InitializeWindowsSockets(), CreateSocket() and Connect().
/// Calls IntroduceMyself() to tell PubSubEngine what type of a client it is.
/// </summary>
// Funkcija za inicijalizaciju TCP komunikacije
int Init() {
    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }
    if (CreateSocket() == false) {
        return 2;
    }
    if (Connect() == false) {
        return 3;
    }
    if (IntroduceMyself() == false) {
        return 4;
    }
    return 0;
}

/// <summary>
/// Function that uses a sending function from TCPLib to introduce himself as a Subscriber.
/// </summary>
// Funkcija kojoj se klijent predstavlja kao subscriber
bool IntroduceMyself() {
    char introducment[2] = "b";
    return TCPSend(connectSocket, introducment);
}

/// <summary>
/// Connects the client socket to the server.
/// </summary>
// Funkcija za konekciju na server putem TCP protokola
bool Connect() {
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ADDRESS);
    serverAddress.sin_port = htons(DEFAULT_PORT);

    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
        return false;
    }
    return true;
}

/// <summary>
/// Creates and inits the clients socket.
/// </summary>
// Funkcija za kreiranje socketa
bool CreateSocket() {
    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return false;
    }
    return true;
}

/// <summary>
/// Initiates use of the Winsock DLL by a process.
/// </summary>
// Funkcija za inicijalizaciju Winsocka
bool InitializeWindowsSockets() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}