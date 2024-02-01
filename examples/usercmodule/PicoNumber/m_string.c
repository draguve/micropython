
#include "m_string.h"

size_t strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

int toLower(int chr)//touches only one character per call
{
    return (chr >='A' && chr<='Z') ? (chr + 32) : (chr);    
}