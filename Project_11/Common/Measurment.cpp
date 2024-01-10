#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../Common/Measurment.h";

// Get string representation from enum object
// Funkcija za konverziju enumeracije u string
const char* GetStringFromEnumHelper(Topic signal) {
	const char* strings[] = { "analog", "status" };
	return strings[signal];
}

// Get string representation from enum object
// Funkcija za konverziju enumeracije u string
const char* GetStringFromEnumHelper(Type type) {
	const char* strings[] = { "fuse", "breaker", "sec_A", "sec_V"};
	return strings[type];
}

// Print measurment struct, specifically to be used by the generic list
// Funkcija za ispis merenja
void PrintMeasurment(Measurment* m) {
	printf("Measurment: ");
	printf(" %s %s %d %.2f\n", GetStringFromEnumHelper(m->signal), GetStringFromEnumHelper(m->type), m->num, m->value);
}