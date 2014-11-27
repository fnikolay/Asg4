//Ryan Schreiber & Fedor Nikolayev

#ifndef __SYMBOLTYPES_H__
#define __SYMBOLTYPES_H__

#include <stdio.h>
//Compiler utils that are used through out the compiler
#include <bitset.h>
#include <string.h>
#include <unordered_map.h>
#include <vector.h>
using namespace std;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
       ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
       ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param,
       ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
       ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol;
using symbol_table = unordered_map<string*,symbol*>;
using symbol_entry = pair<string*,symbol*>;

struct symbol {
   attr_bitset attributes;
   symbol_table* fields;
   size_t filenr, linenr, offset;
   size_t blocknr;
   vector<symbol*>* parameters;
};

#endif