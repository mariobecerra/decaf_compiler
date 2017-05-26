

#ifndef _H_ast_stmt
#define _H_ast_stmt

#include "ast.h"
#include "list.h"

class Decl;
class VarDecl;
class Expr;

class Program : public Node
{
  protected:
    List<Decl*> *decls;

  public:
    
    Program(List<Decl*> *declList);
    
    const char *GetPrintNameForNode() { return "Program"; }
    void PrintChildren(int indentLevel);
    
    void Check();
    void Check(checkT c) { Check(); }
    
    void Emit();
};

class Stmt : public Node
{
  public:
    
    Stmt() : Node() {}
    Stmt(yyltype loc) : Node(loc) {}
};

class StmtBlock : public Stmt
{
  protected:
    List<VarDecl*> *decls;
    List<Stmt*> *stmts;

  public:
    
    StmtBlock(List<VarDecl*> *variableDeclarations, List<Stmt*> *statements);
    
    const char *GetPrintNameForNode() { return "StmtBlock"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    
    void Emit();

  protected:
    void BuildST();
};


class ConditionalStmt : public Stmt
{
  protected:
    Expr *test;
    Stmt *body;

  public:
    
    ConditionalStmt(Expr *testExpr, Stmt *body);
};

class LoopStmt : public ConditionalStmt
{
  protected:
    const char *end_loop_label;

  public:
    
    LoopStmt(Expr *testExpr, Stmt *body)
            : ConditionalStmt(testExpr, body) { end_loop_label = NULL; }
    
    bool IsLoopStmt() { return true; }
    
    virtual const char * GetEndLoopLabel() { return end_loop_label; }
};

class ForStmt : public LoopStmt
{
  protected:
    Expr *init, *step;

  public:
    
    ForStmt(Expr *init, Expr *test, Expr *step, Stmt *body);
    
    const char *GetPrintNameForNode() { return "ForStmt"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    
    void Emit();

  protected:
    void BuildST();
    void CheckType();
};

class WhileStmt : public LoopStmt
{
  public:
    
    WhileStmt(Expr *test, Stmt *body) : LoopStmt(test, body) {}
    
    const char *GetPrintNameForNode() { return "WhileStmt"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    
    void Emit();

  protected:
    void BuildST();
    void CheckType();
};

class IfStmt : public ConditionalStmt
{
  protected:
    Stmt *elseBody;

  public:
    
    IfStmt(Expr *test, Stmt *thenBody, Stmt *elseBody);
    
    const char *GetPrintNameForNode() { return "IfStmt"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    
    void Emit();

  protected:
    void BuildST();
    void CheckType();
};

class BreakStmt : public Stmt
{
  public:
    
    BreakStmt(yyltype loc) : Stmt(loc) {}
    
    const char *GetPrintNameForNode() { return "BreakStmt"; }
    
    void Check(checkT c);
    
    void Emit();
};

class IntConstant;

class CaseStmt : public Stmt
{
  protected:
    IntConstant *value;
    List<Stmt*> *stmts;
    const char *case_label;

  public:
    
    CaseStmt(IntConstant *v, List<Stmt*> *stmts);
    
    const char *GetPrintNameForNode() { return value ? "Case" : "Default"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    bool IsCaseStmt() { return value ? true : false; }
    
    void Emit();
    void GenCaseLabel();
    const char * GetCaseLabel() { return case_label; }
    IntConstant * GetCaseValue() { return value; }

  protected:
    void BuildST();
};

class SwitchStmt : public Stmt
{
  protected:
    Expr *expr;
    List<CaseStmt*> *cases;
    const char *end_switch_label;

  public:
    
    SwitchStmt(Expr *expr, List<CaseStmt*> *cases);
    
    const char *GetPrintNameForNode() { return "SwitchStmt"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    
    void Emit();
    bool IsSwitchStmt() { return true; }
    const char * GetEndSwitchLabel() { return end_switch_label; }

  protected:
    void BuildST();
};

class ReturnStmt : public Stmt
{
  protected:
    Expr *expr;

  public:
    
    ReturnStmt(yyltype loc, Expr *expr);
    
    const char *GetPrintNameForNode() { return "ReturnStmt"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    
    void Emit();
};

class PrintStmt : public Stmt
{
  protected:
    List<Expr*> *args;

  public:
    
    PrintStmt(List<Expr*> *arguments);
    
    const char *GetPrintNameForNode() { return "PrintStmt"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    
    void Emit();
};

#endif

