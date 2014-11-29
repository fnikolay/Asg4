//Ryan Schreiber & Fedor Nikolayev

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symboltypes.h"
#include "astree.h"
#include "stringset.h"
#include "lyutils.h"
#include "comutils.h"

//symbol_table ident_table;
//symbol_table struct_table;
vector<symbol_table*> symbol_stack;



symbol * process_node(astree * node, size_t depth, symbol_table * table){
	symbol * sym = NULL;
    if (node->symbol == TOK_VARDECL){
    	printf("VarDecl \n");
    	sym = process_node(node->children[0], depth, table);
	} 
	else if (node->symbol == TOK_INT){
		printf("Inserting integer symbol %s\n", node->lexinfo->c_str());
		sym = create_sym(node, depth);
		sym->attributes = ATTR_int;
     //   table [node->lexinfo] = sym;
        table->insert (symbol_entry ((string *)node->lexinfo,sym));

	} else {
		printf("Found Token that is not handled yet %s\n", node->lexinfo->c_str());
	}
	return sym;
}

symbol * create_sym (astree * node, size_t depth){
	symbol * sym = new symbol();
	sym->filenr = node->filenr;
	sym->linenr = node->linenr;
	sym->offset = node->offset;
    sym->blocknr = depth;
    sym->fields = NULL;
	sym->parameters = NULL;
    return sym;
}

void parse_ast(astree * root){
	if (root->symbol != TOK_ROOT)
	{
		return;  // must be root
	}
	symbol_table * table = new symbol_table();

	for ( int i = 0; i < root->children.size(); i++){
    	astree * child = root->children[i];
    	process_node(child, 0, table);
    }
}

/*
static void dump_node (FILE* outfile, astree* node) {
   fprintf (outfile, "%s \"%s\" %ld.%ld.%ld",
            tokenBaseName(get_yytname (node->symbol)),
            node->lexinfo->c_str(),
            node->filenr, node->linenr, node->offset);
}

static void dump_astree_rec (FILE* outfile, astree* root, int depth) {
   if (root == NULL) return;
   int i;
   for (i = 0; i < depth; ++i ){
      fprintf (outfile, "|%*s", 3, "");
   }
   dump_node (outfile, root);
   fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size(); ++child) {
      dump_astree_rec (outfile, root->children[child], depth + 1);
   }
}

//dump out the astree to a file
void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}
*/