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
int blockNum;
vector<int> curBlock;

void print_sym(string lexinfo, symb * symref){
    printf("%s (%ld.%ld.%ld) {%ld} {%s}\n", lexinfo.c_str(),
        symref->filenr, symref->linenr, symref->offset, symref->blocknr,
        symref->attributes[ATTR_field] ? "field" : "not a field");

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

void checkVariableType(symbol_table& table, const string* id, symb * symref){
  if(! existsInTable(table, id)){
    eprintf("%s:%ld:%ld error: Invalid variable declaration:\n\
            '%s' is not a type.\n",
            included_filenames[symref->filenr].c_str(), symref->linenr,
            symref->offset, id->c_str());
    set_exitstatus(EXIT_FAILURE);
  }
}

//added this one need to 
void addSymToTable(symbol_table& table, const string* id, symb * symref){
  
  string * newId = (string*) intern_stringset(id->c_str());
  if(!existsInTable(table, id)){
    table[newId] = symref;
    printf("Added symbol to table[%s]\n", newId->c_str());
    //DEBUGF('s', "\tAdded symbol to table[%s]\n", newId->c_str());
  }else{
    eprintf("%s:%d:%d error: redeclaration of '%s'\n\
             previously declared here: %s %d:%d\n",
            included_filenames[symref->filenr].c_str(), (int) symref->linenr,
            (int) symref->offset, newId->c_str(),
            included_filenames[table[newId]->filenr].c_str(),
            (int) table[newId]->linenr,
            (int) table[newId]->offset
    );
    set_exitstatus(EXIT_FAILURE);
  }
}
///////////////////////////////////////////////////////////////////////////////////
//only when adding a struct to a symbol table
void structAstToSym (astree * node, int depth, symb * symref, symbol_table& table, string * structType){
	//blockNum++;
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
   	    	//symb * sym1 = create_sym(child, depth + 1);
   	    	symb * sym1 = create_sym(child, 0);
            addAttributes(sym1->attributes, ATTR_field);
			addAttributes(sym1->attributes, ATTR_struct);
			addAttributes(sym1->attributes, ATTR_typeid);
            //printf("Before assigning type: %s\n", sym1->type->c_str());
            sym1->type =(string *) structType;
            printf("After assigning type: %s\n", sym1->type->c_str());
            addSymToTable(table,id_name, sym1);
            //table[(string *)id_name] = sym1;
            dumpToFile(symFil, *id_name, sym1, depth+1);
           // print_sym(* id_name, sym1);
 	    } else {
   	    	//process_node(child, depth + 1, *sym->fields, true, false);
   	    	process_node(child, depth + 1, *symref->fields, true, false, (string *) structType);
   	    }
    }
}

void protAstToSym (astree * node, int depth, int attr, symb * symref,
	symbol_table& table){
				//block_num = 0;
	printf("\n");
	astree * prot = node->children[0];
	const string * prot_id = prot->children[0]->lexinfo;
	symref = create_sym(prot, depth);

	symref->type = (string *) prot->lexinfo;
	addAttributes(symref->attributes, attr);
	functionAttr(prot, symref);
	addSymToTable(table, prot_id, symref);
	dumpToFile(symFil, *prot_id, symref, depth);

	///////adding new part//////////////
	//blockNum++;
	//curBlock.push_back(blockNum);
	//symbol_stack->push_back(new symbol_table());
	////////////////////////////////////
	astree * param = node->children[1];
	//block_num++;
	if (param->symbol == TOK_PARAM){
		printf("\n");
		printf("IN PARAM\n");
		for(size_t i = 0; i < param->children.size(); i++){
			//symb * parm = process_node(param->children[i], curBlock.back(), ident_table, false, true, (string *)"");
			process_node(param->children[i], depth + 1, ident_table, false, true, (string *)"");

			//checkVariableType(table, param->children[i]->lexinfo, parm);
			//symref->parameters->push_back(parm);

		}
	}
	fprintf(symFil, "\n");

}


void funcAstToSym (astree * node, int depth, int attr, symb * symref,
	symbol_table& table){
	protAstToSym (node, depth, attr, symref, table);
	astree * block = node->children[2];
	if (block->symbol == TOK_BLOCK){
		printf("\n");
		printf("IN BLOCK\n");
		for (size_t i = 0; i < block->children.size(); i++){
			process_node(block->children[i], depth + 1, ident_table, false, false,(string *) "");
		}
		fprintf(symFil, "\n");
	}

}

void ifElseToSym(astree * node, int depth, int attr, symb * symref,
	symbol_table& table){
	astree * cond = node->children[0];
	astree * ifBlock = node->children[1];
	astree * elseBlock = node->children[2];
}

//we will need to check types of all of the needed variables. 
//only doing boolean so far.



const char* attrToStr (symb* symref){
//void attrToStr (symb* sym){
	//printf ("attr to string SYMBOL TYPE is: %s\n", sym->type->c_str()); 
  string attrs = "";
  if(symref->attributes[ATTR_field]){
  	attrs = attrs + "field {" +symref->type->c_str()+"} ";
  	//string * type = sym->type;
  	//printf ("attr to string SYMBOL TYPE is:\n"); //sym->type->c_str());
  }
  if(symref->attributes[ATTR_void]) attrs+="void ";
  if(symref->attributes[ATTR_bool]) attrs+="bool ";
  if(symref->attributes[ATTR_char]) attrs+="char ";
  if(symref->attributes[ATTR_int]) attrs+="int ";
  if(symref->attributes[ATTR_null]) attrs+="null ";
  if(symref->attributes[ATTR_string]) attrs+="string ";
  if(symref->attributes[ATTR_struct]) attrs+="struct ";
  if(symref->attributes[ATTR_typeid]){
  	attrs+= string("\"") + symref->type->c_str() + "\" ";
  }
  if(symref->attributes[ATTR_array]) attrs+="array ";
  if(symref->attributes[ATTR_function]) attrs+="function ";
  if(symref->attributes[ATTR_prototype]) attrs+="prototype ";
  if(symref->attributes[ATTR_variable]) attrs+="variable ";
  if(symref->attributes[ATTR_param]) attrs+="param ";
  if(symref->attributes[ATTR_lval]) attrs+="lval ";
  if(symref->attributes[ATTR_const]) attrs+="const ";
  if(symref->attributes[ATTR_vreg]) attrs+="vreg ";
  if(symref->attributes[ATTR_vaddr]) attrs+="vaddr ";
  return attrs.c_str();
}


void dumpToFile(FILE* symFile, string lexinfo, symb* symref, int depth){
	for(int i = 0; i<depth; i++) fprintf(symFile, "    ");
	if (!symref->attributes[ATTR_field]){
		fprintf(symFile, "%s (%ld.%ld.%ld) {%ld} %s\n", lexinfo.c_str(),
			symref->filenr, symref->linenr, symref->offset, symref->blocknr,
			attrToStr(symref));
	}else{

		fprintf(symFile, "%s (%ld.%ld.%ld) %s\n", lexinfo.c_str(),
			symref->filenr, symref->linenr, symref->offset,
			attrToStr(symref));
	}
}

void checkEqual (astree * node, symb * symref, size_t depth, symbol_table& table){
	if (node->children.size() > 1){
		printf("Need to check equivalance.\n");
		//check if for the symbol there is a boolean attribute.
		if (symref->attributes[ATTR_bool]){
			symb * con = process_node (node->children[1], depth, table, false, false, (string *) "");
			bool equal = typeCheck(symref->attributes, con->attributes, ATTR_bool);
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

void performAttr(symb * symref, bool isField, bool isParam){
	if (isField){
		addAttributes(symref->attributes,ATTR_field);
		return;
	}
	else if(isParam){
		addAttributes(symref->attributes,ATTR_param);
		addAttributes(symref->attributes,ATTR_lval);
		addAttributes(symref->attributes,ATTR_variable);
	}
	else
	{
		addAttributes(symref->attributes,ATTR_lval);
		addAttributes(symref->attributes,ATTR_variable);
	}
}

void classifyIdent (astree * node, size_t depth, symbol_table& table,
	symb * symref, bool isField, bool isParam){
	const string * id_name = node->children[0]->lexinfo;
	printf("id_name is: %s\n", id_name->c_str());
	symref = create_sym(node, depth);
	symref->type = (string *) node->lexinfo;
	addAttributes(symref->attributes, ATTR_struct);
	addAttributes(symref->attributes, ATTR_typeid);
	performAttr(symref, isField, isParam);

	addSymToTable(table, id_name, symref);
	dumpToFile(symFil, *id_name, symref, depth);

	printf("\n");
}

void classifySymbol (astree * node, size_t depth, symbol_table& table,
	symb* symref, int attr, bool isField, bool isParam, string * structType){
	const string * id_name = node->children[0]->lexinfo;
	printf("id_name is: %s\n", id_name->c_str());//, key->c_str());
	symref = create_sym(node, depth);
	addAttributes(symref->attributes, attr);
	performAttr(symref, isField, isParam);
	addSymToTable(table, id_name, symref);
	symref->type = (string *) structType;
	dumpToFile(symFil, *id_name, symref, depth);
	printf("\n");
}
/*
void classifyConst (astree * node, size_t depth, symbol_table& table,
	symb* symref, int attr, bool isField, bool isParam, string * structType){

	addAttributes(symref->attributes, ATTR_int);
	addAttributes(symref->attributes, ATTR_const);
	node->sym = symref;

}
*/

void functionAttr(astree * func, symb * symref){
	addAttributes(symref->attributes, ATTR_function);
	switch (func->symbol){
		case TOK_IDENT:
		{
			addAttributes(symref->attributes, ATTR_struct);
			addAttributes(symref->attributes, ATTR_typeid);
			break;
	    }
		case TOK_INT:
		{
			addAttributes(symref->attributes, ATTR_int);
	        break;
		}
		case TOK_CHAR:
		{
			addAttributes(symref->attributes, ATTR_char);
	        break;
		}
		case TOK_BOOL:
		{
			addAttributes(symref->attributes, ATTR_bool);
	        break;
		}
		case TOK_STRING:
		{
			addAttributes(symref->attributes, ATTR_string);
			break;
		}
		case TOK_VOID:
		{
			addAttributes(symref->attributes, ATTR_void);
			break;
		}
		default:
		{
			eprintf("Cannot have function of type %s.\n", get_yytname(func->symbol));
			break;
		}
	}
}


//Create needed symbol tables
symb * process_node(astree * node, size_t depth, symbol_table& table,
	bool isField, bool isParam, string * structType){

	symb * symref = NULL;

	switch (node->symbol){
		case TOK_TRUE:
		case TOK_FALSE:
		{
			printf("found either TRUE or FALSE\n");
			symref = create_sym (node,depth);
			symref->attributes[ATTR_bool] = true;
			node->sym=symref;
			break;
	    }
	    /*
	    case TOK_INTCON:
	    {

			break;
	    }
	    case TOK_CHARCON:
	    {
			addAttributes(symref->attributes, ATTR_char);
			addAttributes(symref->attributes, ATTR_const);
			node->sym = symref;
			break;
	    }
		case TOK_STRINGCON:
		{
			addAttributes(symref->attributes, ATTR_char);
			addAttributes(symref->attributes, ATTR_const);
			node->sym = symref;
			break;
	    }
	    */
		case TOK_VARDECL:
		{
	    	printf("VarDecl\n");
	    	symref = process_node (node->children[0], depth, table, isField, isParam, (string *) structType);
	    	//printf("going into constant symbol (to the right of the = sign)\n");
	    	
	    	//checkEqual (node,sym,depth,table);
	    	//print_sym(* id_name, sym);
	    	
	    	break;
		}
		case TOK_IDENT:
		{
			classifyIdent (node, depth, table, symref, isField, isParam);
			break;
	    }
		case TOK_INT:
		{
			classifySymbol (node, depth, table, symref, ATTR_int, isField, isParam, (string *) structType);
	        break;
		}
		case TOK_CHAR:
		{
			classifySymbol (node, depth, table, symref, ATTR_char, isField, isParam, (string *) structType);
	        break;
		}
		case TOK_BOOL:
		{
			classifySymbol (node, depth, table, symref, ATTR_bool, isField, isParam, (string *) structType);
	        break;
		}
		case TOK_STRING:
		{
			classifySymbol (node, depth, table, symref, ATTR_string, isField, isParam, (string *) structType);
			break;
		}
		case TOK_VOID:
		{
			classifySymbol (node, depth, table, symref, ATTR_string, isField, isParam, (string *) structType);
			node->children[0]->sym = symref;
			break;
		}
		case TOK_TYPEID:
		{
			const string * struct_id = node->children[0]->lexinfo;
			symref = create_sym(node, 0);
			symref->type=(string *) struct_id;
			addAttributes(symref->attributes, ATTR_struct);
			addAttributes(symref->attributes, ATTR_typeid);
			symref->type = (string *) struct_id;
			printf("After assigning type: %s\n", symref->type->c_str());
			symref->fields = new symbol_table ();
			dumpToFile(symFil, *struct_id, symref, depth);
			printf("struct_id is: %s\n", struct_id->c_str());
			addSymToTable(table, struct_id, symref);
			structAstToSym (node, depth, symref, table, (string *) struct_id);
			fprintf(symFil, "\n");

	    	break;
	    }
	    
	    case TOK_PROTOTYPE:
	    {
	    	protAstToSym (node, depth, ATTR_prototype, symref, table);
			break;
	    }

	    case TOK_FUNCTION:
	    {
	    	funcAstToSym (node, depth, ATTR_function, symref, table);
	    	break;
	    }
	    case TOK_BLOCK:
	    {
	    	//block_num++;
	    	//symbol_stack->push_back(new symbol_table());
	    	process_node(node, depth + 1, table,
				false, false, (string *)"");

	    	break;
	    }
	    case TOK_IFELSE:
	    {

	    	break;
	    }
	    default:
	    {
	    	//classifySymbol (node, depth, table, sym, isField, isParam);
			printf("Found Token that is not handled yet %s with the TOK called: %s\n",
				node->lexinfo->c_str(), get_yytname (node->symbol));
			break;
		}
    }
	return symref;
}

void addAttributes(attr_bitset& sym_attribute, int attribute){
	sym_attribute[attribute] = true;
}

symb * create_sym (astree * node, size_t depth){
	symb * symref = new symb();

	symref->filenr = node->filenr;
	symref->linenr = node->linenr;
	symref->offset = node->offset;
    symref->blocknr = depth;
    symref->fields = NULL;
	symref->parameters = NULL;
	//symref->type = (string *) "test";
    return symref;
}

void parse_ast(astree * root, char** argv, int optind){
	symFil = createFile(getBaseName(argv[optind]), ".sym");
	if (root->symbol != TOK_ROOT)
	{
		return;  // must be root
	}
	blockNum = 0;
	curBlock = vector<int>();
	curBlock.push_back(blockNum);
	//symbol_table table;

	symbol_stack = new vector<symbol_table*>();
	for ( size_t i = 0; i < root->children.size(); i++){
    	astree * child = root->children[i];
    	process_node(child, blockNum, global_table, false, false, (string *)"");
    }

}


