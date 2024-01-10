#pragma once

#include<stdio.h> 
#include<stdlib.h> 

/// <summary>
/// A linked list node
/// </summary>
// Èvor generièke jednostruko spregnute liste
typedef struct Node
{
    void* data;
    struct Node* next;
}NODE;

/// <summary>
/// Initialises the head of the list
/// </summary>
// Funkcija za inicijalizaciju liste
void InitGenericList(NODE** head) {
    *head = NULL;
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

    // Copy contents of new_data to newly allocated memory. 
    memcpy(new_node->data, new_data, data_size);

    // Change head pointer as new node is added at the beginning 
    (*head_ref) = new_node;
}

/// <summary>
/// Function to print nodes in a given linked list. 'fpitr' is used to access the function to be used for printing current node data
/// </summary>
// Funkcija za ispis èvora liste
void PrintGenericList(NODE * node, void (*fptr)(void*))
{
    while (node != NULL)
    {
        (*fptr)(node->data);
        node = node->next;
    }
}

/// <summary>
/// Function to free all the memory taken by list
/// </summary>
// Funkcija za oslobaðanje zauzete memorije
void FreeGenericList(NODE **head) {
    if (*head == NULL) {
        return;
    }

    NODE *next = (*head)->next;
    free((*head)->data);
    (*head)->data = NULL;
    (*head)->next = NULL;
    free(*head);
    *head = NULL;

    if (next != NULL) {
        FreeGenericList(&next);
    }
}

/// <summary>
/// Function to delete a node from the list
/// </summary>
// Funkcija za brisanje èvora iz liste
bool DeleteNode(NODE** head, void* toDelete, size_t size ) {
    NODE* current = *head;

    if (current == NULL) {
        return false;
    }

    NODE* prev = NULL;
    char*  target = (char*)malloc(size);
    memcpy(target, toDelete, size);
    char *data = (char*)malloc(size);

    while (current != NULL) {
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
        prev = current;
        current = current->next;
    }
    free(target);
    free(data);
    return false;
}