//Ryan Schreiber & Fedor Nikolayev
#include <bitset>
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symboltypes.h"
#include "astree.h"
#include "stringset.h"

symbol_table ident_table;
symbol_table struct_table;
vector<symbol_table*> symbol_stack;

void print_sym(string id_name, symbol * sym){
    printf("%s (%ld.%ld.%ld) {%ld} {%s}\n", id_name.c_str(),
        sym->filenr, sym->linenr, sym->offset, sym->blocknr,
        sym->attributes[ATTR_field] ? "field" : "not a field");

}


//Create needed symbol tables
symbol * process_node(astree * node, size_t depth, symbol_table& table, bool isField){

	symbol * sym = NULL;
	switch (node->symbol){
		//case TOK_PROTOTYPE:
		//{
		//	int atr[] = {ATTR_prototype};
		//	sym = create_sym(node, depth);
		//	addAttributes(sym->attributes, atr);
		//	break;
		//}
		case TOK_VARDECL:
		{
	    	printf("VarDecl \n");
	    	sym = process_node(node->children[0], depth, table, isField);
	    	break;
		} 
		case TOK_INT:
		{
			const string * id_name = node->children[0]->lexinfo;
			int atr[] = {ATTR_int};
			sym = create_sym(node, depth);
			addAttributes(sym->attributes,atr);
			sym->attributes[ATTR_field] = isField;
			//sym->attributes[ATTR_int] = true;
	     //   table [node->lexinfo] = sym = create_sym(node, depth);sym;
	        table[(string *)id_name] = sym;
	        //table->insert (symbol_entry ((string *)id_name, sym));
	        print_sym(* id_name, sym);
	        //printf("Inserting integer symbol %s\n", id_name->c_str());
	        break;
		}
		case TOK_TYPEID:
		{
			const string * struct_id = node->lexinfo;

			int atr[] = {ATTR_struct};
			sym = create_sym(node, depth);
			addAttributes(sym->attributes, atr);
			sym->attributes[ATTR_field] = isField;
			sym->fields = new symbol_table ();
			const string * id_name = node->children[0]->lexinfo;
			printf("The id_name of the struct is: %s\n", id_name->c_str());
			struct_table[(string *)id_name] = sym;//->insert (symbol_entry ((string *)id_name,sym));
			for ( size_t i = 1; i < node->children.size(); i++){
	    	    astree * child = node->children[i];
	    	    if (child->symbol == TOK_IDENT) {
	    	    	const string * id_name = child->children[0]->lexinfo;
	    	    	const string * type_name = child->lexinfo;
	    	    	printf("the id_name of TOK_IDENT is: %s and the type_name is %s\n", id_name->c_str(), type_name->c_str());
	    	    	symbol * sym1 = create_sym(child, depth + 1);
	    	    	int atr1[] = {ATTR_field};
	               
	                addAttributes(sym1->attributes, atr1);
	                sym1->type =(string *) type_name;
	                table[(string *)id_name] = sym1;
/*
	    	    	if (child->children.size () > 0) {
	    	    	  int atr1[] = {ATTR_field};
	                  symbol * sym1 = create_sym(node, depth + 1);
	                  addAttributes(sym1->attributes, atr1);
			          //sym1->attributes[ATTR_field] = true;
	    	    	} else {
	                    id_name = (string *)child->lexinfo;
	                }
	                */
	    	    } else {
	    	    	process_node(child, depth + 1, *sym->fields, true);
	    	    }
	    	}
	    	break;
	    }
		case TOK_STRUCT:
		{
			int atr[] = {ATTR_struct};
			sym = create_sym(node, depth);
			addAttributes(sym->attributes, atr);
			sym->attributes[ATTR_field] = isField;
			sym->fields = new symbol_table ();
			const string * id_name = node->children[0]->lexinfo;
			printf("The id_name of the struct is: %s\n", id_name->c_str());
			struct_table[(string *)id_name] = sym;//->insert (symbol_entry ((string *)id_name,sym));
			for ( size_t i = 1; i < node->children.size(); i++){
	    	    astree * child = node->children[i];
	    	    if (child->symbol == TOK_IDENT) {
	    	    	const string * id_name = child->children[0]->lexinfo;
	    	    	const string * type_name = child->lexinfo;
	    	    	printf("the id_name of TOK_IDENT is: %s and the type_name is %s\n", id_name->c_str(), type_name->c_str());
	    	    	symbol * sym1 = create_sym(child, depth + 1);
	    	    	int atr1[] = {ATTR_field};
	               
	                addAttributes(sym1->attributes, atr1);
	                sym1->type =(string *) type_name;
	                table[(string *)id_name] = sym1;
/*
	    	    	if (child->children.size () > 0) {
	    	    	  int atr1[] = {ATTR_field};
	                  symbol * sym1 = create_sym(node, depth + 1);
	                  addAttributes(sym1->attributes, atr1);
			          //sym1->attributes[ATTR_field] = true;
	    	    	} else {
	                    id_name = (string *)child->lexinfo;
	                }
	                */
	    	    } else {
	    	    	process_node(child, depth + 1, *sym->fields, true);
	    	    }
	    	}
	    	
	    	break;
	    }
		default:
			printf("Found Token that is not handled yet %s\n", node->lexinfo->c_str());
    }
	return sym;
}
/*
symbol * protToSymbol (astree * node, depth){

}
*/
//set attributes of the symbol attributes.
void addAttributes(attr_bitset& sym_attribute, int attribute[]){
	int num_att = sizeof(attribute);
	for (int i = 0; i < num_att; ++i){
		sym_attribute[attribute[i]] = true;
	}
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
	symbol_table table;

	for ( size_t i = 0; i < root->children.size(); i++){
    	astree * child = root->children[i];
    	process_node(child, 0, ident_table, false);
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