//Ryan Schreiber & Fedor Nikolayev

#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>


using namespace std;

#include "auxlib.h"
#include "symboltypes.h"

struct symb;
struct astree {
   int symbol;               // token code
   size_t filenr;            // index into filename stack
   size_t linenr;            // line number from source code
   size_t offset;            // offset of token with current line
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
   symb* sym;               // symbol associated with symbol table 
};

astree* new_astree (int symbol, int filenr, int linenr, int offset,
                    const char* lexinfo);
astree* createProto (astree * id, astree * param);

astree * createFunction ( astree * id, astree * param, astree * block );
astree* copyNode(astree* node, int symbol);
astree* adopt1 (astree* root, astree* child);
astree* adopt2 (astree* root, astree* left, astree* right);
astree * changeSym (astree * node, int symbol);
astree* adopt1sym (astree* root, astree* child, int symbol);
void dump_astree (FILE* outfile, astree* root);
void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep);
void free_ast (astree* tree);
void free_ast2 (astree* tree1, astree* tree2);

RCSH("$Id: astree.h,v 1.3 2014-11-17 10:26:12-08 - - $")

#endif
