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

using namespace std;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
       ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
       ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param,
       ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
       ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol;
//changed from string* to string so that we can compare strings. Im not 
//sure if that is right yet.
using symbol_table = unordered_map<string*,symbol*>;
using symbol_entry = pair<string*,symbol*>;

struct symbol {
   attr_bitset attributes;
   symbol_table* fields;
   size_t filenr, linenr, offset;
   size_t blocknr;
   vector<symbol*>* parameters;
   string * type;        //type is used when the variable is of type struct
};
void print_sym(string id_name, symbol * sym);
bool typeCheck(attr_bitset& first, attr_bitset& second, int atr);
bool existsInTable(symbol_table& table, const string* id);
void addSymToTable(symbol_table& table, const string* id, symbol * sym);
void structAstToSym (astree * node, int depth, symbol * sym, symbol_table& table);
const char* attrToStr (symbol* node);
void dumpToFile(FILE* symFile, string lexinfo, symbol* sym, int depth);
void checkEqual (astree * node, symbol * sym, size_t depth, symbol_table& table);
void performAttr(symbol * sym, bool isField, bool isParam);
symbol * process_node(astree * node, size_t depth, symbol_table& table,
  bool isField, bool isParam);
void addAttributes(attr_bitset& sym_attribute, int attribute);
symbol * create_sym (astree * node, size_t depth);
void parse_ast(astree * root, char** argv, int optind);


#endif