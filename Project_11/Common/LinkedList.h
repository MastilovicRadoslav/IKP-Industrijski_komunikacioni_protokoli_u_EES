#pragma once

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "pch.h"
#include "framework.h"
#include "Measurment.h"

/// <summary>
/// Linked List Structure
/// </summary>
// Struktura jednostruko spregnute liste
typedef struct linked_list {
    struct linked_list* next;
    Measurment value;
}LIST;

/// <summary>
/// List initialization function
/// </summary>
// Funkcija za inicijalizaciju liste
void init_list(LIST** head) {
    *head = NULL;
}

/// <summary>
/// Function for adding an element to the beginning of a list
/// </summary>
// Funkcija za dodavanje elementa na poèetak liste
void LISTInputElementAtStart(struct linked_list** head, Measurment value) {
    if (*head == NULL) {
        *head = (struct linked_list*)malloc(sizeof(struct linked_list));
        (*head)->next = NULL;
        (*head)->value = value;
        return;
    }
    struct linked_list* node = (struct linked_list*)malloc(sizeof(struct linked_list));
    node->value = value;
    node->next = *head;
    *head = node;
}

/// <summary>
/// Function for add an element to the end of the list
/// </summary>
// Funkcija za dodavanje elementa na kraj liste
void LISTInputElementAtEnd(struct linked_list** head, Measurment value) {
    if (*head == NULL) {
        *head = (struct linked_list*)malloc(sizeof(struct linked_list));
        (*head)->next = NULL;
        (*head)->value = value;
        return;
    }

    struct linked_list* temp = *head;

    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = (struct linked_list*)malloc(sizeof(struct linked_list));
    temp->next->next = NULL;
    temp->next->value = value;
}

/// <summary>
/// Function for finding value in a list
/// </summary>
// Funkcija za nalaženje vrednosti u listi
int LISTFindValue(struct linked_list* head, int value) {
    while (head != NULL) {
        if (head->value.value == value) {
            return 1;
        }
        head = head->next;
    }
    return 0;
}

/// <summary>
/// List printing function
/// </summary>
// Funkcija za ispis liste
void LISTTraverseAndPrint(struct linked_list* head) {
    int i = 0;

    while (head != NULL) {
        printf("Value %d: %d\n", ++i, head->value.value);
        head = head->next;
    }
}