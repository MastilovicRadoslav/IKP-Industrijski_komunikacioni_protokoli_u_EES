#pragma once
// C program for generic linked list 
#include<stdio.h> 
#include<stdlib.h> 
#include<windows.h>

#define SAFE_DELETE_HANDLE(a)  if(a){CloseHandle(a);}

/// <summary>
/// A linked list node
/// </summary>
// Èvor jednostruko spregnute liste
typedef struct Node
{
    void* data;
    HANDLE mutex;
    struct Node* next;
}NODE;

/// <summary>
/// Function for initialises the head of the list
/// </summary>
// Funkcija za inicijalizaciju liste
void InitGenericList(NODE** head);

/// <summary>
/// Function to add a node at the beginning of Linked List
/// This function expects a pointer to the data to be added and size of the data type
/// </summary>
// Funkcija za dodavanje na poèetak liste
void GenericListPushAtStart(NODE** head_ref, void* new_data, size_t data_size);

/// <summary>
/// Function to print nodes in a given linked list. 'fpitr' is used to access the function to be used for printing current node data
/// </summary>
// Funkcija za ispis èvora liste
void PrintGenericList(NODE* node, void (*fptr)(void*));

/// <summary>
/// Function to free all the memory taken by list
/// </summary>
// Funkcija za oslobaðanje memorije koju je zauzela lista
void FreeGenericList(NODE** head);

/// <summary>
/// Function to delete a node from the list
/// </summary>
// Funkcija za brisanje èvora iz liste
bool DeleteNode(NODE** head, void* toDelete, size_t size);