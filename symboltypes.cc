//Ryan Schreiber & Fedor Nikolayev
//Tried my best to type check
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


//FILE * symFil;
symbol_table ident_table;
symbol_table global_table;
vector<symbol_table*>* symbol_stack;
//int blockNum;
vector<int> currBlockNum;

//check if the two bitset attributes are the same
bool typeCheck(attr_bitset& first, attr_bitset& second, int atr){
    return first[atr] == second[atr];
}

//boolean statement to check if the id already exists in the statement
bool existsInTable(symbol_table& table, const string* id){

  string* newId = (string*) intern_stringset(id->c_str());
  if(table.find(newId) != table.end()){
      return true;
  }else{
      return false;

  }
}

string * getNodeName(symb * symref, astree * node){
    string * name;
    if(symref->attributes[ATTR_array])
        name = (string *) node->children[0]->children[1]->lexinfo;
    else
        name = (string *) node->children[0]->children[0]->lexinfo;
    return name; 
}

//Make sure that the variable type exists in the table
void checkVariableType(symbol_table& table,
  const string* id, symb * symref){
  if(! existsInTable(table, id)){
    
    set_exitstatus(EXIT_FAILURE);
  }
}

//Add a symbol to the table and check if it already exists
//if it exists print an error.
void addSymToTable(symbol_table& table, const string* id,
    symb * symref){
  
    string * newId = (string*) intern_stringset(id->c_str());
    if(!existsInTable(table, id) || symref->attributes[ATTR_param]){
        table[newId] = symref;
    }
}

//Prepare he struct into the symbol table and print it to the .sym file.


//only when adding a struct to a symbol table
symb * structAstToSym (astree * node, int depth, symbol_table& table,
    symb * symref, string * structType, int blockNum, FILE * output){
    //block_num++;
    for ( size_t i = 1; i < node->children.size(); i++){
        astree * child = node->children[i];
        //if the child is of type struct
        if (child->symbol == TOK_IDENT) {
            symb * sym1 = create_sym(child, 0);
               const string * id_name = child->children[0]->lexinfo;
               if (child->children[0]->symbol == TOK_ARRAY){
                id_name = child->children[1]->lexinfo;
                addAttributes(sym1->attributes, ATTR_array);
            }else{
                id_name = child->children[0]->lexinfo;
            }
            addAttributes(sym1->attributes, ATTR_field);
            addAttributes(sym1->attributes, ATTR_struct);
            addAttributes(sym1->attributes, ATTR_typeid);
            sym1->type = (string *) structType;
            //addSymToTable(table, id_name, sym1);
            addSymToTable(table, id_name, sym1);
            dumpToFile(output, *id_name, sym1, depth+1);

         }else{
             process_node(child, depth + 1, table, true, false,
                   (string *) structType, blockNum, output);
               
           }
    }
    return symref;
}

//parse and add all of the proper attributes for the prototypes
symb * protAstToSym (astree * node, int depth, int attr, symb * symref,
    symbol_table& table, int blockNum, FILE * output){
    symref = create_sym(node, 0);
    astree * prot = node->children[0];
    const string * prot_id;
    if (prot->children[0]->symbol == TOK_ARRAY){
        if (prot->symbol == TOK_VOID){


            ///WORKING ON THIS
        }
        prot_id = prot->children[1]->lexinfo;
        addAttributes(symref->attributes, ATTR_array);
    }else{
        prot_id = prot->children[0]->lexinfo;
    }
    //symref = create_sym(prot, depth);

    symref->type = (string *) prot->lexinfo;
    addAttributes(symref->attributes, attr);
    functionAttr(prot, symref);
    addSymToTable(table, prot_id, symref);
    ///////////////COMMENTED OUT?????????????
    dumpToFile(output, *prot_id, symref, depth);
    blockNum++;
    currBlockNum.push_back(blockNum);
    astree * param = node->children[1];
    if (param->symbol == TOK_PARAM){
        symref->parameters = new vector<symb *>();
        symbol_stack->push_back(new symbol_table());
        for(size_t i = 0; i < param->children.size(); i++){
            symb* parameter = process_node(param->children[i],
                depth + 1, *symbol_stack->front(),
                false, true, (string *)"", currBlockNum.back(), output);
            symref->parameters->push_back(parameter);
        }
        symbol_stack->pop_back();
    }
    fprintf(output, "\n");
    return symref;
}


//parse and add all of the proper attributes for the functions
symb * funcAstToSym (astree * node, int depth, int attr, symb * symref,
    symbol_table& table, int blockNum, FILE * output){
    //call prototype first because a prototype is a subset
    //of a function.
    symref = protAstToSym (node, depth, attr, symref,
    table, blockNum, output);
    astree * block = node->children[2];
    if (block->symbol == TOK_BLOCK){
        blockNum++;
        currBlockNum.push_back(blockNum);

        blockToSym(block, depth + 1, table,
            false, false, currBlockNum.back(), output);
        }
        fprintf(output, "\n");
    return symref;
}

//parse and add all of the proper attributes for the if else statements
void ifWhileToSym(astree * node, int depth, symbol_table& table,
    bool isField, bool isParam, int blockNum, FILE * output){
    astree * block = node->children[1];
    //if (block->symbol == TOK_BLOCK){
        blockNum++;
        currBlockNum.push_back(blockNum);
    if (block->symbol == TOK_BLOCK){
        symbol_stack->push_back(new symbol_table());
        blockToSym(block, depth + 1, *symbol_stack->front(),
            isField, isParam, currBlockNum.back(), output);
    }else if(block->symbol == TOK_CALL){
        for (size_t k = 0; k < block->children.size(); k++){
            process_node(block->children[k], depth + 1, 
                table, isField, isParam,
                (string *) "", currBlockNum.back(), output);
            currBlockNum.pop_back();
        }
    }
    //}
}

//parse and add all of the proper attributes for the if else statements
void ifElseToSym(astree * node, int depth,
    bool isField, bool isParam, int blockNum, FILE * output){
    for(size_t i = 1; i < node->children.size(); i++){
        
        astree * block = node->children[i];
        //if (block->symbol == TOK_BLOCK){
        
        blockNum++;
        symbol_stack->push_back(new symbol_table());
        currBlockNum.push_back(blockNum);
        blockToSym(block, depth + 1, *symbol_stack->front(),
            isField, isParam, currBlockNum.back(), output);
        //}
    }
}


//parse and add all of the proper attributes for blocks
void blockToSym(astree * node, int depth, symbol_table& table,
    bool isField, bool isParam, int blockNum, FILE * output){
    symbol_stack->push_back(new symbol_table());
    for(size_t i = 0; i < node->children.size(); i++){
        //process_node(node->children[i], depth, table,
        process_node(node->children[i], depth, table,
            isField, isParam, (string *) "", blockNum, output);
    }
    symbol_stack->pop_back();

}

//we will need to check types of all of the needed variables. 
//only doing boolean so far.


//add the proper attributes for each symbol
const char* attrToStr (symb* symref){
    string attrs = "";
    if(symref->attributes[ATTR_field])
          attrs = attrs + string("field {")
          + symref->type->c_str() + "} ";
      if(symref->attributes[ATTR_void])
          attrs+="void ";
    if(symref->attributes[ATTR_bool])
          attrs+="bool ";
    if(symref->attributes[ATTR_char])
          attrs+="char ";
    if(symref->attributes[ATTR_int])
          attrs+="int ";
      if(symref->attributes[ATTR_null])
          attrs+="null ";
    if(symref->attributes[ATTR_string])
          attrs+="string ";
    if(symref->attributes[ATTR_struct])
          attrs+="struct ";
    if(symref->attributes[ATTR_typeid])
        attrs+= string("\"") + symref->type->c_str() + "\" ";
    if(symref->attributes[ATTR_array])
        attrs+="array ";
    if(symref->attributes[ATTR_prototype])
        attrs+="prototype ";
    if(symref->attributes[ATTR_function])
        attrs+="function ";
    if(symref->attributes[ATTR_variable])
        attrs+="variable ";
    if(symref->attributes[ATTR_param])
        attrs+="param ";
    if(symref->attributes[ATTR_lval])
        attrs+="lval ";
    if(symref->attributes[ATTR_const])
        attrs+="const ";
    if(symref->attributes[ATTR_vreg])
        attrs+="vreg ";
    if(symref->attributes[ATTR_vaddr])
        attrs+="vaddr ";
    return attrs.c_str();
}

//function that dumps to the output file
void dumpToFile(FILE* symFile, string lexinfo, symb* symref, int depth){
    for(int i = 0; i<depth; i++) fprintf(symFile, "    ");
    if (!symref->attributes[ATTR_field]){
        fprintf(symFile, "%s (%ld.%ld.%ld) {%ld} %s\n", lexinfo.c_str(),
            symref->filenr, symref->linenr,
            symref->offset, symref->blocknr,
            attrToStr(symref));
    }else{

        fprintf(symFile, "%s (%ld.%ld.%ld) %s\n", lexinfo.c_str(),
            symref->filenr, symref->linenr, symref->offset,
            attrToStr(symref));
    }
}

//check if equal
void checkEqual (astree * node, symb * symref,
    size_t depth, symbol_table& table,
    int blockNum, FILE * output){
    if (node->children.size() > 1){
        //check if for the symbol there is a boolean attribute.
        if (symref->attributes[ATTR_bool]){
            symb * con = process_node (node->children[1], depth, table,
                false, false, (string *) "", blockNum, output);
            bool equal = typeCheck(symref->attributes,
            con->attributes, ATTR_bool);
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

//Add the proper attributes
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



//function that classifies the Idents
symb * classifyIdent (astree * node, size_t depth, symbol_table& table,
    symb * symref, bool isField, bool isParam,
    int blockNum, FILE * output){
    symref = create_sym(node, blockNum);
    if(node->children.size() != 0){
        const string * id_name;
        if(node->children[0]->symbol == TOK_ARRAY){
            id_name = node->children[1]->lexinfo;
            addAttributes(symref->attributes, ATTR_array);
        }else{
            id_name = node->children[0]->lexinfo;
        }
        symref->type = (string *) node->lexinfo;
        addAttributes(symref->attributes, ATTR_struct);
        addAttributes(symref->attributes, ATTR_typeid);
        performAttr(symref, isField, isParam);
        addSymToTable(table, id_name, symref);
        dumpToFile(output, *id_name, symref, depth);
    }

    return symref;
}

//function that classifies other types
symb * classifySymbol (astree * node, size_t depth, symbol_table& table,
    symb* symref, int attr, bool isField, bool isParam,
    string * structType, int blockNum, FILE * output){
    symref = create_sym(node, blockNum);
    const string * id_name;
    if (node->children[0]->symbol == TOK_ARRAY){
        id_name = node->children[1]->lexinfo;
        addAttributes(symref->attributes, ATTR_array);
    }else{
        id_name = node->children[0]->lexinfo;
    }
    addAttributes(symref->attributes, attr);
    performAttr(symref, isField, isParam);
    addSymToTable(table, id_name, symref);
    symref->type = (string *) structType;
    dumpToFile(output, *id_name, symref, depth);
    return symref;
}


//Add attributes for a function
void functionAttr(astree * func, symb * symref){
    //addAttributes(symref->attributes, ATTR_function);
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
            eprintf("Cannot have function of type %s.\n",
            get_yytname(func->symbol));
            break;
        }
    }
}


//Create needed symbol tables
symb * process_node(astree * node, size_t depth, symbol_table& table,
    bool isField, bool isParam, string * structType, int blockNum,
    FILE * output){

    symb * symref = NULL;
    switch (node->symbol){

        case TOK_BLOCK:
        {
            blockToSym(node, depth, table, isField,
            isParam, blockNum, output);
            break;
        }
        case TOK_TRUE:
        case TOK_FALSE:
        {
            symref = create_sym (node,depth);
            symref->attributes[ATTR_bool] = true;
            symref->blocknr = blockNum;
            node->sym=symref;

            break;
        }
        case TOK_VARDECL:
        {
            astree * type = node->children[0];
            symref = process_node (type, depth, table, isField, isParam,
                (string *) structType, blockNum, output);
            //string * var = (string *) getNodeName(symref, node);
            break;
        }
        case TOK_IDENT:
        {
            symref = classifyIdent (node, depth, table, symref,
                isField, isParam, blockNum, output);
            break;
        }
        case TOK_INT:
        {
            symref = classifySymbol (node, depth, table, symref,
            ATTR_int, isField, isParam, (string *) structType,
            blockNum, output);
            break;
        }
        case TOK_CHAR:
        {
            symref = classifySymbol (node, depth, table,
            symref, ATTR_char,
            isField, isParam, (string *) structType, blockNum, output);
            break;
        }
        case TOK_BOOL:
        {
            symref = classifySymbol (node, depth, table,
                symref, ATTR_bool,
                isField, isParam, (string *) structType,
                blockNum, output);
            break;
        }
        case TOK_STRING:
        {
            symref = classifySymbol (node, depth, table,
                symref, ATTR_string, isField, isParam,
                (string *) structType, blockNum,
                output);
            break;
        }
        case TOK_VOID:
        {
            symref = classifySymbol (node, depth, table,
                symref, ATTR_string,
                isField, isParam, (string *) structType,
                blockNum, output);
            node->children[0]->sym = symref;
            break;
        }
        case TOK_TYPEID:
        {
            const string * struct_id;
            if (node->children[0]->symbol == TOK_ARRAY){
                struct_id = node->children[1]->lexinfo;
                addAttributes(symref->attributes, ATTR_array);
            }else{
                struct_id = node->children[0]->lexinfo;
            }
            symref = create_sym(node, 0);
            symref->type=(string *) struct_id;
            addAttributes(symref->attributes, ATTR_struct);
            addAttributes(symref->attributes, ATTR_typeid);
            symref->type = (string *) struct_id;
            symref->fields = new symbol_table ();
            dumpToFile(output, *struct_id, symref, depth);
            addSymToTable(table, struct_id, symref);

            symref = structAstToSym (node, depth,
            *symref->fields, symref,
            (string *) struct_id, blockNum, output);
            fprintf(output, "\n");

            break;
        }
        
        case TOK_PROTOTYPE:
        {
            symref = protAstToSym (node, depth, 
            ATTR_prototype, symref, table, blockNum, output);
            break;
        }

        case TOK_FUNCTION:
        {
            funcAstToSym (node, depth,
            ATTR_function, symref, table, blockNum, output);
            
            break;
        }
        case TOK_IFELSE:
        {
            ifElseToSym(node, depth, isField,
            isParam, blockNum, output);
            break;
        }
        case TOK_WHILE:
        case TOK_IF:
        {
            ifWhileToSym(node, depth,
            table, isField, isParam, blockNum, output);
            break;
        }
        case TOK_CALL:
        {
            symref = create_sym(node, blockNum);
            break;
        }
        default:
        {
            //create_sym (node, blockNum);
            break;
        }
    }
    return symref;
}

//function to add an attribute.
void addAttributes(attr_bitset& sym_attribute, int attribute){
    sym_attribute[attribute] = true;
}


//Function to set up the symbol 
symb * create_sym (astree * node, size_t blcknr){
    symb * symref = new symb();
    symref->filenr = node->filenr;
    symref->linenr = node->linenr;
    symref->offset = node->offset;
    symref->blocknr = blcknr;
    //symref->type= NULL;
    symref->fields = NULL;
    symref->parameters = NULL;
    //symref->type = (string *) "test";
    return symref;
}

//void parse_ast(astree * root, char** argv, int optind){
//main function to start the parsing the ast
void parse_ast(FILE * output, astree * root){
    //symFil = createFile(getBaseName(argv[optind]), ".sym");
    if (root->symbol != TOK_ROOT)
    {
        eprintf("The astree needs to start with a TOK_ROOT\n");
    }
    int blockNum = 0;
    currBlockNum = vector<int>();
    currBlockNum.push_back(blockNum);
    symbol_stack = new vector<symbol_table*>();
    for ( size_t i = 0; i < root->children.size(); i++){
        astree * child = root->children[i];
        process_node(child, blockNum, global_table,
            false, false, (string *)"", blockNum,
            output);

    }

}


