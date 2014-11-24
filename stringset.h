// $Id: stringset.h,v 1.1 2014-11-13 13:13:39-08 - - $
//Ryan Schreiber & Fedor Nikolayev

#ifndef __STRINGSET__
#define __STRINGSET__

#include <string>
#include <unordered_set>
using namespace std;

#include <stdio.h>

const string* intern_stringset (const char*);

void dump_stringset (FILE*);

#endif

