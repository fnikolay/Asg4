//Ryan Schreiber & Fedor Nikolayev

#ifndef __SYMBOLTYPES_H__
#define __SYMBOLTYPES_H__

#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"
#include "comutils.h"
#include "lyutils.h"
#include "auxlib.h"

extern FILE* symFil;

using namespace std;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
       ATTR_string, ATTR_struct, ATTR_array, ATTR_prototype, 
       ATTR_function, ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param,
       ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
       ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct astree;
struct symb;
//changed from string* to string so that we can compare strings. Im not 
//sure if that is right yet.
using symbol_table = unordered_map<string*,symb*>;
using symbol_entry = pair<string*,symb*>;

struct symb {
   attr_bitset attributes;
   symbol_table* fields;
   size_t filenr, linenr, offset;
   size_t blocknr;
   vector<symb*>* parameters;
   string * type;        //type is used when the variable is of type struct
};
void print_sym(string id_name, symb * symref);
bool typeCheck(attr_bitset& first, attr_bitset& second, int atr);
bool existsInTable(symbol_table& table, const string* id);
string * getNodeName(symb * symref, astree * node);
void checkVariableType(symbol_table& table, const string* id, symb * symref);
void addSymToTable(symbol_table& table, const string* id, symb * symref);
symb *  structAstToSym (astree * node, int depth, symbol_table& table,
  string * structType, int blockNum, FILE * output);
symb *  protAstToSym (astree * node, int depth, int attr, symb * symref,
  symbol_table& table, int blockNum, FILE * output);
symb *  funcAstToSym (astree * node, int depth, int attr, symb * symref,
  symbol_table& table, int blockNum, FILE * output);
void ifWhileToSym(astree * node, int depth, symbol_table& table,
  bool isField, bool isParam, int blockNum, FILE * output);
void ifElseToSym(astree * node, int depth, symbol_table& table,
  bool isField, bool isParam, int blockNum, FILE * output);
void blockToSym(astree * node, int depth, symbol_table& table,
  bool isField, bool isParam, int blockNum, FILE * output);
const char* attrToStr (symb* node);
//void attrToStr (symb* node);
void dumpToFile(FILE* symFile, string lexinfo, symb* symref, int depth);
void checkEqual (astree * node, symb * symref, size_t depth,
  symbol_table& table, int blockNum, FILE * output);
void performAttr(symb * symref, bool isField, bool isParam);
symb *  classifyIdent (astree * node, size_t depth, symbol_table& table,
  symb* symref, bool isField, bool isParam, string * structType,
  int blockNum, FILE * output);
symb *  classifySymbol (astree * node, size_t depth, symbol_table& table,
  symb* symref, int attr, bool isField, bool isParam,
  int blockNum, FILE * output);
void functionAttr(astree * func, symb * symref);
symb * process_node(astree * node, size_t depth, symbol_table& table,
  bool isField, bool isParam , string * structType,
  int blockNum, FILE * output);
void addAttributes(attr_bitset& sym_attribute, int attribute);
symb * create_sym (astree * node, size_t blcknr);
//void parse_ast(astree * root, char** argv, int optind);
void parse_ast(FILE * output, astree * root);


#endif