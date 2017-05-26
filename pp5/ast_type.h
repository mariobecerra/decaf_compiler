

#ifndef _H_ast_type
#define _H_ast_type

#include <iostream>
#include "ast.h"
#include "list.h"

class Type : public Node
{
  protected:
    char *typeName;

  public :
    
    static Type *intType, *doubleType, *boolType, *voidType,
                *nullType, *stringType, *errorType;
    
    Type(yyltype loc) : Node(loc) { expr_type = NULL; }
    Type(const char *str);
    
    const char *GetPrintNameForNode() { return "Type"; }
    void PrintChildren(int indentLevel);
    virtual void PrintToStream(std::ostream& out) { out << typeName; }
    friend std::ostream& operator<<(std::ostream& out, Type *t)
        { t->PrintToStream(out); return out; }
    
    void Check(checkT c);
    virtual void Check(checkT c, reasonT r) { Check(c); }
    virtual bool IsBasicType() { return !IsNamedType() && !IsArrayType(); }
    virtual bool IsNamedType() { return false; }
    virtual bool IsArrayType() { return false; }
    virtual bool IsEquivalentTo(Type *other) { return this == other; }
    virtual bool IsCompatibleWith(Type *other) { return this == other; }
    char * GetTypeName() { return typeName; }
    virtual void SetSelfType() { expr_type = this; }
    
    virtual int GetTypeSize() { return 4; }
};

class NamedType : public Type
{
  protected:
    Identifier *id;

  public:
    
    NamedType(Identifier *i);
    
    const char *GetPrintNameForNode() { return "NamedType"; }
    void PrintChildren(int indentLevel);
    void PrintToStream(std::ostream& out) { out << id; }
    
    void Check(checkT c, reasonT r);
    void Check(checkT c) { Check(c, LookingForType); }
    bool IsNamedType() { return true; }
    bool IsEquivalentTo(Type *other);
    bool IsCompatibleWith(Type *other);
    Identifier *GetId() { return id; }

  protected:
    void CheckDecl(reasonT r);
};

class ArrayType : public Type
{
  protected:
    Type *elemType;

  public:
    
    ArrayType(yyltype loc, Type *elemType);
    
    const char *GetPrintNameForNode() { return "ArrayType"; }
    void PrintChildren(int indentLevel);
    void PrintToStream(std::ostream& out) { out << elemType << "[]"; }
    
    void Check(checkT c);
    bool IsArrayType() { return true; }
    bool IsEquivalentTo(Type *other);
    bool IsCompatibleWith(Type *other);
    Type * GetElemType() { return elemType->GetType(); }

  protected:
    void CheckDecl();
};

#endif

