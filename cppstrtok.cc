//cppstrtok.cpp has the main method. It runs cpp against lines.
//Ryan Schreiber & Fedor Nikolayev
// Use cpp to scan a file and print line numbers.
// Print out each input line read in, then strtok it for
// tokens.

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

#include "stringset.h"
#include "auxlib.h"
#include "astree.h"
#include "lyutils.h"
#include "comutils.h"
#include "symboltypes.h"

FILE * tok;
FILE * ast;
FILE * symFil;
const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;
string yyin_cpp_command;
int opt;
string flag = "";
char * atName;
char * DName;

void getOpts(int argc, char** argv){
   yy_flex_debug = 0;
   yydebug = 0;
   ///////////////////checking options
   //used this code sample from the lecture 08 sample code from class.
   for(;;) {
      opt = getopt (argc, argv, "@:D:ly");
      if (opt == EOF) break;
      switch (opt) {
         case '@':
            //At = 1;
            atName = optarg;
            set_debugflags(atName);
            break;
         case 'D':
            // D = 1;
            DName = optarg;
            flag.append("-D");
            flag.append(DName);
            break;
         case 'l':
            yy_flex_debug = 1;
            break;
         case 'y':
            yydebug = 1;
            break;
         default:
            errprintf ("%:bad option (%c)\n", optopt);
            exit (EXIT_FAILURE);
            break;
      }
   }
}

void scanTokens (char** argv){

    ast = createFile(getBaseName(argv[optind]), ".ast");
    tok = createFile(getBaseName(argv[optind]), ".tok");
    symFil = createFile(getBaseName(argv[optind]), ".sym");
    
    yyparse();
    dump_astree (ast, yyparse_astree);
    parse_ast (symFil, yyparse_astree);
}

void yyin_cpp_popen (const char* filename, int optind, char** argv) {
   yyin_cpp_command = CPP + " " + flag + " " + filename;
   yyin = popen (yyin_cpp_command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (yyin_cpp_command.c_str());
      exit (get_exitstatus());
   } else {
      //scan the necessary tokens that are in the file
      scanTokens (argv);
      
      int pclose_rc = pclose (yyin);
      eprint_status (yyin_cpp_command.c_str(), pclose_rc);
      //check this again.
      if (pclose_rc != 0){
         set_exitstatus(EXIT_FAILURE);
      }
      dump_stringset (createFile(getBaseName(argv[optind]), ".str"));
   }
}


//Main function that sorts the flags and checks the correct
//
int main (int argc, char** argv) {
   set_execname (argv[0]);
   //Check if there is only one argument in command line
   //if so return an error.
   if (argc == 1){
      fprintf(stderr, "A file was not included.\n");
      exit (EXIT_FAILURE);
   }

   getOpts(argc, argv);

   if (optind > argc) {
      errprintf ("Usage: %s [-ly] [-@flag...] [-Dstring] program.oc\n",
                  argv[0]);
      exit (get_exitstatus());
   }
   const char* filename = optind == argc ? "-" : argv[optind];
   
   if (argv[optind + 1] != NULL){
      fprintf(stderr, "Can't have more than one file to process.\n");
      exit (EXIT_FAILURE);
   }
   ///////////////////end of checking options
   yyin_cpp_popen (filename, optind, argv);

   return get_exitstatus();
   
}

