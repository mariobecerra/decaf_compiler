/* File: parser.y
 * --------------
 * Yacc input file to generate the parser for the compiler.
 *
 * pp2: your job is to write a parser that will construct the parse tree
 *      and if no parse errors were found, print it.  The parser should 
 *      accept the language as described in specification, and as augmented 
 *      in the pp2 handout.
 */

%{

/* Just like lex, the text within this first region delimited by %{ and %}
 * is assumed to be C/C++ code and will be copied verbatim to the y.tab.c
 * file ahead of the definitions of the yyparse() function. Add other header
 * file inclusions or C++ variable declarations/prototypes that are needed
 * by your code here.
 */
#include "scanner.h" // for yylex
#include "parser.h"
#include "errors.h"

void yyerror(char *msg); // standard error-handling routine

%}

/* The section before the first %% is the Definitions section of the yacc
 * input file. Here is where you declare tokens and types, add precedence
 * and associativity options, and so on.
 */
 
/* yylval 
 * ------
 * Here we define the type of the yylval global variable that is used by
 * the scanner to store attibute information about the token just scanned
 * and thus communicate that information to the parser. 
 *
 * pp2: You will need to add new fields to this union as you add different 
 *      attributes to your non-terminal symbols.
 */
%union {
    int integerConstant;
    bool boolConstant;
    char *stringConstant;
    double doubleConstant;
    char identifier[MaxIdentLen+1]; // +1 for terminating null
    Decl *decl;
    List<Decl*> *declList;


    // Declarations by MBC
    
    Program *program;
    VarDecl *varDecl;
    // Variable
    Type *type;
    FnDecl *fnDecl;
    // Formals
    List<VarDecl*> *varDeclList;
    ClassDecl *classDecl;
    // Field_star
    // Ident_plus_comma
    // Field
    InterfaceDecl *interfDecl;
    // Prototype_star
    // Prototype
    // Var_Decl_Plus
    List<Stmt*> *stmt_plus;
    StmtBlock *stmtBlock;
    Stmt *stmt;
    IfStmt *ifStmt;
    WhileStmt *whileStmt;
    ForStmt *forStmt;
    // optional_expr
    ReturnStmt *returnStmt;
    BreakStmt *breakStmt;
    PrintStmt *printStmt;
    List<Expr*> *expr_plus_comma;
    Expr *expr;
    LValue *lValue;
    Call *call;
    // Actuals
    // Constant
    NamedType *namedType;
    List<NamedType*> *namedTypeList;

}


/* Tokens
 * ------
 * Here we tell yacc about all the token types that we are using.
 * Yacc will assign unique numbers to these and export the #define
 * in the generated y.tab.h header file.
 */
%token   T_Void T_Bool T_Int T_Double T_String T_Class 
%token   T_LessEqual T_GreaterEqual T_Equal T_NotEqual T_Dims
%token   T_And T_Or T_Null T_Extends T_This T_Interface T_Implements
%token   T_While T_For T_If T_Else T_Return T_Break
%token   T_New T_NewArray T_Print T_ReadInteger T_ReadLine

%token   <identifier> T_Identifier
%token   <stringConstant> T_StringConstant 
%token   <integerConstant> T_IntConstant
%token   <doubleConstant> T_DoubleConstant
%token   <boolConstant> T_BoolConstant


/* Non-terminal types
 * ------------------
 * In order for yacc to assign/access the correct field of $$, $1, we
 * must to declare which field is appropriate for the non-terminal.
 * As an example, this first type declaration establishes that the DeclList
 * non-terminal uses the field named "declList" in the yylval union. This
 * means that when we are setting $$ for a reduction for DeclList ore reading
 * $n which corresponds to a DeclList nonterminal we are accessing the field
 * of the union named "declList" which is of type List<Decl*>.
 * pp2: You'll need to add many of these of your own.
 */

%type <program>         Program
%type <declList>        DeclList Field_star Prototype_star
%type <decl>            Decl Field Prototype
%type <varDecl>         VariableDecl Variable
%type <fnDecl>          FunctionDecl
%type <type>            Type
%type <varDeclList>     Formals VarList Var_Decl_plus
%type <classDecl>       ClassDecl
%type <interfDecl>      InterfaceDecl
%type <stmt_plus>       Stmt_plus
%type <stmtBlock>       StmtBlock
%type <stmt>            Stmt
%type <ifStmt>          IfStmt
%type <whileStmt>       WhileStmt
%type <forStmt>         ForStmt
%type <returnStmt>      ReturnStmt
%type <breakStmt>       BreakStmt
%type <printStmt>       PrintStmt
%type <expr_plus_comma> Expr_plus_comma Actuals
%type <expr>            Expr Optional_Expr Constant
%type <lValue>          LValue
%type <call>            Call


/*  Precedence and associativity 
    Operators are declared in increasing order of precedence.
    All operators declared on the same line are at the same precedence level.
    (O'Riley Flex and Bison page 155)
*/


%nonassoc IF_NO_ELSE
%nonassoc T_Else
%nonassoc '='
%left T_Or
%left T_And
%left T_Equal T_NotEqual
%nonassoc T_LessEqual T_GreaterEqual '<' '>'
%left '+' '-'
%left '*' '/' '%'
%right '!' UMINUS /* UMINUS: pseudotoken standing for unary minus. (O'Riley Flex and Bison page 60) */
%nonassoc '[' '.'


%%
/* Rules
 * -----
 * All productions and actions should be placed between the start and stop
 * %% markers which delimit the Rules section.
	 
 */



Program           :   DeclList            
                      { 
                        @1; 
                        /* pp2: The @1 is needed to convince 
                         * yacc to set up yylloc. You can remove 
                         * it once you have other uses of @n*/
                        Program *program = new Program($1);
                        // if no errors, advance to next phase
                        if (ReportError::NumErrors() == 0) 
                            program->Print(0);
                      }
                  ;



DeclList          :   DeclList Decl       
                      { 
                        ($$=$1)->Append($2); 
                      }
                  |   Decl                
                      { 
                        ($$ = new List<Decl*>)->Append($1); 
                      }
                  ;



Decl              :   VariableDecl        
                      {
                        /* actions */  
                      } 
                  |   FunctionDecl        
                      { 
                        /* actions */  
                      } 
                  |   ClassDecl           
                      {
                        /* actions */  
                      }
                  |   InterfaceDecl       
                      {
                        /* actions */  
                      }
                  ;



VariableDecl      :   Variable ';'        
                      {
                        /* actions */ 
                      }



Variable          :   Type T_Identifier   
                      {
                        /* actions */  
                      }



Type              :   T_Int               
                      {
                        /* actions */  
                      }
                  |   T_Double            
                      {
                        /* actions */   
                      }
                  |   T_Bool              
                      {
                        /* actions */  
                      }
                  |   T_String            
                      {
                        /* actions */
                      }
                  |   T_Identifier        
                      {
                        /* actions */  
                      }
                  |   Type T_Dims /* '[]' is declared as T_Dims */ 
                      {
                        /* actions */  
                      }



FunctionDecl      :   Type T_Identifier '(' Formals ')' StmtBlock 
                      {
                          /* actions */  
                      }
                  |   T_Void T_Identifier '(' Formals ')' StmtBlock 
                      {
                        /* actions */   
                      }
                  ;



Formals           :   VarList
                      {
                        /* actions */   
                      }
                  |   /* epsilon */
                      {
                        /* actions */   
                      }
                  ;



VarList           :   Variable {}
                  |   VarList ',' Variable {}
                  ;



ClassDecl         :   T_Class T_Identifier T_Extends T_Identifier T_Implements Ident_plus_comma '{' Field_star '}'
                      {
                        /* actions */
                      }
                  |   T_Class T_Identifier T_Implements Ident_plus_comma '{' Field_star '}'
                      {
                        /* actions */
                      }
                  |   T_Class T_Identifier T_Extends T_Identifier '{' Field_star '}'
                      {
                        /* actions */
                      }
                  |   T_Class T_Identifier '{' Field_star '}'
                      {
                        /* actions */
                      }
                  ;



Field_star        :   /* epsilon */ {}
                  |   Field_star Field {}
                  ;       



Ident_plus_comma  :   T_Identifier
                  |   Ident_plus_comma ',' T_Identifier
                  ;                                 



Field             :   VariableDecl 
                      {
                        /* actions */
                      }
                  |   FunctionDecl
                      {
                        /* actions */
                      }
                  ;



InterfaceDecl     :   T_Interface T_Identifier '{' Prototype_star '}'
                      {
                        /* actions */
                      }
                  ;



Prototype_star    :   /* epsilon */ {}
                  |   Prototype_star Prototype {}
                  ;



Prototype         :   Type T_Identifier '(' Formals ')' ';' 
                      {
                        /* actions */
                      }
                  |   T_Void T_Identifier '(' Formals ')' ';'
                      {
                        /* actions */
                      }
                  ;



Var_Decl_plus     :   VariableDecl {}
                  |   Var_Decl_plus VariableDecl {}
                  ;



Stmt_plus         :   Stmt {}
                  |   Stmt_plus Stmt {}
                  ;



StmtBlock         :   '{' Var_Decl_plus Stmt_plus '}'
                      {
                        /* actions */
                      }
                  |   '{' Stmt_plus '}'
                      {
                        /* actions */
                      }
                  |   '{' Var_Decl_plus '}'
                      {
                        /* actions */
                      }
                  |   '{' '}'
                      {
                        /* actions */
                      }
                  ;



Stmt              :   ';'
                      {
                          /* actions */
                      }
                  |   Expr ';'
                      {
                          /* actions */
                      }
                  |   IfStmt
                      {
                          /* actions */
                      }
                  |   WhileStmt
                      {
                          /* actions */
                      }
                  |   ForStmt
                      {
                          /* actions */
                      }
                  |   BreakStmt
                      {
                          /* actions */
                      }
                  |   ReturnStmt
                      {
                          /* actions */
                      }
                  |   PrintStmt
                      {
                          /* actions */
                      }
                  |   StmtBlock
                      {
                          /* actions */
                      }
                  ;



IfStmt            :   T_If '(' Expr ')' Stmt %prec IF_NO_ELSE
                      {
                        /* actions */
                      }
                  |   T_If '(' Expr ')' Stmt T_Else Stmt
                      {
                        /* actions */
                      }
                  ;



WhileStmt         :   T_While '(' Expr ')' Stmt
                      {
                        /* actions */
                      }
                  ;



ForStmt           :   T_For '(' Optional_Expr ';' Expr ';' Optional_Expr ')' Stmt
                      {
                        /* actions */
                      }
                  ;



Optional_Expr     :   /* epsilon */ {}
                  |   Expr {}
                  ;



ReturnStmt        :   T_Return Optional_Expr ';'
                      {
                        /* actions */
                      }
                  ;



BreakStmt         :   T_Break ';'
                      {
                        /* actions */
                      }
                  ;



PrintStmt         :   T_Print '(' Expr_plus_comma ')' ';'
                      {
                        /* actions */
                      }
                  ;



Expr_plus_comma   :   Expr {}
                  |   Expr_plus_comma ',' Expr {}
                  ;



Expr              :   LValue '=' Expr 
                      {
                        /* actions */
                      }
                  |   Constant
                      {
                        /* actions */
                      }
                  |   LValue
                      {
                        /* actions */
                      }
                  |   T_This 
                      {
                        /* actions */
                      }
                  |   Call 
                      {
                        /* actions */
                      }
                  |   '(' Expr ')'
                      {
                        /* actions */
                      }
                  |   Expr '+' Expr
                      {
                        /* actions */
                      }
                  |   Expr '-' Expr
                      {
                        /* actions */
                      }
                  |   Expr '*' Expr
                      {
                        /* actions */
                      }
                  |   Expr '/' Expr
                      {
                        /* actions */
                      }
                  |   Expr '%' Expr
                      {
                        /* actions */
                      }
                  |   '-' Expr %prec UMINUS
                      {
                        /* actions */
                      }
                  |   Expr '<' Expr
                      {
                        /* actions */
                      }
                  |   Expr T_LessEqual Expr
                      {
                        /* actions */
                      }
                  |   Expr '>' Expr
                      {
                        /* actions */
                      }
                  |   Expr T_GreaterEqual Expr
                      {
                        /* actions */
                      }
                  |   Expr T_Equal Expr
                      {
                        /* actions */
                      }
                  |   Expr T_NotEqual Expr
                      {
                        /* actions */
                      }
                  |   Expr T_And Expr
                      {
                        /* actions */
                      }
                  |   Expr T_Or Expr
                      {
                        /* actions */
                      }
                  |   '!' Expr
                      {
                        /* actions */
                      }
                  |   T_ReadInteger '(' ')'
                      {
                        /* actions */
                      }
                  |   T_ReadLine '(' ')'
                      {
                        /* actions */
                      }
                  |   T_New '(' T_Identifier ')'
                      {
                        /* actions */
                      }
                  |   T_NewArray '(' Expr ',' Type ')'
                      {
                        /* actions */
                      }
                  ;



LValue            :   T_Identifier
                      {
                        /* actions */
                      }
                  |   Expr '.' T_Identifier 
                      {
                        /* actions */
                      }
                  |   Expr '[' Expr ']'
                      {
                        /* actions */
                      }
                  ;



Call              :   T_Identifier '(' Actuals ')'
                      {
                        /* actions */
                      }
                  |   Expr '.' T_Identifier '(' Actuals ')'
                      {
                        /* actions */
                      }
                  ;



Actuals           :   Expr_plus_comma
                      {
                        /* actions */
                      }
                  |   /* epsilon */
                      {
                        /* actions */
                      }
                  ;



Constant          :   T_IntConstant
                      {
                        /* actions */
                      }
                  |   T_DoubleConstant
                      {
                        /* actions */
                      }
                  |   T_BoolConstant
                      {
                        /* actions */
                      }
                  |   T_StringConstant
                      {
                        /* actions */
                      }
                  |   T_Null
                      {
                        /* actions */
                      }
                  ;




%%

/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated y.tab.c file.
 * This section is where you put definitions of helper functions.
 */

/* Function: InitParser
 * --------------------
 * This function will be called before any calls to yyparse().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the parser (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yydebug that controls whether yacc prints debugging information about
 * parser actions (shift/reduce) and contents of state stack during parser.
 * If set to false, no information is printed. Setting it to true will give
 * you a running trail that might be helpful when debugging your parser.
 * Please be sure the variable is set to false when submitting your final
 * version.
 */
void InitParser()
{
   PrintDebug("parser", "Initializing parser");
   yydebug = false;
}
