#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../Common/ThreadSafeGenericList.h";

/// <summary>
/// Function for initialises the head of the list
/// </summary>
// Funkcija za inicijalizaciju liste
void InitGenericList(NODE** head) {
    *head = NULL;
    (*head)->mutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex
}

/// <summary>
/// Function to add a node at the beginning of Linked List
/// This function expects a pointer to the data to be added and size of the data type
/// </summary>
// Funkcija za dodavanje na poèetak liste
void GenericListPushAtStart(NODE** head_ref, void* new_data, size_t data_size)
{
    // Allocate memory for node 
    NODE* new_node = (NODE*)malloc(sizeof(NODE));

    new_node->data = malloc(data_size);
    new_node->next = (*head_ref);
    new_node->mutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex

    // Copy contents of new_data to newly allocated memory. 
    memcpy(new_node->data, new_data, data_size);

    // Change head pointer as new node is added at the beginning 
    (*head_ref) = new_node;
}

/// <summary>
/// Function to print nodes in a given linked list. 'fpitr' is used to access the function to be used for printing current node data
/// </summary>
// Funkcija za ispis èvora liste
void PrintGenericList(NODE* node, void (*fptr)(void*))
{
    DWORD dwWaitResult;
    while (node != NULL)
    {
        dwWaitResult = WaitForSingleObject(
            node->mutex,    // handle to mutex
            INFINITE);  // no time-out interval
        if (dwWaitResult == WAIT_OBJECT_0) {  // The thread got ownership of the mutex
            (*fptr)(node->data);
            node = node->next;
            ReleaseMutex(node->mutex);
        }
    }
}

/// <summary>
/// Function to free all the memory taken by list
/// </summary>
// Funkcija za oslobaðanje zauzete memorije
void FreeGenericList(NODE** head) {
    if (*head == NULL) {
        return;
    }

    NODE* next = (*head)->next;
    DWORD dwWaitResult = WaitForSingleObject(
        (*head)->mutex,    // handle to mutex
        INFINITE);  // no time-out interval
    if (dwWaitResult == WAIT_OBJECT_0) {
        CloseHandle((*head)->mutex);
        free((*head)->data);
        (*head)->data = NULL;
        (*head)->next = NULL;
        free(*head);
        *head = NULL;
    }

    if (next != NULL) {
        FreeGenericList(&next);
    }
}

/// <summary>
/// Function to delete a node from the list
/// </summary>
// Funkcija za brisanje èvora iz liste
bool DeleteNode(NODE** head, void* toDelete, size_t size) {
    NODE* current = *head;

    if (current == NULL) {
        return false;
    }

    NODE* prev = NULL;
    char* target = (char*)malloc(size);
    memcpy(target, toDelete, size);
    char* data = (char*)malloc(size);

    while (current != NULL) {
        DWORD dwWaitResult = WaitForSingleObject(
            (current)->mutex,    // handle to mutex
            INFINITE);  // no time-out interval
        if (dwWaitResult == WAIT_OBJECT_0) {
            memcpy(data, current->data, size);
            if (*target == *data) {
                if (prev == NULL) {
                    (*head) = current->next;
                }
                else {
                    prev->next = current->next;
                }
                free(current->data);
                free(current);
                free(data);
                free(target);
                return true;
            }
            ReleaseMutex(current->mutex);
            CloseHandle(current->mutex);
            prev = current;
            current = current->next;
        }
    }
    free(target);
    free(data);
    return false;
}