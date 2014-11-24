//Ryan Schreiber & Fedor Nikolayev

#ifndef __COMUTILS_H__
#define __COMUTILS_H__

#include <stdio.h>
//Compiler utils that are used through out the compiler

char * getBaseName(char * filePath);

FILE * createFile(char * name, const char * suffix);

const char * tokenBaseName(const char * token);

#endif
