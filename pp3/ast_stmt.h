/* File: ast_stmt.h
 * ----------------
 * The Stmt class and its subclasses are used to represent
 * statements in the parse tree.  For each statment in the
 * language (for, if, return, etc.) there is a corresponding
 * node class for that construct. 
 *
 * pp3: You will need to extend the Stmt classes to implement
 * semantic analysis for rules pertaining to statements.
 */


#ifndef _H_ast_stmt
#define _H_ast_stmt

#include "list.h"
#include "ast.h"
#include "hashtable.h"
#include "ast_type.h"

class Decl;
class VarDecl;
class Expr;
class Type;
class ClassDecl;
class LoopStmt;
class FnDecl;
class SwitchStmt;

class Scope
{
  private:
    Scope *parent;

  public:
    Hashtable<Decl*> *table;
    ClassDecl *classDecl;
    LoopStmt *loopStmt;
    SwitchStmt *switchStmt;
    FnDecl *fnDecl;

  public:
    Scope() : table(new Hashtable<Decl*>), classDecl(NULL), loopStmt(NULL),
              fnDecl(NULL) {}

    void SetParent(Scope *p) { parent = p; }
    Scope* GetParent() { return parent; }

    void SetClassDecl(ClassDecl *d) { classDecl = d; }
    ClassDecl* Get_Class_Declaration() { return classDecl; }

    void SetLoopStmt(LoopStmt *s) { loopStmt = s; }
    LoopStmt* GetLoopStmt() { return loopStmt; }
    
    void SetSwitchStmt(SwitchStmt *s) { switchStmt = s; }
    SwitchStmt* GetSwitchStmt() { return switchStmt; }

    void SetFnDecl(FnDecl *d) { fnDecl = d; }
    FnDecl* GetFnDecl() { return fnDecl; }

    int Add_Declaration(Decl *decl);
    friend std::ostream& operator<<(std::ostream& out, Scope *s);
};
   
class Program : public Node
{
  protected:
     List<Decl*> *decls;
     
  public:
     static Scope *G_Scope;
     Program(List<Decl*> *declList);
     void Check();
  
  private:
     void ScopeMaker();
};

class Stmt : public Node
{
  protected:
     Scope *scope;
     
  public:
     Stmt() : Node(), scope(new Scope) {}
     Stmt(yyltype loc) : Node(loc), scope(new Scope) {}
     virtual void ScopeMaker(Scope *parent);
     virtual void Check() = 0;
};

class StmtBlock : public Stmt 
{
  protected:
    List<VarDecl*> *decls;
    List<Stmt*> *stmts;
    
  public:
    StmtBlock(List<VarDecl*> *variableDeclarations, List<Stmt*> *statements);
    void ScopeMaker(Scope *parent);
    void Check();
};

  
class ConditionalStmt : public Stmt
{
  protected:
    Expr *test;
    Stmt *body;
  
  public:
    ConditionalStmt(Expr *testExpr, Stmt *body);
    virtual void ScopeMaker(Scope *parent);
    virtual void Check();
};

class LoopStmt : public ConditionalStmt 
{
  public:
    LoopStmt(Expr *testExpr, Stmt *body)
            : ConditionalStmt(testExpr, body) {}
    virtual void ScopeMaker(Scope *parent);
};

class ForStmt : public LoopStmt 
{
  protected:
    Expr *init, *step;
  
  public:
    ForStmt(Expr *init, Expr *test, Expr *step, Stmt *body);
};

class WhileStmt : public LoopStmt 
{
  public:
    WhileStmt(Expr *test, Stmt *body) : LoopStmt(test, body) {}
};

class IfStmt : public ConditionalStmt 
{
  protected:
    Stmt *elseBody;
  
  public:
    IfStmt(Expr *test, Stmt *thenBody, Stmt *elseBody);
    void ScopeMaker(Scope *parent);
    void Check();
};

class BreakStmt : public Stmt 
{
  public:
    BreakStmt(yyltype loc) : Stmt(loc) {}
    void Check();
};

class ReturnStmt : public Stmt  
{
  protected:
    Expr *expr;
  
  public:
    ReturnStmt(yyltype loc, Expr *expr);
    void ScopeMaker(Scope *parent);
    void Check();
};

class PrintStmt : public Stmt
{
  protected:
    List<Expr*> *args;
    
  public:
    PrintStmt(List<Expr*> *arguments);
    void ScopeMaker(Scope *parent);
    void Check();
};

class SwitchStmt : public Stmt
{
  public:
    class CaseStmt : public Stmt
    {
      protected:
        Expr *intConst;
        List<Stmt*> *caseBody;

      public:
        CaseStmt(Expr *intConst, List<Stmt*> *caseBody);
        void ScopeMaker(Scope *parent);
        void Check();
    };

  protected:
    Expr *expr;
    List<CaseStmt*> *caseStmts;

  public:
    SwitchStmt(Expr *expr, List<CaseStmt*> *caseStmts);
    void ScopeMaker(Scope *parent);
    void Check();
};

#endif
