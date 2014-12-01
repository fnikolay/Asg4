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
#include "lyutils.h"
#include "auxlib.h"

symbol_table ident_table;
symbol_table global_table;
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

///////////////////////////////////////////////////////////////////////////////////////////

bool existsInTable(symbol_table& table, const string* id){

  string* newId = (string*) intern_stringset(id->c_str());
  if(table.find(newId) != table.end()){
  	return true;
  }else{
  	return false;

  }
}

//added this one need to 
void addSymToTable(symbol_table& table, const string* id, symbol * sym){
  
  string * newId = (string*) intern_stringset(id->c_str());
  if(!existsInTable(table, id)){
    table[newId] = sym;
    printf("Added symbol to table[%s]\n", newId->c_str());
    //DEBUGF('s', "\tAdded symbol to table[%s]\n", newId->c_str());
  }else{
    eprintf("%s:%d:%d error: redeclaration of '%s'\n\
             previously declared here: %s %d:%d\n",
            included_filenames[sym->filenr].c_str(), (int) sym->linenr,
            (int) sym->offset, newId->c_str(),
            included_filenames[table[newId]->filenr].c_str(),
            (int) table[newId]->linenr,
            (int) table[newId]->offset
    );
    set_exitstatus(EXIT_FAILURE);
  }
}
///////////////////////////////////////////////////////////////////////////////////
//only when adding a struct to a symbol table
void structAstToSym (astree * node, int depth, symbol * sym, symbol_table& table){
	for ( size_t i = 1; i < node->children.size(); i++){
	    astree * child = node->children[i];
	    if (child->symbol == TOK_IDENT) {
	    	printf("\n");
   	    	const string * id_name = child->children[0]->lexinfo;
   	    	const string * type_name = child->lexinfo;
   	    	printf("in struct:\n");
   	    	printf("id_name is: %s, type name is:%s.\n", id_name->c_str(), type_name->c_str());
   	    	//printf("the id_name of TOK_IDENT is: %s and the type_name is %s\n",
   	    	//	id_name->c_str(), type_name->c_str());
   	    	symbol * sym1 = create_sym(child, depth + 1);
            addAttributes(sym1->attributes, ATTR_field);
            sym1->type =(string *) type_name;
            addSymToTable(table,id_name, sym1);
            //table[(string *)id_name] = sym1;
            print_sym(* id_name, sym1);
 	    } else {
   	    	process_node(child, depth + 1, *sym->fields, true);
   	    }
    }
}

void funcAstToSym (astree * node, int depth, symbol * sym, symbol_table& table){
	for ( size_t i = 1; i < node->children.size(); i++){
	    astree * child = node->children[i];
	    if (child->symbol == TOK_IDENT) {
	    	printf("\n");
   	    	const string * id_name = child->children[0]->lexinfo;
   	    	const string * type_name = child->lexinfo;
   	    	printf("in struct:\n");
   	    	printf("id_name is: %s, type name is:%s.\n", id_name->c_str(), type_name->c_str());
   	    	//printf("the id_name of TOK_IDENT is: %s and the type_name is %s\n",
   	    	//	id_name->c_str(), type_name->c_str());
   	    	symbol * sym1 = create_sym(child, depth + 1);
            addAttributes(sym1->attributes, ATTR_field);
            sym1->type =(string *) type_name;
            addSymToTable(table,id_name, sym1);
            //table[(string *)id_name] = sym1;
            print_sym(* id_name, sym1);
 	    } else {
   	    	process_node(child, depth + 1, *sym->fields, false);
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
			printf("NOT CHECKING this TYPE YET! NEED TO ADD.\n");
		}
		
	}
}

void performFieldAttr(symbol * sym, bool isField){
	if (isField)
		addAttributes(sym->attributes,ATTR_field);
	else{
		addAttributes(sym->attributes,ATTR_lval);
		addAttributes(sym->attributes,ATTR_variable);
	}
}


//Create needed symbol tables
symbol * process_node(astree * node, size_t depth, symbol_table& table, bool isField){

	symbol * sym = NULL;

	switch (node->symbol){
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
		case TOK_IDENT:
		{
			const string * id_name = node->children[0]->lexinfo;
   	    	const string * type_name = node->lexinfo;
			printf("id_name is: %s, type name is:%s.\n", id_name->c_str(), type_name->c_str());
			sym = create_sym(node, depth);
			printf("went into tok_ident\n");
			sym->type =(string *) type_name;
			addAttributes(sym->attributes,ATTR_struct);
			//it is a field only if it is in a struct
			performFieldAttr(sym, isField);
			addSymToTable(table, id_name, sym);
	        //table[(string *)key] = sym;
	        //table[(string *)id_name] = sym;
	        print_sym(* id_name, sym);
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
			performFieldAttr(sym, isField);
			addSymToTable(table, id_name, sym);				

			//table[(string *)key] = sym;
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
			performFieldAttr(sym, isField);
			addSymToTable(table, id_name, sym);	
	        //table[(string *)key] = sym;
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
			performFieldAttr(sym, isField);
			addSymToTable(table, id_name, sym);
	        //table[(string *)key] = sym;
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
			performFieldAttr(sym, isField);
			addSymToTable(table, id_name, sym);
	        //table[(string *)key] = sym;
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
			performFieldAttr(sym, isField);
			addSymToTable(table, id_name, sym);
	        //table[(string *)key] = sym;
	        //table[(string *)id_name] = sym;
	        print_sym(* id_name, sym);
			break;
		}
		case TOK_TYPEID:
		{
			const string * struct_id = node->lexinfo;
			sym = create_sym(node, depth);
			addAttributes(sym->attributes, ATTR_struct);
			performFieldAttr(sym, isField);
			sym->type = (string *) struct_id;
			sym->fields = new symbol_table ();
			const string * id_name = node->children[0]->lexinfo;
			printf("id_name is: %s\n", id_name->c_str());
			//global_table[(string *)id_name] = sym;
			table[(string *)id_name] = sym;
	    	structAstToSym (node, depth, sym, table);
	    	break;
	    }
	    /*
	    case TOK_PROTOTYPE:
	    {
			const string * prot_id = node->lexinfo;
			sym = create_sym(node, depth);
			addAttributes(sym->attributes, ATTR_struct);
			performFieldAttr(sym, isField);
			sym->fields = new symbol_table ();
			const string * id_name = node->children[0]->lexinfo;
			printf("id_name is: %s\n", id_name->c_str());
			struct_table[(string *)id_name] = sym;
	    	structAstToSym (node, depth, sym, table);
	    	break;
	    }
	    */
	    case TOK_FUNCTION:
	    {
	    	printf("\n");
	    	bool isFunc = true;
	    	astree * func = node->children[0];
	    	const string * func_id = func->children[0]->lexinfo;
	    	const string * func_type = func->lexinfo;
			//printf("The function type is: %s, and ID name is: %s\n",func_type.c_str(), func_id.c_str());
			sym = create_sym(func, depth);
			addAttributes(sym->attributes, ATTR_function);
			addSymToTable(global_table, func_id, sym);

			astree * param = node->children[1];

			if (param->symbol == TOK_PARAM){
				addAttributes(sym->attributes, ATTR_param);
				process_node(param, depth, table, false);
			}
			//table[(string *)id_name] = sym;
			//performFieldAttr(sym, false);
			//need to go through parameters of a function

			//need to go through the block of the function

			//need to check the return value

/*
			sym->fields = new symbol_table ();
			const string * id_name = node->children[0]->lexinfo;
			printf("id_name is: %s\n", id_name->c_str());
			//struct_table[(string *)id_name] = sym;
	    	structAstToSym (node, depth, sym, table);
	    	*/
	    	break;
	    }
	    case TOK_PARAM:
	    {

	    	break;
	    }
	    default:
	    {
			printf("Found Token that is not handled yet %s with the TOK called: %s\n",
				node->lexinfo->c_str(), get_yytname (node->symbol));
		}
    
    }
	return sym;
}

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