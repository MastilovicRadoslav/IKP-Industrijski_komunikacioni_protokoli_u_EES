#include "pch.h"
#include "framework.h"

#pragma once

// Enumeration for signal
// Enumeracija (nabrajanje) za signal
typedef enum MeasurmentTopic {analog = 0, status}Topic;

// Enumeration for type
// Enumeracija (nabrajanje) za type
typedef enum MeasurmentType {fuse = 0, breaker, sec_A, sec_V}Type;

typedef struct _msgFormat {
    Topic signal;
    Type type;
    unsigned int num;
    double value;
}Measurment;

// Get string representation from enum object
// Funkcija za konverziju enumeracije u string
const char* GetStringFromEnumHelper(Topic topic);
const char* GetStringFromEnumHelper(Type type);

// Print measurment struct, specifically to be used by the generic list
// Funkcija za ispis merenja
void PrintMeasurment(Measurment* m);