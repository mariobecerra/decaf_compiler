

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "list.h"
#include "ast_type.h"

class Type;
class NamedType;
class Identifier;
class Stmt;

class Decl : public Node
{
  protected:
    Identifier *id;
    int idx;

  public:
    
    Decl(Identifier *name);
    
    friend std::ostream& operator<<(std::ostream& out, Decl *d)
        { return out << d->id; }
    
    Identifier * GetId() { return id; }
    int GetIndex() { return idx; }
    virtual bool IsVarDecl() { return false; }
    virtual bool IsClassDecl() { return false; }
    virtual bool IsInterfaceDecl() { return false; }
    virtual bool IsFnDecl() { return false; }
    
    virtual void AssignOffset() {}
    virtual void AssignMemberOffset(bool inClass, int offset) {}
    virtual void AddPrefixToMethods() {}
};

class VarDecl : public Decl
{
  protected:
    Type *type;
    bool is_global;
    int class_member_ofst;

  public:
    
    VarDecl(Identifier *name, Type *type);
    
    const char *GetPrintNameForNode() { return "VarDecl"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    bool IsVarDecl() { return true; }
    
    void AssignOffset();
    void AssignMemberOffset(bool inClass, int offset);
    void Emit();
    void SetEmitLoc(Location *l) { emit_loc = l; }

  protected:
    void BuildST();
    void CheckDecl();
    bool IsGlobal() { return this->GetParent()->GetParent() == NULL; }
    bool IsClassMember() {
        Decl *d = dynamic_cast<Decl*>(this->GetParent());
        return d ? d->IsClassDecl() : false;
    }
};

class FnDecl;

class ClassDecl : public Decl
{
  protected:
    List<Decl*> *members;
    NamedType *extends;
    List<NamedType*> *implements;
    int instance_size;
    int vtable_size;
    List<VarDecl*> *var_members;
    List<FnDecl*> *methods;

  public:
    
    ClassDecl(Identifier *name, NamedType *extends,
              List<NamedType*> *implements, List<Decl*> *members);
    
    const char *GetPrintNameForNode() { return "ClassDecl"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    bool IsClassDecl() { return true; }
    bool IsChildOf(Decl *other);
    NamedType * GetExtends() { return extends; }
    
    void AssignOffset();
    void Emit();
    int GetInstanceSize() { return instance_size; }
    int GetVTableSize() { return vtable_size; }
    void AddMembersToList(List<VarDecl*> *vars, List<FnDecl*> *fns);
    void AddPrefixToMethods();

  protected:
    void BuildST();
    void CheckDecl();
    void CheckInherit();
};

class InterfaceDecl : public Decl
{
  protected:
    List<Decl*> *members;

  public:
    
    InterfaceDecl(Identifier *name, List<Decl*> *members);
    
    const char *GetPrintNameForNode() { return "InterfaceDecl"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    bool IsInterfaceDecl() { return true; }
    List<Decl*> * GetMembers() { return members; }
    
    void Emit();

  protected:
    void BuildST();
    void CheckDecl();
};

class FnDecl : public Decl
{
  protected:
    List<VarDecl*> *formals;
    Type *returnType;
    Stmt *body;
    int vtable_ofst;

  public:
    
    FnDecl(Identifier *name, Type *returnType, List<VarDecl*> *formals);
    void SetFunctionBody(Stmt *b);
    
    const char *GetPrintNameForNode() { return "FnDecl"; }
    void PrintChildren(int indentLevel);
    
    void Check(checkT c);
    bool IsFnDecl() { return true; }
    bool IsEquivalentTo(Decl *fn);
    List<VarDecl*> * GetFormals() { return formals; }
    
    void AddPrefixToMethods();
    void AssignMemberOffset(bool inClass, int offset);
    void Emit();
    int GetVTableOffset() { return vtable_ofst; }
    bool HasReturnValue() { return returnType != Type::voidType; }
    bool IsClassMember() {
        Decl *d = dynamic_cast<Decl*>(this->GetParent());
        return d ? d->IsClassDecl() : false;
    }

  protected:
    void BuildST();
    void CheckDecl();
};

#endif

