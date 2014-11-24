//Ryan Schreiber & Fedor Nikolayev

#include <string>
using namespace std;
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <iostream>
//Method to extract the program name
//If the input was /test/file/foo.oc the method would return foo
char * getBaseName(char * filePath) {
    string strFile(basename(filePath));
    int lastindex = strFile.find_last_of(".");
    //check if the suffix of the file is *.oc, 
    //if not exit with a failure.
    if (strFile.substr(lastindex + 1) != "oc"){
        fprintf(stderr, "Not a *.oc file.\n");
        exit (EXIT_FAILURE);
    }
    string name = strFile.substr(0, lastindex);
    char* url = new char[name.length()+1];
    strcpy(url,name.c_str());
    return url;
}
//Function that creates the a specified text folder.
// Returns a FILE with given suffix.
FILE * createFile(char * name, const char * suffix){
    char buffer[256];
    strncpy(buffer, name, sizeof(buffer));
    strncat(buffer, suffix, sizeof(buffer));
    FILE * output = fopen(buffer, "w");
    return output;
}

const char * tokenBaseName(const char * token){
   const char * tokName = token;
   if (strstr (tokName, "TOK_") == tokName){
    tokName += 4;
   }
   return tokName;
}
