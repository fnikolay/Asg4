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
vector<symbol_table*>* symbol_stack;
int block_num;

void print_sym(string lexinfo, symbol * sym){
    printf("%s (%ld.%ld.%ld) {%ld} {%s}\n", lexinfo.c_str(),
        sym->filenr, sym->linenr, sym->offset, sym->blocknr,
        sym->attributes[ATTR_field] ? "field" : "not a field");

}

//check if the two bitset attributes are the same
bool typeCheck(attr_bitset& first, attr_bitset& second, int atr){
	return first[atr] == second[atr];
}

/*
//added this one need to 
void intern_sym(symbol_table& table, sym_ref* sym, const string* key){
  
  string * key = (string*) intern_stringset(key->c_str());
  
  if(not is_in_sym_table(table, key)){
    table[newKey] = sym;
    DEBUGF('s', "\tAdded symbol to table[%s]\n", newKey->c_str());
  }else{
    eprintf("%s:%d:%d error: redeclaration of '%s'\n\
             previously declared here: %s %d:%d\n",
            included_filenames[sym->filenr].c_str(), (int) sym->linenr,
            (int) sym->offset, newKey->c_str(),
            included_filenames[table[newKey]->filenr].c_str(),
            (int) table[newKey]->linenr,
            (int) table[newKey]->offset
    );
    set_exitstatus(EXIT_FAILURE);
  }
}
*/
//only when adding a struct to a symbol table
void structAstToSym (astree * node, int depth, symbol * sym, symbol_table& table){
	for ( size_t i = 1; i < node->children.size(); i++){
	    astree * child = node->children[i];
	    if (child->symbol == TOK_IDENT) {
   	    	const string * id_name = child->children[0]->lexinfo;
   	    	const string * type_name = child->lexinfo;
   	    	
   	    	//printf("the id_name of TOK_IDENT is: %s and the type_name is %s\n",
   	    	//	id_name->c_str(), type_name->c_str());
   	    	symbol * sym1 = create_sym(child, depth + 1);
            addAttributes(sym1->attributes, ATTR_field);
            sym1->type =(string *) type_name;
            table[(string *)id_name] = sym1;
            print_sym(* id_name, sym1);
 	    } else {
   	    	process_node(child, depth + 1, *sym->fields, true);
   	    }
    }
}

//we will need to check types of all of the needed variables. 
//only doing boolean so far.
void checkEqual (astree * node, symbol * sym, size_t depth, symbol_table& table){
	if (node->children.size() > 1){
		printf("Need to check equivalance.\n");
		//check if for the symbol there is a boolean attribute.
		if (sym->attributes[ATTR_bool]){
			symbol * con = process_node (node->children[1], depth, table, false);
			bool equal = typeCheck(sym->attributes, con->attributes, ATTR_bool);
			printf("the two attributes are: %d\n", equal);
			if(!equal){
				printf("NOT A BOOLEAN\n");
			}else{
				printf("IS A BOOLEAN\n");
			}
		}else{
			printf("NOT CHECKING this TYPE YET! NEED TO ADD.\n", );
		}
		
	}
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
		case TOK_TRUE:
		case TOK_FALSE:
		{
			printf("found either TRUE or FALSE\n");
			sym = create_sym (node,depth);
			sym->attributes[ATTR_bool] = true;

			break;
	    }
		case TOK_VARDECL:
		{
	    	printf("VarDecl\n");
	    	sym = process_node (node->children[0], depth, table, isField);
	    	//printf("going into constant symbol (to the right of the = sign)\n");
	    	
	    	checkEqual (node,sym,depth,table);
	    	//print_sym(* id_name, sym);
	    	
	    	break;
		}
		case TOK_INT:
		{
			const string * key = (string*) intern_stringset("int");
			const string * id_name = node->children[0]->lexinfo;
			printf("id_name is: %s, key name is:%s.\n", id_name->c_str(), key->c_str());
			sym = create_sym(node, depth);
			addAttributes(sym->attributes,ATTR_int);
			//it is a field only if it is in a struct
			if (isField)
				addAttributes(sym->attributes,ATTR_field);
			table[(string *)key] = sym;
	        //table[(string *)id_name] = sym;
	        print_sym(* id_name, sym);
	        //printf("Inserting integer symbol %s\n", id_name->c_str());
	        break;
		}
		case TOK_CHAR:
		{
			const string * key = (string*) intern_stringset("char");
			const string * id_name = node->children[0]->lexinfo;
			printf("id_name is: %s, key name is:%s.\n", id_name->c_str(), key->c_str());
			sym = create_sym(node, depth);
			addAttributes(sym->attributes,ATTR_char);
			//it is a field only if it is in a struct
			if (isField)
				addAttributes(sym->attributes,ATTR_field);
	        table[(string *)key] = sym;
	        //table[(string *)id_name] = sym;
	        print_sym(* id_name, sym);
	        break;
		}
		case TOK_BOOL:
		{
			const string * key = (string*) intern_stringset("bool");
			const string * id_name = node->children[0]->lexinfo;
			printf("id_name is: %s, key name is:%s.\n", id_name->c_str(), key->c_str());
			sym = create_sym(node, depth);
			addAttributes(sym->attributes,ATTR_bool);
			//it is a field only if it is in a struct
			if (isField)
				addAttributes(sym->attributes,ATTR_field);
	        table[(string *)key] = sym;
	        //table[(string *)id_name] = sym;
	        print_sym(* id_name, sym);

	        break;
		}
		case TOK_STRING:
		{
			const string * key = (string*) intern_stringset("bool");
			const string * id_name = node->children[0]->lexinfo;
			printf("id_name is: %s, key name is:%s.\n", id_name->c_str(), key->c_str());
			sym = create_sym(node, depth);
			addAttributes(sym->attributes,ATTR_string);
			//it is a field only if it is in a struct
			if (isField)
				addAttributes(sym->attributes,ATTR_field);
	        table[(string *)key] = sym;
	        //table[(string *)id_name] = sym;
	        print_sym(* id_name, sym);

			break;
		}
		case TOK_VOID:
		{
			const string * key = (string*) intern_stringset("void");
			const string * id_name = node->children[0]->lexinfo;
			printf("id_name is: %s, key name is:%s.\n", id_name->c_str(), key->c_str());
			sym = create_sym(node, depth);
			addAttributes(sym->attributes,ATTR_string);
			//it is a field only if it is in a struct
			if (isField)
				addAttributes(sym->attributes,ATTR_field);
	        table[(string *)key] = sym;
	        //table[(string *)id_name] = sym;
	        print_sym(* id_name, sym);
			break;
		}
		case TOK_TYPEID:
		{

			const string * struct_id = node->lexinfo;
			sym = create_sym(node, depth);
			addAttributes(sym->attributes, ATTR_struct);
			if (isField)
				addAttributes(sym->attributes, ATTR_field);
			sym->fields = new symbol_table ();
			const string * id_name = node->children[0]->lexinfo;
			printf("id_name is: %s\n", id_name->c_str());
			struct_table[(string *)id_name] = sym;
	    	structAstToSym (node, depth, sym, table);
	    	break;
	    }
		case TOK_STRUCT:
		{
			sym = create_sym(node, depth);
			addAttributes(sym->attributes, ATTR_struct);
			sym->attributes[ATTR_field] = isField;
			sym->fields = new symbol_table ();
			const string * id_name = node->children[0]->lexinfo;
			printf("The id_name of the struct is: %s\n", id_name->c_str());
			struct_table[(string *)id_name] = sym;
			for ( size_t i = 1; i < node->children.size(); i++){
	    	    astree * child = node->children[i];
	    	    if (child->symbol == TOK_IDENT) {
	    	    	const string * id_name = child->children[0]->lexinfo;
	    	    	const string * type_name = child->lexinfo;
	    	    	printf("the id_name of TOK_IDENT is: %s and the type_name is %s\n", id_name->c_str(), type_name->c_str());
	    	    	symbol * sym1 = create_sym(child, depth + 1);	               
	                addAttributes(sym1->attributes, ATTR_field);
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
			printf("Found Token that is not handled yet %s with the TOK called: %s\n",
				node->lexinfo->c_str(), get_yytname (node->symbol));
    }
	return sym;
}
/*
symbol * protToSymbol (astree * node, depth){

}
*/
//set attributes of the symbol attributes.


void addAttributes(attr_bitset& sym_attribute, int attribute){
	sym_attribute[attribute] = true;
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
	//symbol_table table;

	for ( size_t i = 0; i < root->children.size(); i++){
    	astree * child = root->children[i];
    	process_node(child, 0, ident_table, false);
    }
    block_num = 0;
    symbol_stack = new vector<symbol_table*>();
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