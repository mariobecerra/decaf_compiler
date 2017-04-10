/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>

ClassDecl* Expr::GetClassDecl(Scope *s) {
    while (s != NULL) {
        ClassDecl *d;
        if ((d = s->GetClassDecl()) != NULL)
            return d;
        s = s->GetParent();
    }

    return NULL;
}

Decl* Expr::GetFieldDecl(Identifier *f, Type *b) {
    NamedType *t = dynamic_cast<NamedType*>(b);

    while (t != NULL) {
        Decl *d = Program::gScope->table->Lookup(t->Name());
        ClassDecl *c = dynamic_cast<ClassDecl*>(d);
        InterfaceDecl *i = dynamic_cast<InterfaceDecl*>(d);

        Decl *fieldDecl;
        if (c != NULL) {
            if ((fieldDecl = GetFieldDecl(f, c->GetScope())) != NULL)
                return fieldDecl;
            else
                t = c->GetExtends();
        } else if (i != NULL) {
            if ((fieldDecl = GetFieldDecl(f, i->GetScope())) != NULL)
                return fieldDecl;
            else
                t = NULL;
        } else {
            t = NULL;
        }
    }

    return GetFieldDecl(f, scope);
}

Decl* Expr::GetFieldDecl(Identifier *f, Scope *s) {
    while (s != NULL) {
        Decl *lookup;
        if ((lookup = s->table->Lookup(f->Name())) != NULL)
            return lookup;

        s = s->GetParent();
    }

    return NULL;
}

Type* EmptyExpr::GetType() {
    return Type::errorType;
}

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}

Type* IntConstant::GetType() {
    return Type::intType;
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}

Type* DoubleConstant::GetType() {
    return Type::doubleType;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}

Type* BoolConstant::GetType() {
    return Type::boolType;
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
}

Type* StringConstant::GetType() {
    return Type::stringType;
}

Type* NullConstant::GetType() {
    return Type::nullType;
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}
CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r)
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL;
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}

void CompoundExpr::BuildScope(Scope *parent) {
    scope->SetParent(parent);

    if (left != NULL)
        left->BuildScope(scope);
    if (right != NULL)
        right->BuildScope(scope);
}

void CompoundExpr::Check() {
    if (left != NULL)
        left->Check();

    if (right != NULL)
        right->Check();
}

Type* ArithmeticExpr::GetType() {
    Type *rtype = right->GetType();

    if (left == NULL) {
        if (rtype->IsEquivalentTo(Type::intType) ||
            rtype->IsEquivalentTo(Type::doubleType))
            return rtype;
        else
            return Type::errorType;
    }

    Type *ltype = left->GetType();

    if (ltype->IsEquivalentTo(Type::intType) &&
        rtype->IsEquivalentTo(Type::intType))
        return ltype;

    if (ltype->IsEquivalentTo(Type::doubleType) &&
        rtype->IsEquivalentTo(Type::doubleType))
        return ltype;

    return Type::errorType;
}

void ArithmeticExpr::Check() {
    if (left != NULL)
        left->Check();

    right->Check();

    Type *rtype = right->GetType();

    if (left == NULL) {
        return;
    }

    Type *ltype = left->GetType();

    if (ltype->IsEquivalentTo(Type::intType) &&
        rtype->IsEquivalentTo(Type::intType))
        return;

    
    if (ltype->IsEquivalentTo(Type::doubleType) &&
        rtype->IsEquivalentTo(Type::doubleType))
        return;
}

Type* RelationalExpr::GetType() {
    Type *rtype = right->GetType();
    Type *ltype = left->GetType();

    if (ltype->IsEquivalentTo(Type::intType) &&
        rtype->IsEquivalentTo(Type::intType))
        return Type::errorType;

    if (ltype->IsEquivalentTo(Type::doubleType) &&
        rtype->IsEquivalentTo(Type::doubleType))
        return Type::errorType;

    return Type::boolType;
}

void RelationalExpr::Check() {
    left->Check();
    right->Check();
    Type *rtype = right->GetType();
    Type *ltype = left->GetType();

    if (ltype->IsEquivalentTo(Type::intType) &&
        rtype->IsEquivalentTo(Type::intType))
        return;

    if (ltype->IsEquivalentTo(Type::doubleType) &&
        rtype->IsEquivalentTo(Type::doubleType))
        return;
}

Type* EqualityExpr::GetType() {
    Type *rtype = right->GetType();
    Type *ltype = left->GetType();

    if (!rtype->IsEquivalentTo(ltype) &&
        !ltype->IsEquivalentTo(rtype))
        return Type::errorType;

   return Type::boolType;
}

void EqualityExpr::Check() {
    left->Check();
    right->Check();
}

Type* LogicalExpr::GetType() {
    Type *rtype = right->GetType();

    if (left == NULL) {
        if (rtype->IsEquivalentTo(Type::boolType))
            return Type::boolType;
        else
            return Type::errorType;
    }

    Type *ltype = left->GetType();

    if (ltype->IsEquivalentTo(Type::boolType) &&
        rtype->IsEquivalentTo(Type::boolType))
        return Type::boolType;

    return Type::errorType;
}

void LogicalExpr::Check() {
    if (left != NULL)
        left->Check();

    right->Check();

    Type *rtype = right->GetType();

    
    if (left == NULL) {
        return;
    }

    Type *ltype = left->GetType();

    if (ltype->IsEquivalentTo(Type::boolType) &&
        rtype->IsEquivalentTo(Type::boolType))
        return;
}

Type* AssignExpr::GetType() {
    Type *ltype = left->GetType();
    Type *rtype = right->GetType();

    if (!rtype->IsEquivalentTo(ltype))
        return Type::errorType;

    return ltype;
}

void AssignExpr::Check() {
    left->Check();
    right->Check();

    Type *ltype = left->GetType();
    Type *rtype = right->GetType();
}

Type* This::GetType() {
    ClassDecl *d = GetClassDecl(scope);
    if (d == NULL)
        return Type::errorType;

    return d->GetType();
}

void This::Check() {
}
  
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

Type* ArrayAccess::GetType() {
    ArrayType *t = dynamic_cast<ArrayType*>(base->GetType());
    if (t == NULL)
        return Type::errorType;

    return t->GetElemType();
}

void ArrayAccess::BuildScope(Scope *parent) {
    scope->SetParent(parent);

    base->BuildScope(scope);
    subscript->BuildScope(scope);
}

void ArrayAccess::Check() {
    base->Check();
    subscript->Check();
    
    if (base->GetType() == Type::errorType) // The base is an undeclared variable, so we don't need to further check the fields.
        return;
    
    ArrayType *t = dynamic_cast<ArrayType*>(base->GetType());
}
     
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); 
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}

Type* FieldAccess::GetType() {
    Decl *d;
    ClassDecl *c;
    Type *t;

    c = GetClassDecl(scope);

    if (base == NULL) {
        if (c == NULL) {
            d = GetFieldDecl(field, scope);
        } else {
            t = c->GetType();
            d = GetFieldDecl(field, t);
        }
    } else {
        t = base->GetType();
        d = GetFieldDecl(field, t);
    }

    if (d == NULL)
        return Type::errorType;

    if (dynamic_cast<VarDecl*>(d) == NULL)
        return Type::errorType;

    return static_cast<VarDecl*>(d)->GetType();
}

void FieldAccess::BuildScope(Scope *parent) {
    scope->SetParent(parent);

    if (base != NULL)
        base->BuildScope(scope);
}

void FieldAccess::Check() {
    if (base != NULL) {
        base->Check();

        if (base->GetType() == Type::errorType) 
            return;
    }
    Decl *d;
    Type *t;

    if (base == NULL) {
        ClassDecl *c = GetClassDecl(scope);
        if (c == NULL) {
            if ((d = GetFieldDecl(field, scope)) == NULL) {
                return;
            }
        } else {
            t = c->GetType();
            if ((d = GetFieldDecl(field, t)) == NULL) {
                return;
            }
        }
    } else {
        t = base->GetType();
        if ((d = GetFieldDecl(field, t)) == NULL) {
            return;
        }
        else if (GetClassDecl(scope) == NULL) {
            return;
        }
    }
}

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); 
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}

Type* Call::GetType() {
    Decl *d;

    if (base == NULL) {
        ClassDecl *c = GetClassDecl(scope);
        if (c == NULL) {
            if ((d = GetFieldDecl(field, scope)) == NULL)
                return Type::errorType;
        } else {
            if ((d = GetFieldDecl(field, c->GetType())) == NULL)
                return Type::errorType;
        }
    } else {
        Type *t = base->GetType();
        if ((d = GetFieldDecl(field, t)) == NULL) {

            if (dynamic_cast<ArrayType*>(t) != NULL &&
                strcmp("length", field->Name()) == 0)
                return Type::intType;

            return Type::errorType;
        }
    }

    if (dynamic_cast<FnDecl*>(d) == NULL)
        return Type::errorType;

    return static_cast<FnDecl*>(d)->GetReturnType();
}

void Call::BuildScope(Scope *parent) {
    scope->SetParent(parent);

    if (base != NULL)
        base->BuildScope(scope);

    for (int i = 0, n = actuals->NumElements(); i < n; ++i)
        actuals->Nth(i)->BuildScope(scope);
}

void Call::Check() {
    if (base != NULL)
        base->Check();

    Decl *d;
    Type *t;

    if (base == NULL) {
        ClassDecl *c = GetClassDecl(scope);
        if (c == NULL) {
            if ((d = GetFieldDecl(field, scope)) == NULL) {
                CheckActuals(d);
                return;
            }
        } else {
            t = c->GetType();
            if ((d = GetFieldDecl(field, t)) == NULL) {
                CheckActuals(d);
                return;
            }
        }
    } else {
        t = base->GetType();
        if (! t->typeDeclared) {return;} 
        if ((d = GetFieldDecl(field, t)) == NULL) {  
            CheckActuals(d);

            if (dynamic_cast<ArrayType*>(t) == NULL ||
                strcmp("length", field->Name()) != 0)
            return;
        }
    }

    CheckActuals(d);
}

void Call::CheckActuals(Decl *d) {
    for (int i = 0, n = actuals->NumElements(); i < n; ++i)
        actuals->Nth(i)->Check();

    FnDecl *fnDecl = dynamic_cast<FnDecl*>(d);
    if (fnDecl == NULL)
        return;

    List<VarDecl*> *formals = fnDecl->GetFormals();

    int numExpected = formals->NumElements();
    int numGiven = actuals->NumElements();
    if (numExpected != numGiven) {
        return;
    }

    for (int i = 0, n = actuals->NumElements(); i < n; ++i) {
        Type *given = actuals->Nth(i)->GetType();
        Type *expected = formals->Nth(i)->GetType();
    }
}

NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) { 
  Assert(c != NULL);
  (cType=c)->SetParent(this);
}

Type* NewExpr::GetType() {
    Decl *d = Program::gScope->table->Lookup(cType->Name());
    ClassDecl *c = dynamic_cast<ClassDecl*>(d);

    if (c == NULL)
        return Type::errorType;

    return c->GetType();
}

void NewExpr::Check() {
    Decl *d = Program::gScope->table->Lookup(cType->Name());
    ClassDecl *c = dynamic_cast<ClassDecl*>(d);
}

NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this); 
    (elemType=et)->SetParent(this);
}

Type* NewArrayExpr::GetType() {
    return new ArrayType(elemType);
}

void NewArrayExpr::BuildScope(Scope *parent) {
    scope->SetParent(parent);

    size->BuildScope(scope);
}

void NewArrayExpr::Check() {
    size->Check();

    if (elemType->IsPrimitive() && !elemType->IsEquivalentTo(Type::voidType))
        return;

    Decl *d = Program::gScope->table->Lookup(elemType->Name());
    if (dynamic_cast<ClassDecl*>(d) == NULL)
        elemType->ReportNotDeclaredIdentifier(LookingForType);
}

Type* ReadIntegerExpr::GetType() {
    return Type::intType;
}

Type* ReadLineExpr::GetType() {
    return Type::stringType;
}

       
