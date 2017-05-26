

#ifndef _H_ast
#define _H_ast

#include <iostream>
#include <stdlib.h>   
#include "location.h"
#include "scope.h"
#include "errors.h"
#include "codegen.h"


extern CodeGenerator *CG;

class Node
{
  protected:
    yyltype *location;
    Node *parent;
    Type *expr_type; 
    Location *emit_loc;

  public:
    
    Node(yyltype loc);
    Node();
    
    yyltype *GetLocation()   { return location; }
    void SetParent(Node *p)  { parent = p; }
    Node *GetParent()        { return parent; }
    
    virtual const char *GetPrintNameForNode() = 0;
    
    
    void Print(int indentLevel, const char *label = NULL);
    virtual void PrintChildren(int indentLevel) {}
    
    virtual void Check(checkT c) {}
    virtual Type * GetType() { return expr_type; }
    virtual bool IsLoopStmt() { return false; }
    virtual bool IsSwitchStmt() { return false; }
    virtual bool IsCaseStmt() { return false; }
    
    virtual void Emit() {}
    virtual Location * GetEmitLoc() { return emit_loc; }
};

class Identifier : public Node
{
  protected:
    char *name;
    Decl *decl;

  public:
    
    Identifier(yyltype loc, const char *name);
    
    const char *GetPrintNameForNode()   { return "Identifier"; }
    void PrintChildren(int indentLevel);
    friend std::ostream& operator<<(std::ostream& out, Identifier *id)
        { return out << id->name; }
    
    void Check(checkT c);
    bool IsEquivalentTo(Identifier *other);
    char *GetIdName() { return name; }
    void SetDecl(Decl *d) { decl = d; }
    Decl * GetDecl() { return decl; }
    
    void Emit();
    void AddPrefix(const char *prefix);
    Location * GetEmitLocDeref() { return GetEmitLoc(); }

  protected:
    void CheckDecl();
};






class Error : public Node
{
  public:
    
    Error() : Node() {}
    
    const char *GetPrintNameForNode()   { return "Error"; }
};

#endif

