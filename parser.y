%{
// Parser for scanner project.
// Ryan Schreiber & Fedor Nikolayev

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose
%destructor { error_destructor ($$); } <>

%token IDENT DIGIT
%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_FUNCTION TOK_ORD TOK_CHR TOK_ROOT TOK_RETURNVOID
%token TOK_NEWSTRING TOK_INDEX TOK_DECLID 
%token TOK_PARAM TOK_PROTOTYPE TOK_VARDECL


%right  TOK_IF TOK_ELSE TOK_IFELSE
%right  '='
%left   TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left   '+' '-'
%left   '*' '/' '%'
%right  TOK_POS TOK_NEG '!' TOK_ORD TOK_CHR
%left   '[' ']' '.' '(' ')'
%nonassoc TOK_NEW

%start  start

%%

start       : program                   { yyparse_astree = $1; }
            ;

program     : program structdef         { $$ = adopt1 ($1, $2);}
            | program function          { $$ = adopt1 ($1, $2);}
            | program statement         { $$ = adopt1 ($1, $2); }
            | program error '}'         { $$ = $1; }
            | program error ';'         { $$ = $1; }
            |                           { $$ = new_parseroot(); }
            ;

structdef   : contstruct '}'            { $$ = $1; free_ast($2); }
            ;

contstruct  : contstruct fielddecl ';'  {free_ast($3); 
                                            $$ = adopt1($1, $2);
                                        }
            | TOK_STRUCT TOK_IDENT '{'  {$$ = adopt1sym($1,
                                            $2, TOK_TYPEID);
                                            free_ast($3);}
            ;

fielddecl   : basetype TOK_IDENT        { $$ = adopt1 ($1, changeSym($2,
                                            TOK_FIELD)); }
            | basetype TOK_ARRAY TOK_IDENT 
                                        { $$ = adopt2 ($2, $1,
                                            changeSym($3, TOK_FIELD)); }
            ;


basetype    : TOK_VOID                  { $$ = $1; }
            | TOK_BOOL                  { $$ = $1; }
            | TOK_CHAR                  { $$ = $1; }
            | TOK_INT                   { $$ = $1; }
            | TOK_STRING                { $$ = $1; }
            | TOK_IDENT                 { $$ = $1; }

function    : identdecl '(' contfunc ')' block
                                        { $2 = adopt1sym($2, $3,
                                            TOK_PARAM);
                                            $$ = 
                                            createFunction($1, $2, $5);
                                            free_ast($4); }
            | identdecl '(' ')' block   { 
                                            $$ = createFunction ($1,
                                            adopt1sym($2, $3,
                                            TOK_PARAM), $4); }
            ;

contfunc    : contfunc ',' identdecl    { $$ = adopt1($1, $3); 
                                            free_ast($2);}
            | identdecl                 { $$ = $1; }      
            ;

identdecl   : basetype TOK_IDENT        { $$ = adopt1 ($1,
                                            changeSym($2,
                                            TOK_DECLID)); }
            | basetype TOK_ARRAY TOK_IDENT
                                        { $$ = adopt2 ($2, $1,
                                            changeSym($3, TOK_DECLID));
                                        }
            ;


block       : state '}'                { $$ = $1, free_ast($2); }
            |'{' '}'                   { $$ = changeSym($1,TOK_BLOCK); free_ast ($2); }
            | ';'                      { free_ast($1);}
            //{ $$ = changeSym($1,TOK_BLOCK); } 
            ;

state       : '{' statement             { $$ = adopt1(changeSym($1,TOK_BLOCK), $2); }
            | state statement           { $$ = adopt1($1,$2); }
            ;

statement   : block                     { $$ = $1; }
            | vardecl                   { $$ = $1; }
            | while                     { $$ = $1; }
            | ifelse                    { $$ = $1; }
            | return                    { $$ = $1; }
            | expr ';'                  { free_ast ($2); 
                                          $$ = $1; }
            ;

vardecl     : identdecl '=' expr ';'    { free_ast($4);
                                            $$ = adopt2 (changeSym($2,
                                            TOK_VARDECL), $1, $3);
                                        }
            ;

while       : TOK_WHILE '(' expr ')' statement
                                        { $$ = adopt2 ($1, $3, $5);
                                            free_ast2 ($2, $4); }
            ;

ifelse      : TOK_IF '(' expr ')' statement %prec TOK_ELSE 
                                        { $$ = adopt2 ($1, $3, $5);
                                            free_ast2 ($2, $4); }
            | TOK_IF '(' expr ')' statement TOK_ELSE statement
                                        { $$ = adopt2 (adopt1sym ($1,
                                              $3, TOK_IFELSE), $5, $7);
                                            free_ast2 ($2, $4);
                                            free_ast($6); }
            ;

return      : TOK_RETURN ';'            { free_ast($2);
                                            $$ = changeSym ($1,
                                            TOK_RETURNVOID);
                                        }
            | TOK_RETURN expr ';'       { free_ast ($3); 
                                            $$ = adopt1 ($1, $2);
                                        }
            ;

expr        : binopexpr                 { $$ = $1; }
            | unopexpr                  { $$ = $1; }
            | allocator                 { $$ = $1; }
            | call                      { $$ = $1; }
            | variable                  { $$ = $1; }
            | constant                  { $$ = $1; }
            | '(' expr ')'              { $$ = $2; free_ast2 ($1, $3); }
            ;

binopexpr   : expr '=' expr             { $$ = adopt2 ($2, $1, $3); }
            | expr TOK_EQ expr          { $$ = adopt2 ($2, $1, $3); }
            | expr TOK_NE expr          { $$ = adopt2 ($2, $1, $3); }
            | expr TOK_LT expr          { $$ = adopt2 ($2, $1, $3); }
            | expr TOK_LE expr          { $$ = adopt2 ($2, $1, $3); }
            | expr TOK_GT expr          { $$ = adopt2 ($2, $1, $3); }
            | expr TOK_GE expr          { $$ = adopt2 ($2, $1, $3); }
            | expr '+' expr             { $$ = adopt2 ($2, $1, $3); }
            | expr '-' expr             { $$ = adopt2 ($2, $1, $3); }
            | expr '*' expr             { $$ = adopt2 ($2, $1, $3); }
            | expr '/' expr             { $$ = adopt2 ($2, $1, $3); }
            | expr '%' expr             { $$ = adopt2 ($2, $1, $3); }
            ;

unopexpr    : '+' expr %prec TOK_POS    { $$ = adopt1sym ($1,
                                            $2, TOK_POS); }
            | '-' expr %prec TOK_NEG    { $$ = adopt1sym ($1,
                                            $2, TOK_NEG); }
            | '!' expr                  { $$ = adopt1 ($1, $2); }
            | TOK_ORD expr              { $$ = adopt1 ($1, $2); }
            | TOK_CHR expr              { $$ = adopt1 ($1, $2); }

            ;

allocator   : TOK_NEW TOK_IDENT '(' ')'  
                                        { $$ = adopt1sym ($1,
                                            $2, TOK_TYPEID);
                                            free_ast2($3, $4); }
            | TOK_NEW TOK_STRING '(' expr ')' 
                                        { $$ = adopt1sym ($1,
                                            $4, TOK_NEWSTRING);
                                            free_ast($2);
                                            free_ast2($3, $5); }
            | TOK_NEW basetype '[' expr ']'
                                        { $$ = adopt1 (adopt1sym ($1,
                                            $2, TOK_NEWARRAY), $4); 
                                            free_ast2($3, $5); }
            ;

call        : contcall ')'
                                        { $$ = $1; free_ast ($2); }
            | TOK_IDENT '(' ')'         { $$ = adopt1sym($1, $2,
                                            TOK_VOID); free_ast($3); }
            ;

contcall    : TOK_IDENT '(' expr        { $$ = adopt1 (adopt1sym ($2,
                                            $1, TOK_CALL), $3); }
            | contcall ',' expr         { $$ = adopt1 ($1, $3);
                                            free_ast ($2); }
            ;


variable    : TOK_IDENT                 { $$ = $1; }
            | expr '[' expr ']'         { $$ = adopt2(changeSym($2,
                                            TOK_INDEX), $1, $3);
                                            free_ast($4); }
            | expr '.' TOK_IDENT        { $$ = adopt2 ($2, $1,
                                            changeSym ($3, TOK_FIELD) ); }
            ;

constant    : TOK_INTCON                { $$ = $1; }
            | TOK_CHARCON               { $$ = $1; }
            | TOK_STRINGCON             { $$ = $1; }
            | TOK_FALSE                 { $$ = $1; }
            | TOK_TRUE                  { $$ = $1; }
            | TOK_NULL                  { $$ = $1; }
            ;



%%

const char* get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}
