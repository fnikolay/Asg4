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


FILE * symFil;
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
void structAstToSym (astree * node, int depth, symbol * sym, symbol_table& table, string * structType){
	//block_num++;
	for ( size_t i = 1; i < node->children.size(); i++){
	    astree * child = node->children[i];
	    if (child->symbol == TOK_IDENT) {
	    	printf("\n");
   	    	const string * id_name = child->children[0]->lexinfo;
   	    	//const string * type_name = child->lexinfo;
   	    	printf("in struct:\n");
   	    	printf("id_name is: %s, type name is:%s.\n", id_name->c_str(), structType->c_str());
   	    	//printf("the id_name of TOK_IDENT is: %s and the type_name is %s\n",
   	    	//	id_name->c_str(), type_name->c_str());
   	    	//symbol * sym1 = create_sym(child, depth + 1);
   	    	symbol * sym1 = create_sym(child, 0);
            addAttributes(sym1->attributes, ATTR_field);
			addAttributes(sym1->attributes, ATTR_struct);
			addAttributes(sym1->attributes, ATTR_typeid);
            //printf("Before assigning type: %s\n", sym1->type->c_str());
            sym1->type =(string *) structType;
            printf("After assigning type: %s\n", sym1->type->c_str());

            fprintf(symFil,"SYMBOL TYPE is: %s\n", sym1->type->c_str());
            addSymToTable(table,id_name, sym1);
            //table[(string *)id_name] = sym1;
            dumpToFile(symFil, *id_name, sym1, depth+1);
           // print_sym(* id_name, sym1);
 	    } else {
   	    	//process_node(child, depth + 1, *sym->fields, true, false);
   	    	process_node(child, depth + 1, *sym->fields, true, false, (string *) structType);
   	    }
    }
}

/*
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
   	    	process_node(child, depth + 1, *sym->fields, false, true);
   	    }
    }
}
*/
//we will need to check types of all of the needed variables. 
//only doing boolean so far.



const char* attrToStr (symbol* sym){
//void attrToStr (symbol* sym){
	//printf ("attr to string SYMBOL TYPE is: %s\n", sym->type->c_str()); 
  string attrs = "";
  if(sym->attributes[ATTR_field]){
  	attrs = attrs + "field {" +sym->type->c_str()+"} ";
  	//string * type = sym->type;
  	//printf ("attr to string SYMBOL TYPE is:\n"); //sym->type->c_str());
  }
  if(sym->attributes[ATTR_void]) attrs+="void ";
  if(sym->attributes[ATTR_bool]) attrs+="bool ";
  if(sym->attributes[ATTR_char]) attrs+="char ";
  if(sym->attributes[ATTR_int]) attrs+="int ";
  if(sym->attributes[ATTR_null]) attrs+="null ";
  if(sym->attributes[ATTR_string]) attrs+="string ";
  if(sym->attributes[ATTR_struct]) attrs+="struct ";
  if(sym->attributes[ATTR_typeid]){
  	attrs+= string("\"") + sym->type->c_str() + "\" ";
  }
  if(sym->attributes[ATTR_array]) attrs+="array ";
  if(sym->attributes[ATTR_function]) attrs+="function ";
  //if(sym->attributes[ATTR_prototype]) attrs+="prototype ";
  if(sym->attributes[ATTR_variable]) attrs+="variable ";
  if(sym->attributes[ATTR_param]) attrs+="param ";
  if(sym->attributes[ATTR_lval]) attrs+="lval ";
  if(sym->attributes[ATTR_const]) attrs+="const ";
  if(sym->attributes[ATTR_vreg]) attrs+="vreg ";
  if(sym->attributes[ATTR_vaddr]) attrs+="vaddr ";
  return attrs.c_str();
}


void dumpToFile(FILE* symFile, string lexinfo, symbol* sym, int depth){
	for(int i = 0; i<depth; i++) fprintf(symFile, "    ");
	if (!sym->attributes[ATTR_field]){
		fprintf(symFile, "%s (%ld.%ld.%ld) {%ld} %s\n", lexinfo.c_str(),
			sym->filenr, sym->linenr, sym->offset, sym->blocknr,
			attrToStr(sym));
	}else{

		fprintf(symFile, "%s (%ld.%ld.%ld) %s\n", lexinfo.c_str(),
			sym->filenr, sym->linenr, sym->offset,
			attrToStr(sym));
	}

  /*
  fprintf(symFile, "%s (%ld.%ld.%ld) {%ld}\n", lexinfo.c_str(),
          sym->filenr, sym->linenr, sym->offset, sym->blocknr);
  attrToStr(sym);
*/
}

void checkEqual (astree * node, symbol * sym, size_t depth, symbol_table& table){
	if (node->children.size() > 1){
		printf("Need to check equivalance.\n");
		//check if for the symbol there is a boolean attribute.
		if (sym->attributes[ATTR_bool]){
			symbol * con = process_node (node->children[1], depth, table, false, false, (string *) "");
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

void performAttr(symbol * sym, bool isField, bool isParam){
	if (isField){
		addAttributes(sym->attributes,ATTR_field);
		return;
	}
	else if(isParam){
		addAttributes(sym->attributes,ATTR_param);
		addAttributes(sym->attributes,ATTR_lval);
		addAttributes(sym->attributes,ATTR_variable);
	}
	else
	{
		addAttributes(sym->attributes,ATTR_lval);
		addAttributes(sym->attributes,ATTR_variable);
	}
}

void classifyIdent (astree * node, size_t depth, symbol_table& table,
	symbol* sym, bool isField, bool isParam){
	const string * id_name = node->children[0]->lexinfo;
	printf("id_name is: %s\n", id_name->c_str());
	sym = create_sym(node, depth);
	sym->type = (string *) node->lexinfo;
	addAttributes(sym->attributes, ATTR_struct);
	addAttributes(sym->attributes, ATTR_typeid);
	performAttr(sym, isField, isParam);
	addSymToTable(table, id_name, sym);
	dumpToFile(symFil, *id_name, sym, depth);
	printf("\n");
}

void classifySymbol (astree * node, size_t depth, symbol_table& table,
	symbol* sym, int attr, bool isField, bool isParam, string * structType){
	const string * id_name = node->children[0]->lexinfo;
	printf("id_name is: %s\n", id_name->c_str());//, key->c_str());
	sym = create_sym(node, depth);
	addAttributes(sym->attributes, attr);
	performAttr(sym, isField, isParam);
	addSymToTable(table, id_name, sym);
	sym->type = (string *) structType;
	dumpToFile(symFil, *id_name, sym, depth);
	printf("\n");
}

/*
void addType(symbol * sym, astree * node, bool isParam){
	if(isParam) sym->type = ;
	else{

	}
}
*/

void functionAttr(astree * func, symbol * sym){
	addAttributes(sym->attributes, ATTR_function);
	switch (func->symbol){
		case TOK_IDENT:
		{
			addAttributes(sym->attributes, ATTR_struct);
			break;
	    }
		case TOK_INT:
		{
			addAttributes(sym->attributes, ATTR_int);
	        break;
		}
		case TOK_CHAR:
		{
			addAttributes(sym->attributes, ATTR_char);
	        break;
		}
		case TOK_BOOL:
		{
			addAttributes(sym->attributes, ATTR_bool);
	        break;
		}
		case TOK_STRING:
		{
			addAttributes(sym->attributes, ATTR_string);
			break;
		}
		case TOK_VOID:
		{
			addAttributes(sym->attributes, ATTR_void);
			break;
		}
	}
}


//Create needed symbol tables
symbol * process_node(astree * node, size_t depth, symbol_table& table,
	bool isField, bool isParam, string * structType){

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
	    	sym = process_node (node->children[0], depth, table, isField, isParam, (string *) structType);
	    	//printf("going into constant symbol (to the right of the = sign)\n");
	    	
	    	//checkEqual (node,sym,depth,table);
	    	//print_sym(* id_name, sym);
	    	
	    	break;
		}
		case TOK_IDENT:
		{
			classifyIdent (node, depth, table, sym, isField, isParam);
			break;
	    }
		case TOK_INT:
		{
			classifySymbol (node, depth, table, sym, ATTR_int, isField, isParam, (string *) structType);
	        break;
		}
		case TOK_CHAR:
		{
			classifySymbol (node, depth, table, sym, ATTR_char, isField, isParam, (string *) structType);
	        break;
		}
		case TOK_BOOL:
		{
			classifySymbol (node, depth, table, sym, ATTR_bool, isField, isParam, (string *) structType);
	        break;
		}
		case TOK_STRING:
		{
			classifySymbol (node, depth, table, sym, ATTR_string, isField, isParam, (string *) structType);
			break;
		}
		case TOK_VOID:
		{
			classifySymbol (node, depth, table, sym, ATTR_string, isField, isParam, (string *) structType);
			break;
		}
		case TOK_TYPEID:
		{
			const string * struct_id = node->children[0]->lexinfo;
			printf("TEST\n");
			//sym = create_sym
			//sym = create_sym(node, depth);
			sym = create_sym(node, 0);
			printf("TEST\n");
			addAttributes(sym->attributes, ATTR_struct);
			printf("TEST\n");
			performAttr(sym, isField, isParam);
			printf("TEST\n");
			//printf("Before assigning type: %s\n", sym->type->c_str());
			sym->type = (string *) struct_id;
			//str_type = ;
			printf("After assigning type: %s\n", sym->type->c_str());
			sym->fields = new symbol_table ();
			dumpToFile(symFil, *struct_id, sym, depth);
			printf("struct_id is: %s\n", struct_id->c_str());
			//global_table[(string *)id_name] = sym;
			addSymToTable(table, struct_id, sym);
			//print_sym(* struct_id, sym);
			//table[(string *)id_name] = sym;
			structAstToSym (node, depth, sym, table, (string *) struct_id);
			//structAstToSym (node, depth, sym, table);
			fprintf(symFil, "\n");

	    	break;
	    }
	    
	    case TOK_PROTOTYPE:
	    {
			//block_num = 0;
	    	printf("\n");
	    	astree * prot = node->children[0];
	    	const string * prot_id = prot->children[0]->lexinfo;
			sym = create_sym(prot, depth);

			addAttributes(sym->attributes, ATTR_function);
			addSymToTable(table, prot_id, sym);
			dumpToFile(symFil, *prot_id, sym, depth);
			//print_sym(* prot_id, sym);			
			astree * param = node->children[1];
			//block_num++;
			if (param->symbol == TOK_PARAM){
				printf("\n");
				printf("IN PARAM\n");
				for(size_t i = 0; i < param->children.size(); i++){
					process_node(param->children[i], depth + 1, ident_table, false, true, (string *)"");
				}
			}
			fprintf(symFil, "\n");
			break;
	    }
	    
	    case TOK_FUNCTION:
	    {
	    	//block_num = 0;
	    	printf("\n");
	    	//bool isFunc = true;
	    	astree * func = node->children[0];
	    	const string * func_id = func->children[0]->lexinfo;
	    	const string * func_type = func->lexinfo;
			
			fprintf(symFil, "The function type is: %s, and ID name is: %s\n",func_type->c_str(), func_id->c_str());
			sym = create_sym(func, depth);
			//printf("Before assigning type: %s\n", sym->type->c_str());
			sym->type = (string *) func_type;
			printf("After assigning type: %s\n", sym->type->c_str());
			functionAttr(func, sym);
			addSymToTable(table, func_id, sym);
			dumpToFile(symFil, *func_id, sym, depth);
			//print_sym(* func_id, sym);			
			astree * param = node->children[1];
			//block_num++;
			if (param->symbol == TOK_PARAM){
				//block_num++;
				printf("\n");
				printf("IN PARAM\n");
				//addAttributes(sym->attributes, ATTR_param);
				for(size_t i = 0; i < param->children.size(); i++){
					process_node(param->children[i], depth + 1, ident_table, false, true, (string *) "NONE");
				}
				fprintf(symFil, "\n");
			}
			astree * block = node->children[2];
			if (block->symbol == TOK_BLOCK){
				printf("\n");
				printf("IN BLOCK\n");
				for (size_t i = 0; i < block->children.size(); i++){
					process_node(block->children[i], depth + 1, ident_table, false, false,(string *) "");
				}
				fprintf(symFil, "\n");
			}
	    	break;
	    }
	    case TOK_BLOCK:
	    {
	    	//block_num++;
	    	symbol_stack->push_back(new symbol_table());
	    	process_node(node, depth + 1, table,
				false, false, (string *)"");

	    	break;
	    }
	    default:
	    {
	    	//classifySymbol (node, depth, table, sym, isField, isParam);
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
	//sym->type = (string *) "test";
    return sym;
}

void parse_ast(astree * root, char** argv, int optind){
	symFil = createFile(getBaseName(argv[optind]), ".sym");
	if (root->symbol != TOK_ROOT)
	{
		return;  // must be root
	}
	//symbol_table table;
	block_num = 0;
	symbol_stack = new vector<symbol_table*>();
	for ( size_t i = 0; i < root->children.size(); i++){
    	astree * child = root->children[i];
    	process_node(child, block_num, global_table, false, false, (string *)"");
    }

}


