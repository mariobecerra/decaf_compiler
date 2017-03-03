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
%type<namedTypeList>    Ident_plus_comma


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
                        $$ = $1;
                      } 
                  |   FunctionDecl        
                      { 
                        $$ = $1;
                      } 
                  |   ClassDecl           
                      {
                        $$ = $1;
                      }
                  |   InterfaceDecl       
                      {
                        $$ = $1;
                      }
                  ;



VariableDecl      :   Variable ';'        
                      {
                        $$ = $1;
                      }



Variable          :   Type T_Identifier   
                      {
                        Identifier *ident = new Identifier(@2, $2);
                        $$ = new VarDecl(ident, $1);
                      }



Type              :   T_Int               
                      {
                        $$ = Type::intType;
                      }
                  |   T_Double            
                      {
                        $$ = Type::doubleType;
                      }
                  |   T_Bool              
                      {
                        $$ = Type::boolType;
                      }
                  |   T_String            
                      {
                        $$ = Type::stringType;
                      }
                  |   T_Identifier        
                      {
                        Identifier *ident = new Identifier(@1, $1);
                        $$ = new NamedType(ident);
                      }
                  |   Type T_Dims /* '[]' is declared as T_Dims */ 
                      {
                        $$ = new ArrayType(@1, $1);
                      }



FunctionDecl      :   Type T_Identifier '(' Formals ')' StmtBlock 
                      {
                          Identifier *ident = new Identifier(@2, $2);
                          $$ = new FnDecl(ident, $1, $4);
                          $$ -> SetFunctionBody($6);
                      }
                  |   T_Void T_Identifier '(' Formals ')' StmtBlock 
                      {
                          Identifier *ident = new Identifier(@2, $2);
                          $$ = new FnDecl(ident, Type::voidType, $4);
                          $$ -> SetFunctionBody($6);
                      }
                  ;



Formals           :   VarList
                      {
                        $$ = $1;
                      }
                  |   /* epsilon */
                      {
                        $$ = new List<VarDecl*>; 
                      }
                  ;



VarList           :   Variable 
                      {
                        $$ = new List<VarDecl*>;
                        $$ -> Append($1);
                      }
                  |   VarList ',' Variable 
                      {
                        $$ -> Append($3);
                      }
                  ;



ClassDecl         :   T_Class T_Identifier T_Extends T_Identifier T_Implements Ident_plus_comma '{' Field_star '}'
                      {
                        Identifier *ident_1 = new Identifier(@2, $2);
                        Identifier *ident_2 = new Identifier(@4, $4);
                        NamedType *nt = new NamedType(ident_2);
                        $$ = new ClassDecl(ident_1, nt, $6, $8);
                      }
                  |   T_Class T_Identifier T_Implements Ident_plus_comma '{' Field_star '}'
                      { 
                        Identifier *ident = new Identifier(@2, $2);
                        $$ = new ClassDecl(ident, NULL, $4, $6);
                      }
                  |   T_Class T_Identifier T_Extends T_Identifier '{' Field_star '}'
                      {
                        Identifier *ident_1 = new Identifier(@2, $2);
                        Identifier *ident_2 = new Identifier(@4, $4);
                        NamedType *nt = new NamedType(ident_2);
                        $$ = new ClassDecl(ident_1, nt, new List<NamedType*>, $6);
                      }
                  |   T_Class T_Identifier '{' Field_star '}'
                      {
                        Identifier *ident = new Identifier(@2, $2);
                        $$ = new ClassDecl(ident, NULL, new List<NamedType*>, $4);
                      }
                  ;



Field_star        :   /* epsilon */ 
                      {
                        $$ = new List<Decl*>;
                      }
                  |   Field_star Field 
                      {
                        $$ = new List<Decl*>;
                        $$ -> Append($2);
                      }
                  ;       



Ident_plus_comma  :   T_Identifier 
                      {
                        Identifier *ident = new Identifier(@1, $1);
                        NamedType *nt = new NamedType(ident);
                        $$ = new List<NamedType*>;
                        $$ -> Append(nt);
                      }
                  |   Ident_plus_comma ',' T_Identifier
                      {
                        Identifier *ident = new Identifier(@3, $3);
                        NamedType *nt = new NamedType(ident);
                        $$ -> Append(nt);
                      }
                  ;                                 



Field             :   VariableDecl 
                      {
                        $$ = $1;
                      }
                  |   FunctionDecl
                      {
                        $$ = $1;
                      }
                  ;



InterfaceDecl     :   T_Interface T_Identifier '{' Prototype_star '}'
                      {
                        Identifier *ident = new Identifier(@2, $2);
                        $$ = new InterfaceDecl(ident, $4);
                      }
                  ;



Prototype_star    :   /* epsilon */ 
                      {
                        $$ = new List<Decl*>;
                      }
                  |   Prototype_star Prototype 
                      {
                        $$ -> Append($2);
                      }
                  ;



Prototype         :   Type T_Identifier '(' Formals ')' ';' 
                      {
                        Identifier *ident = new Identifier(@2, $2);
                        $$ = new FnDecl(ident, $1, $4);
                      }
                  |   T_Void T_Identifier '(' Formals ')' ';'
                      {
                        Identifier *ident = new Identifier(@2, $2);
                        $$ = new FnDecl(ident, Type::voidType, $4);
                      }
                  ;



Var_Decl_plus     :   VariableDecl 
                      {
                        $$ = new List<VarDecl*>;
                        $$ -> Append($1);
                      }
                  |   Var_Decl_plus VariableDecl 
                      {
                        $$ -> Append($2);
                      }
                  ;



Stmt_plus         :   Stmt 
                      {
                        $$ = new List<Stmt*>;
                        $$ -> Append($1);
                      }
                  |   Stmt_plus Stmt 
                      {
                        $$ -> Append($2); 
                      }
                  ;



StmtBlock         :   '{' Var_Decl_plus Stmt_plus '}'
                      {
                        $$ = new StmtBlock($2, $3);
                      }
                  |   '{' Stmt_plus '}'
                      {
                        $$ = new StmtBlock(new List<VarDecl*>, $2);
                      }
                  |   '{' Var_Decl_plus '}'
                      {
                        $$ = new StmtBlock($2, new List<Stmt*>);
                      }
                  |   '{' '}'
                      {
                        $$ = new StmtBlock(new List<VarDecl*>, new List<Stmt*>);
                      }
                  ;



Stmt              :   Optional_Expr ';'
                      {
                          $$ = $1;
                      }
                  |   IfStmt
                      {
                          $$ = $1;
                      }
                  |   WhileStmt
                      {
                          $$ = $1;
                      }
                  |   ForStmt
                      {
                          $$ = $1;
                      }
                  |   BreakStmt
                      {
                          $$ = $1;
                      }
                  |   ReturnStmt
                      {
                          $$ = $1;
                      }
                  |   PrintStmt
                      {
                          $$ = $1;
                      }
                  |   StmtBlock
                      {
                          $$ = $1;
                      }
                  ;



IfStmt            :   T_If '(' Expr ')' Stmt %prec IF_NO_ELSE
                      {
                        $$ = new IfStmt($3, $5, NULL);
                      }
                  |   T_If '(' Expr ')' Stmt T_Else Stmt
                      {
                        $$ = new IfStmt($3, $5, $7);
                      }
                  ;



WhileStmt         :   T_While '(' Expr ')' Stmt
                      {
                        $$ = new WhileStmt($3, $5);
                      }
                  ;



ForStmt           :   T_For '(' Optional_Expr ';' Expr ';' Optional_Expr ')' Stmt
                      {
                        $$ = new ForStmt($3, $5, $7, $9);
                      }
                  ;



Optional_Expr     :   /* epsilon */ 
                      {
                        $$ = new EmptyExpr();
                      }
                  |   Expr 
                      {
                        $$ = $1;
                      }
                  ;



ReturnStmt        :   T_Return Optional_Expr ';'
                      {
                        $$ = new ReturnStmt(@1, $2);
                      }
                  ;



BreakStmt         :   T_Break ';'
                      {
                        $$ = new BreakStmt(@1);
                      }
                  ;



PrintStmt         :   T_Print '(' Expr_plus_comma ')' ';'
                      {
                        $$ = new PrintStmt($3);
                      }
                  ;



Expr_plus_comma   :   Expr 
                      {
                        $$ = new List<Expr*>;
                        $$ -> Append($1);
                      }
                  |   Expr_plus_comma ',' Expr 
                      {
                        $$ -> Append($3);
                      }
                  ;



Expr              :   LValue '=' Expr 
                      {
                        Operator *op = new Operator(@2, "=");
                        $$ = new AssignExpr($1, op, $3);
                      }
                  |   Constant
                      {
                        $$ = $1;
                      }
                  |   LValue
                      {
                        $$ = $1;
                      }
                  |   T_This 
                      {
                        $$ = new This(@1);
                      }
                  |   Call 
                      {
                        $$ = $1;
                      }
                  |   '(' Expr ')'
                      {
                        $$ = $2;
                      }
                  |   Expr '+' Expr
                      {
                        Operator *op = new Operator(@2, "+");
                        $$ = new ArithmeticExpr($1, op, $3);
                      }
                  |   Expr '-' Expr
                      {
                        Operator *op = new Operator(@2, "-");
                        $$ = new ArithmeticExpr($1, op, $3);
                      }
                  |   Expr '*' Expr
                      {
                        Operator *op = new Operator(@2, "*");
                        $$ = new ArithmeticExpr($1, op, $3);
                      }
                  |   Expr '/' Expr
                      {
                        Operator *op = new Operator(@2, "/");
                        $$ = new ArithmeticExpr($1, op, $3);
                      }
                  |   Expr '%' Expr
                      {
                        Operator *op = new Operator(@2, "%");
                        $$ = new ArithmeticExpr($1, op, $3);
                      }
                  |   '-' Expr %prec UMINUS
                      {
                        Operator *op = new Operator(@1, "-");
                        $$ = new ArithmeticExpr(op, $2);
                      }
                  |   Expr '<' Expr
                      {
                        Operator *op = new Operator(@2, "<");
                        $$ = new RelationalExpr($1, op, $3);
                      }
                  |   Expr T_LessEqual Expr
                      {
                        Operator *op = new Operator(@2, "<=");
                        $$ = new RelationalExpr($1, op, $3);
                      }
                  |   Expr '>' Expr
                      {
                        Operator *op = new Operator(@2, ">");
                        $$ = new RelationalExpr($1, op, $3);
                      }
                  |   Expr T_GreaterEqual Expr
                      {
                        Operator *op = new Operator(@2, ">=");
                        $$ = new RelationalExpr($1, op, $3);
                      }
                  |   Expr T_Equal Expr
                      {
                        Operator *op = new Operator(@2, "==");
                        $$ = new EqualityExpr($1, op, $3);
                      }
                  |   Expr T_NotEqual Expr
                      {
                        Operator *op = new Operator(@2, "!=");
                        $$ = new EqualityExpr($1, op, $3);
                      }
                  |   Expr T_And Expr
                      {
                        Operator *op = new Operator(@2, "&&");
                        $$ = new LogicalExpr($1, op, $3);
                      }
                  |   Expr T_Or Expr
                      {
                        Operator *op = new Operator(@2, "||");
                        $$ = new LogicalExpr($1, op, $3);
                      }
                  |   '!' Expr
                      {
                        Operator *op = new Operator(@1, "!");
                        $$ = new LogicalExpr(op, $2);
                      }
                  |   T_ReadInteger '(' ')'
                      {
                        $$ = new ReadIntegerExpr(@1);
                      }
                  |   T_ReadLine '(' ')'
                      {
                        $$ = new ReadLineExpr(@1);
                      }
                  |   T_New '(' T_Identifier ')'
                      {
                        Identifier *ident = new Identifier(@3, $3);
                        NamedType *nt = new NamedType(ident);
                        $$ = new NewExpr(@1, nt);
                      }
                  |   T_NewArray '(' Expr ',' Type ')'
                      {
                        $$ = new NewArrayExpr(@1, $3, $5);
                      }
                  ;



LValue            :   T_Identifier
                      {
                        Identifier *ident = new Identifier(@1, $1);
                        $$ = new FieldAccess(NULL, ident);
                      }
                  |   Expr '.' T_Identifier 
                      {
                        Identifier *ident = new Identifier(@3, $3);
                        $$ = new FieldAccess($1, ident);
                      }
                  |   Expr '[' Expr ']'
                      {
                        $$ = new ArrayAccess(@1, $1, $3);
                      }
                  ;



Call              :   T_Identifier '(' Actuals ')'
                      {
                        Identifier *ident = new Identifier(@1, $1);
                        $$ = new Call(@1, NULL, ident, $3);  
                      }
                  |   Expr '.' T_Identifier '(' Actuals ')'
                      {
                        Identifier *ident = new Identifier(@3, $3);
                        $$ = new Call(@1, $1, ident, $5);
                      }
                  ;



Actuals           :   Expr_plus_comma
                      {
                        $$ = $1;
                      }
                  |   /* epsilon */
                      {
                        $$ = new List<Expr*>;
                      }
                  ;



Constant          :   T_IntConstant
                      {
                        $$ = new IntConstant(@1, $1);
                      }
                  |   T_DoubleConstant
                      {
                        $$ = new DoubleConstant(@1, $1);
                      }
                  |   T_BoolConstant
                      {
                        $$ = new BoolConstant(@1, $1);
                      }
                  |   T_StringConstant
                      {
                        $$ = new StringConstant(@1, $1);
                      }
                  |   T_Null
                      {
                        $$ = new NullConstant(@1);
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
