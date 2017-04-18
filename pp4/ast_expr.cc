/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>

ClassDecl* Expr::Get_Class_Declaration(Scope *s) {
    while (s != NULL) {
        ClassDecl *d;
        if ((d = s->Get_Class_Declaration()) != NULL)
            return d;
        s = s->GetParent();
    }

    return NULL;
}

Decl* Expr::GetFieldDecl(Identifier *f, Type *b) {
    NamedType *t = dynamic_cast<NamedType*>(b);

    while (t != NULL) {
        Decl *d = Program::G_Scope->table->Lookup(t->Name());
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

Type* EmptyExpr::TypeFinder() {
    return Type::errorType;
}

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}

Type* IntConstant::TypeFinder() {
    return Type::intType;
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}

Type* DoubleConstant::TypeFinder() {
    return Type::doubleType;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}

Type* BoolConstant::TypeFinder() {
    return Type::boolType;
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
}

Type* StringConstant::TypeFinder() {
    return Type::stringType;
}

Type* NullConstant::TypeFinder() {
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

void CompoundExpr::ScopeMake(Scope *parent) {
    scope->SetParent(parent);

    if (left != NULL)
        left->ScopeMake(scope);
    if (right != NULL)
        right->ScopeMake(scope);
}

void CompoundExpr::Check() {
    if (left != NULL)
        left->Check();

    if (right != NULL)
        right->Check();
}

Type* ArithmeticExpr::TypeFinder() {
    Type *rtype = right->TypeFinder();

    if (left == NULL) {
        if (rtype->AreEquiv(Type::intType) ||
            rtype->AreEquiv(Type::doubleType))
            return rtype;
        else
            return Type::errorType;
    }

    Type *ltype = left->TypeFinder();

    if (ltype->AreEquiv(Type::intType) &&
        rtype->AreEquiv(Type::intType))
        return ltype;

    if (ltype->AreEquiv(Type::doubleType) &&
        rtype->AreEquiv(Type::doubleType))
        return ltype;

    return Type::errorType;
}

void ArithmeticExpr::Check() {
    if (left != NULL)
        left->Check();

    right->Check();

    Type *rtype = right->TypeFinder();

    if (left == NULL) {
        
        if (rtype->AreEquiv(Type::intType) ||
            rtype->AreEquiv(Type::doubleType))
            return;
        else
            ReportError::IncompatibleOperand(op, rtype);

        return;
    }

    Type *ltype = left->TypeFinder();

    if (ltype->AreEquiv(Type::intType) &&
        rtype->AreEquiv(Type::intType))
        return;

    
    if (ltype->AreEquiv(Type::doubleType) &&
        rtype->AreEquiv(Type::doubleType))
        return;

    ReportError::IncompatibleOperands(op, ltype, rtype);
}

Type* RelationalExpr::TypeFinder() {
    Type *rtype = right->TypeFinder();
    Type *ltype = left->TypeFinder();

    if (ltype->AreEquiv(Type::intType) &&
        rtype->AreEquiv(Type::intType))
        return Type::errorType;

    if (ltype->AreEquiv(Type::doubleType) &&
        rtype->AreEquiv(Type::doubleType))
        return Type::errorType;

    return Type::boolType;
}

void RelationalExpr::Check() {
    left->Check();
    right->Check();
    Type *rtype = right->TypeFinder();
    Type *ltype = left->TypeFinder();

    if (ltype->AreEquiv(Type::intType) &&
        rtype->AreEquiv(Type::intType))
        return;

    if (ltype->AreEquiv(Type::doubleType) &&
        rtype->AreEquiv(Type::doubleType))
        return;

    ReportError::IncompatibleOperands(op, ltype, rtype);
}

Type* EqualityExpr::TypeFinder() {
    Type *rtype = right->TypeFinder();
    Type *ltype = left->TypeFinder();

    if (!rtype->AreEquiv(ltype) &&
        !ltype->AreEquiv(rtype))
        return Type::errorType;

   return Type::boolType;
}

void EqualityExpr::Check() {
    left->Check();
    right->Check();

    Type *rtype = right->TypeFinder();
    Type *ltype = left->TypeFinder();

    
    if (!rtype->AreEquiv(ltype) &&
        !ltype->AreEquiv(rtype))
        ReportError::IncompatibleOperands(op, ltype, rtype);
}

Type* LogicalExpr::TypeFinder() {
    Type *rtype = right->TypeFinder();

    if (left == NULL) {
        if (rtype->AreEquiv(Type::boolType))
            return Type::boolType;
        else
            return Type::errorType;
    }

    Type *ltype = left->TypeFinder();

    if (ltype->AreEquiv(Type::boolType) &&
        rtype->AreEquiv(Type::boolType))
        return Type::boolType;

    return Type::errorType;
}

void LogicalExpr::Check() {
    if (left != NULL)
        left->Check();

    right->Check();

    Type *rtype = right->TypeFinder();

    
    if (left == NULL) {
        if (rtype->AreEquiv(Type::boolType))
            return;
        else
            ReportError::IncompatibleOperand(op, rtype);

        return;
    }

    Type *ltype = left->TypeFinder();

    if (ltype->AreEquiv(Type::boolType) &&
        rtype->AreEquiv(Type::boolType))
        return;

    ReportError::IncompatibleOperands(op, ltype, rtype);
}

Type* AssignExpr::TypeFinder() {
    Type *ltype = left->TypeFinder();
    Type *rtype = right->TypeFinder();

    if (!rtype->AreEquiv(ltype))
        return Type::errorType;

    return ltype;
}

void AssignExpr::Check() {
    left->Check();
    right->Check();

    Type *ltype = left->TypeFinder();
    Type *rtype = right->TypeFinder();

    if (!rtype->AreEquiv(ltype) && !ltype->IsEqualTo(Type::errorType))
        ReportError::IncompatibleOperands(op, ltype, rtype);
}

Type* This::TypeFinder() {
    ClassDecl *d = Get_Class_Declaration(scope);
    if (d == NULL)
        return Type::errorType;

    return d->TypeFinder();
}

void This::Check() {
    if (Get_Class_Declaration(scope) == NULL)
        ReportError::ThisOutsideClassScope(this);
}
  
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

Type* ArrayAccess::TypeFinder() {
    ArrayType *t = dynamic_cast<ArrayType*>(base->TypeFinder());
    if (t == NULL)
        return Type::errorType;

    return t->GetElemType();
}

void ArrayAccess::ScopeMake(Scope *parent) {
    scope->SetParent(parent);

    base->ScopeMake(scope);
    subscript->ScopeMake(scope);
}

void ArrayAccess::Check() {
    base->Check();
    subscript->Check();
    
    if (base->TypeFinder() == Type::errorType) // The base is an undeclared variable, so we don't need to further check the fields.
        return;
    
    ArrayType *t = dynamic_cast<ArrayType*>(base->TypeFinder());
    
    if (t == NULL)
        ReportError::BracketsOnNonArray(base);

    
    if (subscript->TypeFinder() != Type::errorType && !subscript->TypeFinder()->IsEqualTo(Type::intType))
        ReportError::SubscriptNotInteger(subscript);
}
     
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}

Type* FieldAccess::TypeFinder() {
    Decl *d;
    ClassDecl *c;
    Type *t;

    c = Get_Class_Declaration(scope);

    if (base == NULL) {
        if (c == NULL) {
            d = GetFieldDecl(field, scope);
        } else {
            t = c->TypeFinder();
            d = GetFieldDecl(field, t);
        }
    } else {
        t = base->TypeFinder();
        d = GetFieldDecl(field, t);
    }

    if (d == NULL)
        return Type::errorType;

    if (dynamic_cast<VarDecl*>(d) == NULL)
        return Type::errorType;

    return static_cast<VarDecl*>(d)->TypeFinder();
}

void FieldAccess::ScopeMake(Scope *parent) {
    scope->SetParent(parent);

    if (base != NULL)
        base->ScopeMake(scope);
}

void FieldAccess::Check() {
    if (base != NULL) {
        base->Check();

        if (base->TypeFinder() == Type::errorType) 
            return;
    }
    Decl *d;
    Type *t;

    if (base == NULL) {
        ClassDecl *c = Get_Class_Declaration(scope);
        if (c == NULL) {
            if ((d = GetFieldDecl(field, scope)) == NULL) {
                
                ReportError::IdentifierNotDeclared(field, LookingForVariable);
                return;
            }
        } else {
            t = c->TypeFinder();
            if ((d = GetFieldDecl(field, t)) == NULL) {
                
                ReportError::FieldNotFoundInBase(field, t);
                return;
            }
        }
    } else {
        t = base->TypeFinder();
        if ((d = GetFieldDecl(field, t)) == NULL) {
            
            ReportError::FieldNotFoundInBase(field, t);
            return;
        }
        else if (Get_Class_Declaration(scope) == NULL) {
            
            ReportError::InaccessibleField(field, t);
            return;
        }
    }

    
    if (dynamic_cast<VarDecl*>(d) == NULL)
        ReportError::IdentifierNotDeclared(field, LookingForVariable);
}

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}

Type* Call::TypeFinder() {
    Decl *d;

    if (base == NULL) {
        ClassDecl *c = Get_Class_Declaration(scope);
        if (c == NULL) {
            if ((d = GetFieldDecl(field, scope)) == NULL)
                return Type::errorType;
        } else {
            if ((d = GetFieldDecl(field, c->TypeFinder())) == NULL)
                return Type::errorType;
        }
    } else {
        Type *t = base->TypeFinder();
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

void Call::ScopeMake(Scope *parent) {
    scope->SetParent(parent);

    if (base != NULL)
        base->ScopeMake(scope);

    for (int i = 0, n = actuals->NumElements(); i < n; ++i)
        actuals->Nth(i)->ScopeMake(scope);
}

void Call::Check() {
    if (base != NULL)
        base->Check();

    Decl *d;
    Type *t;

    if (base == NULL) {
        ClassDecl *c = Get_Class_Declaration(scope);
        if (c == NULL) {
            if ((d = GetFieldDecl(field, scope)) == NULL) {
                ActualsFinder(d);
                
                ReportError::IdentifierNotDeclared(field, LookingForFunction);
                return;
            }
        } else {
            t = c->TypeFinder();
            if ((d = GetFieldDecl(field, t)) == NULL) {
                ActualsFinder(d);
                
                ReportError::IdentifierNotDeclared(field, LookingForFunction);
                return;
            }
        }
    } else {
        t = base->TypeFinder();
        if (! t->TD) {return;} // No need to check the fields of undeclared type.
        if ((d = GetFieldDecl(field, t)) == NULL) {  //&& f->TypeFinder() == lookup->TypeFinder()
            ActualsFinder(d);

            if (dynamic_cast<ArrayType*>(t) == NULL ||
                strcmp("length", field->Name()) != 0)
                    
                    ReportError::FieldNotFoundInBase(field, t);

            return;
        }
    }

    ActualsFinder(d);
}

void Call::ActualsFinder(Decl *d) {
    for (int i = 0, n = actuals->NumElements(); i < n; ++i)
        actuals->Nth(i)->Check();

    FnDecl *fnDecl = dynamic_cast<FnDecl*>(d);
    if (fnDecl == NULL)
        return;

    List<VarDecl*> *formals = fnDecl->GetFormals();

    int numExpected = formals->NumElements();
    int numGiven = actuals->NumElements();
    if (numExpected != numGiven) {
        
        ReportError::NumArgsMismatch(field, numExpected, numGiven);
        return;
    }

    for (int i = 0, n = actuals->NumElements(); i < n; ++i) {
        Type *given = actuals->Nth(i)->TypeFinder();
        Type *expected = formals->Nth(i)->TypeFinder();
        
        if (!given->AreEquiv(expected))
            ReportError::ArgMismatch(actuals->Nth(i), i+1, given, expected);
    }
}

NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) { 
  Assert(c != NULL);
  (cType=c)->SetParent(this);
}

Type* NewExpr::TypeFinder() {
    Decl *d = Program::G_Scope->table->Lookup(cType->Name());
    ClassDecl *c = dynamic_cast<ClassDecl*>(d);

    if (c == NULL)
        return Type::errorType;

    return c->TypeFinder();
}

void NewExpr::Check() {
    Decl *d = Program::G_Scope->table->Lookup(cType->Name());
    ClassDecl *c = dynamic_cast<ClassDecl*>(d);

    
    if (c == NULL)
        ReportError::IdentifierNotDeclared(cType->GetId(), LookingForClass);
}

NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this); 
    (elemType=et)->SetParent(this);
}

Type* NewArrayExpr::TypeFinder() {
    return new ArrayType(elemType);
}

void NewArrayExpr::ScopeMake(Scope *parent) {
    scope->SetParent(parent);

    size->ScopeMake(scope);
}

void NewArrayExpr::Check() {
    size->Check();

    
    if (size->TypeFinder() != Type::errorType && !size->TypeFinder()->IsEqualTo(Type::intType))
        ReportError::NewArraySizeNotInteger(size);

    if (elemType->IsPrimitive() && !elemType->AreEquiv(Type::voidType))
        return;

    Decl *d = Program::G_Scope->table->Lookup(elemType->Name());
    if (dynamic_cast<ClassDecl*>(d) == NULL)
        elemType->RepUndeclaredId(LookingForType);
}

Type* ReadIntegerExpr::TypeFinder() {
    return Type::intType;
}

Type* ReadLineExpr::TypeFinder() {
    return Type::stringType;
}

       
