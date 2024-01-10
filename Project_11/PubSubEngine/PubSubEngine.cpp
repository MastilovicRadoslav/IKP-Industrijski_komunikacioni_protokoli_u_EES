#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../Common/ThreadSafeGenericList.h";
#include "../Common/ThreadSafeGenericList.cpp";
#include "../Common/Measurment.h";
#include "../Common/Measurment.cpp";
#include "../TCPLib/TCPLib.cpp";

#pragma comment(lib,"WS2_32")

#define DEFAULT_BUFLEN 1000
#define DEFAULT_PORT "27016"
#define MAX_CLIENTS 10
#define TIMEVAL_SEC 0
#define TIMEVAL_USEC 0

#define SAFE_DELETE_HANDLE(a)  if(a){CloseHandle(a);}

bool InitializeWindowsSockets();
bool InitializeListenSocket();
bool BindListenSocket();
void SetNonBlocking();
int Init();
DWORD WINAPI Listen(LPVOID);
void SetAcceptedSocketsInvalid();
void ProcessMessages();
void ProcessMeasurment(Measurment*);
void Shutdown();
void UpdateSubscribers(Measurment*, NODE *);
void SendToNewSubscriber(SOCKET, NODE*);
bool InitCriticalSections();

fd_set readfds;
SOCKET listenSocket = INVALID_SOCKET;
SOCKET acceptedSockets[MAX_CLIENTS];
addrinfo* resultingAddress = NULL;
timeval timeVal;

CRITICAL_SECTION CSAnalogData;
CRITICAL_SECTION CSStatusData;
CRITICAL_SECTION CSAnalogSubs;
CRITICAL_SECTION CSStatusSubs;

NODE *publisherList = NULL;
NODE *subscriberList = NULL;
NODE *statusData = NULL;
NODE *analogData = NULL;
NODE *statusSubscribers = NULL;
NODE *analogSubscribers = NULL;

HANDLE listenHandle;

int main()
{
    DWORD listenID;
    int result = Init();

    if (result) {
        printf("ERROR CODE %d, press any key to exit\n", result);
        getchar();
        return result;
    }

    printf("Server live and ready to listen\n");
    listenHandle = CreateThread(NULL, 0, &Listen, (LPVOID)0, 0, &listenID);

    if (listenHandle) {
        WaitForSingleObject(listenHandle, INFINITE);
    }
    getchar();
    Shutdown();
}

/// <summary>
/// Call function InitializeWindowsSockets(), InitializeListenSocket(), BindListenSocket(), InitCriticalSections() to initialize TCP communication
/// </summary>
// Funkcija za inicijalizaciju TCP komunikacije
int Init() {
    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }
    if (InitializeListenSocket() == false) {
        return 2;
    }
    if (BindListenSocket() == false) {
        return 3;
    }
    if (InitCriticalSections() == false) {
        return 4;
    }
    freeaddrinfo(resultingAddress);
    SetNonBlocking();
    SetAcceptedSocketsInvalid();
    return 0;
}

/// <summary>
/// Calls InitializeCriticalSection(...) for every CRITICAL_SECTION handle.
/// </summary>
// Funkcija za inicijalizaciju TCP komunikacije
bool InitCriticalSections() {
    InitializeCriticalSection(&CSAnalogData);
    InitializeCriticalSection(&CSStatusData);
    InitializeCriticalSection(&CSAnalogSubs);
    InitializeCriticalSection(&CSStatusSubs);
    return true;
}

/// <summary>
/// After the serice is initialised, enter the listening state.
/// </summary>
// Funkcija za podešavanje socketa za slušanje
DWORD WINAPI Listen(LPVOID param) {
    int iResult = listen(listenSocket, SOMAXCONN);

    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        Shutdown();
        return 0;
    }

    printf("Listening...\n");

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(listenSocket, &readfds);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (acceptedSockets[i] != INVALID_SOCKET)
                FD_SET(acceptedSockets[i], &readfds);
        }

        int value = select(0, &readfds, NULL, NULL, &timeVal);

        if (value == 0) {
        }
        else if (value == SOCKET_ERROR) {
            printf("select failed with error: %d\n", WSAGetLastError());
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (FD_ISSET(acceptedSockets[i], &readfds)) {
                    closesocket(acceptedSockets[i]);
                    acceptedSockets[i] = INVALID_SOCKET;
                }
            }
        }
        else {
            if (FD_ISSET(listenSocket, &readfds)) {
                int  i;
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (acceptedSockets[i] == INVALID_SOCKET) {
                        acceptedSockets[i] = accept(listenSocket, NULL, NULL);
                        if (acceptedSockets[i] == INVALID_SOCKET)
                        {
                            printf("accept failed with error: %d\n", WSAGetLastError());
                            closesocket(acceptedSockets[i]);
                            acceptedSockets[i] = INVALID_SOCKET;
                            closesocket(listenSocket);
                            WSACleanup();
                            return 0;
                        }

                        break;
                    }
                }
                if (i == MAX_CLIENTS) {
                }
                
            }
            else {
                ProcessMessages();
            }
        }
    }
}

/// <summary>
/// Set up the socket to accept non-blocking TCP mode.
/// </summary>
// Funkcija za podeèavanje neblokirajuæeg moda za socket
void SetNonBlocking() {
    unsigned long mode = 1;
    int iResult = ioctlsocket(listenSocket, FIONBIO, &mode);
    FD_ZERO(&readfds);
    FD_SET(listenSocket, &readfds);
    timeVal.tv_sec = TIMEVAL_SEC;
    timeVal.tv_usec = TIMEVAL_USEC;
}

/// <summary>
/// Initiates use of the Winsock DLL by a process.
/// </summary>
// Funkcija za inicijalizaciju Winsocka
bool InitializeWindowsSockets()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

/// <summary>
/// Sets up listening socket.
/// </summary>
// Funkcija podešavanje slušajuæeg socketa
bool InitializeListenSocket() {
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    int iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);

    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return false;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
        return false;
    }
    return true;
}

/// <summary>
/// Binds listening socket with the configured address and port.
/// </summary>
// Funkcija za povezivanje slušajuæeg socketa na adresu i port
bool BindListenSocket() {
    int iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);

    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }
    return true;
}

/// <summary>
/// Sets up accepted sockets to default (empty) value.
/// </summary>
// Funkcija za podešavanje socketa za prihvatanje na poèetnu vrednost
void SetAcceptedSocketsInvalid() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        acceptedSockets[i] = INVALID_SOCKET;
    }
}

/// <summary>
/// Checks accepted sockets that raised an event and calls TCPLib.TCPReceiveMeasurment to processes the message.
/// Fills publishers and subrscribers list for introducment messages and calls ProccessMeasurment for processing data.
/// </summary>
// Funkcija za rad sa klijentima (subscriber i publisher)
void ProcessMessages() {
    char* data = (char*)malloc(sizeof(Measurment));

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (FD_ISSET(acceptedSockets[i], &readfds)) {
            bool succes = TCPReceive(acceptedSockets[i], data, sizeof(Measurment));

            if (!succes) {
                EnterCriticalSection(&CSAnalogSubs);
                DeleteNode(&analogSubscribers, &acceptedSockets[i], sizeof(SOCKET));
                LeaveCriticalSection(&CSAnalogSubs);

                EnterCriticalSection(&CSStatusSubs);
                DeleteNode(&statusSubscribers, &acceptedSockets[i], sizeof(SOCKET));
                LeaveCriticalSection(&CSStatusSubs);

                closesocket(acceptedSockets[i]);
                acceptedSockets[i] = INVALID_SOCKET;
                continue;
            }

            SOCKET* ptr = &acceptedSockets[i];

            if (data[0] == 'p') {
                printf("Connected client: publisher\n");
            }
            else if (data[0] == 'b') {
                printf("Connected client: subscriber\n");
            }
            else if (data[0] == 'a') {
                EnterCriticalSection(&CSAnalogSubs);
                GenericListPushAtStart(&analogSubscribers, ptr, sizeof(SOCKET));
                SendToNewSubscriber(acceptedSockets[i], analogData);
                LeaveCriticalSection(&CSAnalogSubs);
            }
            else if (data[0] == 's') {
                EnterCriticalSection(&CSStatusSubs);
                GenericListPushAtStart(&statusSubscribers, ptr, sizeof(SOCKET));
                SendToNewSubscriber(acceptedSockets[i], statusData);
                LeaveCriticalSection(&CSStatusSubs);
            }
            else {
                Measurment* newMeasurment = (Measurment*)malloc(sizeof(Measurment));
                memcpy(newMeasurment, data, sizeof(Measurment));
                ProcessMeasurment(newMeasurment);
                free(newMeasurment);
            }
        }
    }
    free(data);
}

/// <summary>
/// Save the measurment in the list corresponding to its type.
/// m = Measurment to be saved (by reference)
/// </summary>
// Funkcija za dodavanje merenja u listu
void ProcessMeasurment(Measurment *m) {
    printf("[DEBUG] Service received: ");
    PrintMeasurment(m);

    switch (m->signal)
    {
    case analog:
        EnterCriticalSection(&CSAnalogData);
        GenericListPushAtStart(&analogData, m, sizeof(Measurment));
        UpdateSubscribers(m, analogSubscribers);
        LeaveCriticalSection(&CSAnalogData);
        break;
    case status:
        EnterCriticalSection(&CSStatusData);
        GenericListPushAtStart(&statusData, m, sizeof(Measurment));
        UpdateSubscribers(m, statusSubscribers);
        LeaveCriticalSection(&CSStatusData);
        break;
    default:
        printf("[ERROR] Topic %d not supported.", m->signal);
        break;
    }
}

/// <summary>
/// Frees program memory, handles and performs WSA shutdown logic.
/// </summary>
// Funkcija za zatvaranje komunikacije i oslobaðanje memorije
void Shutdown() {
    closesocket(listenSocket);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        closesocket(acceptedSockets[i]);
    }

    WSACleanup();
    FreeGenericList(&publisherList);
    FreeGenericList(&subscriberList);
    FreeGenericList(&statusData);
    FreeGenericList(&analogData);
    FreeGenericList(&statusSubscribers);
    FreeGenericList(&analogSubscribers);

    SAFE_DELETE_HANDLE(listenHandle);
    
    DeleteCriticalSection(&CSStatusData);
    DeleteCriticalSection(&CSAnalogData);
    DeleteCriticalSection(&CSStatusSubs);
    DeleteCriticalSection(&CSAnalogSubs);

    printf("Service freed all memory.");
    getchar();
}

/// <summary>
/// After a new measurment arrives, send it to all the subscribers that all subscribed to that topic.
/// </summary>
// Funkcija za ažuriranje subcribera
void UpdateSubscribers(Measurment* m, Node *list) {
    Node* temp = list;

    if (temp == NULL) {
        return;
    }

    while (temp != NULL) {
        SOCKET s;
        memcpy(&s, temp->data, sizeof(SOCKET));
        TCPSend(s, *m);
        temp = temp->next;
    }
}

/// <summary>
/// After a Subscriber subscribes to a new topic, send all saved data of the said topic to him.
/// </summary>
// Funkcija za slanje podataka novom subcriberu
void SendToNewSubscriber(SOCKET sub, NODE *dataHead) {
    Node* temp = dataHead;

    if (temp == NULL) {
        return;
    }

    Measurment* data = (Measurment*)malloc(sizeof(Measurment));

    while (temp != NULL) {
        memcpy(data, temp->data, sizeof(Measurment));
        TCPSend(sub, *data);
        temp = temp->next;
    }
    free(data);
}