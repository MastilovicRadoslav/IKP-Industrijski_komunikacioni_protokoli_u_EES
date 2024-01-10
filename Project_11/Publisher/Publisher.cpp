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
#include <time.h>

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
#define RAND_MAX 100

int Init();
bool InitializeWindowsSockets();
bool CreateSocket();
bool Connect();
Measurment *GenerateMeasurment();
Measurment *CreateMeasurment();
bool IntroduceMyself();
void Publish(int publishingType, int interval);

SOCKET connectSocket = INVALID_SOCKET;
sockaddr_in serverAddress;

int main()
{
    srand(time(NULL));
    int result = Init();

    if (result) {
        printf("ERROR CODE %d, press any key to exit\n", result);
        getchar();
        return result;
    }

    printf("Client initialised.");
    int picked = 0;
    int interval = 1000;

    do {
        printf("Pick a way of publishing:\n1)Randomly generated message\n2)User created message\n");

        if (scanf("%d", &picked) != 1) {
            printf("Invalid input. Please enter a number.\n");

            while (getchar() != '\n');
            continue;
        }

        if (picked == 1) {
            printf("Please pick an interval of publishing in ms: \n");

            if (scanf("%d", &interval) != 1) {
                printf("Invalid input. Please enter a number for the interval.\n");

                while (getchar() != '\n');
                continue;
            }
        }
    } while (picked != 1 && picked != 2);
    getchar();
    Publish(picked, interval);
    getchar();
}

/// <summary>
/// Function that sends randomly generated Measurment every 1001 ms by using a sending function from TCPLib.
/// </summary>
// Funkcija koja šalje podatke na server
void Publish(int publishingType, int interval) {
    while (true) {
        printf("Sending...\n");
        Measurment* m = (Measurment *)malloc(sizeof(Measurment));

        switch (publishingType)
        {
        case 1:
             m = GenerateMeasurment();
             break;
        case 2:
             m = CreateMeasurment();
             break;
        default:
            break;
        }

        if (TCPSend(connectSocket, *m)) {
            printf("Sent: %s %s %d %.2f\n", GetStringFromEnumHelper(m->signal), GetStringFromEnumHelper(m->type), m->num, m->value);
        }
        else {
            printf("An error occured\n");
        }
        free(m);
        Sleep(1000);
    }
}

/// <summary>
/// Generates a randomly valued Measurment structure.
/// </summary>
// Funkcija koja generiše random merenja, koristi se za stres test
Measurment * GenerateMeasurment() {
    Measurment* msg = (Measurment*)malloc(sizeof(Measurment));
    enum MeasurmentTopic a = analog;
    msg->num = (rand() % 100) + 1;
    int signal = (rand() % 2);
    int type = (rand() % 2);

    switch (signal) {
        case 0: 
            msg->signal = analog;
            msg->num = (rand() % 100);
            msg->value = (rand() % 100) + ((double)rand() / RAND_MAX);
            break;
        case 1: 
            msg->signal = status;
            msg->type = fuse;
            msg->num = (rand() % 5);
            msg->value = (rand() % 5);
            break;
        case 2:
            msg->signal = status;
            msg->type = breaker;
            msg->num = (rand() % 5);
            msg->value = (rand() % 5);
            break;
    }
    if (msg->signal == analog) {
        switch (type)
        {
        case 0:
            msg->type = sec_A;
            break;
        case 1:
            msg->type = sec_V;
            break;
        }
    }
    return msg;
}

/// <summary>
/// Creates a Measurment structure by users input.
/// </summary>
// Funkcija u kojoj korisnik unosi merenje
Measurment* CreateMeasurment() {
    Measurment* msg = (Measurment*)malloc(sizeof(Measurment));

    do {
        printf("Enter signal (analog or status): ");
        char signal[20];
        scanf("%s", signal);

        if (strcmp(signal, "analog") == 0 || strcmp(signal, "status") == 0) {
            if (strcmp(signal, "analog") == 0) {
                msg->signal = analog;
            }
            else {
                msg->signal = status;
            }
            break;
        }
        else {
            printf("Invalid signal. Please enter 'analog' or 'status'.\n");
        }
    } while (1);

    do {
        printf("Enter type (fuse, breaker, sec_A or sec_V): ");
        char type[20];
        scanf("%s", type);

        if ((msg->signal == analog && (strcmp(type, "sec_A") == 0 || strcmp(type, "sec_V") == 0)) ||
            (msg->signal == status && (strcmp(type, "breaker") == 0 || strcmp(type, "fuse") == 0))) {
            if (strcmp(type, "fuse") == 0) {
                msg->type = fuse;
            }
            else if (strcmp(type, "sec_A") == 0) {
                msg->type = sec_A;
            }
            else if (strcmp(type, "sec_V") == 0) {
                msg->type = sec_V;
            }
            else if (strcmp(type, "breaker") == 0) {
                msg->type = breaker;
            }
            break;
        }
        else {
            printf("Invalid type. Please enter a valid type.\n");
        }
    } while (1);

    do {
        printf("Enter ordinal number of the device: ");
        int val = 0;

        if (scanf("%d", &val) == 1 && val >= 0) {
            msg->num = val;
            break;
        }
        else {
            printf("Invalid num. Please enter a valid non-negative integer.\n");
            while (getchar() != '\n');
        }
    } while (1);

    do {
        printf("Enter measured value: ");
        double val = 0.0;

        if (scanf("%lf", &val) == 1 && val >= 0.0) {
            if ((int)val == val) {
                int intVal = (int)val;
                if (intVal == 0 || intVal == 1) {
                    msg->value = val;
                    break;
                }
                else {
                    printf("Invalid integer. Please enter 0 or 1.\n");
                }
            }
            else {
                msg->value = val;
                break;
            }
        }
        else {
            printf("Invalid input. Please enter a valid non-negative number.\n");
            while (getchar() != '\n');
        }
    } while (1);
    return msg;
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
// Funkcija kojoj se klijent predstavlja kao publisher
bool IntroduceMyself() {
    char introducment[2] = "p";
    return TCPSend(connectSocket,introducment);
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