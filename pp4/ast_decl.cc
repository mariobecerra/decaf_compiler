/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/// Decl
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////        
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()), scope(new Scope) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}

bool Decl::AreEquiv(Decl *other) {
    return true;
}

void Decl::ScopeMake(Scope *parent) {
    scope->SetParent(parent);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/// VarDecl
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
}

bool VarDecl::AreEquiv(Decl *other) {
    VarDecl *varDecl = dynamic_cast<VarDecl*>(other);
    if (varDecl == NULL)
        return false;

    return type->AreEquiv(varDecl->type);
}

void VarDecl::Check() {
    FindType();
}

void VarDecl::FindType() {
    if (type->IsPrimitive())
        return;

    Scope *s = scope;
    while (s != NULL) {
        Decl *d;
        if ((d = s->table->Lookup(type->Name())) != NULL) {
            if (dynamic_cast<ClassDecl*>(d) == NULL &&
                dynamic_cast<InterfaceDecl*>(d) == NULL) {
                type->RepUndeclaredId(LookingForType);
                type->TD = false; // Tag it as an undeclared type, so the field name check can be skipped in the future.
            }

            return;
        }
        s = s->GetParent();
    }

    type->RepUndeclaredId(LookingForType);
    type->TD = false; // Tag it as an undeclared type, so the field name check can be skipped in the future.
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/// ClassDecl
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);     
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
}

void ClassDecl::ScopeMake(Scope *parent) {
    scope->SetParent(parent);
    scope->SetClassDecl(this);

    for (int i = 0, n = members->NumElements(); i < n; ++i)
        scope->Add_Declaration(members->Nth(i));

    for (int i = 0, n = members->NumElements(); i < n; ++i)
        members->Nth(i)->ScopeMake(scope);
}

void ClassDecl::Check() {
    for (int i = 0, n = members->NumElements(); i < n; ++i)
        members->Nth(i)->Check();

    ExtendsFinder();
    ImplementsFinder();

    for (int i = 0, n = implements->NumElements(); i < n; ++i)
        ImplementedMembersFinder(implements->Nth(i));

    ExtendedMembersFinder(extends);
    ImplementsFinderInterfaces();
}

void ClassDecl::ExtendsFinder() {
    if (extends == NULL)
        return;

    Decl *lookup = scope->GetParent()->table->Lookup(extends->Name());
    if (dynamic_cast<ClassDecl*>(lookup) == NULL)
        extends->RepUndeclaredId(LookingForClass);
}

void ClassDecl::ImplementsFinder() {
    Scope *s = scope->GetParent();

    for (int i = 0, n = implements->NumElements(); i < n; ++i) {
        NamedType *nth = implements->Nth(i);
        Decl *lookup = s->table->Lookup(implements->Nth(i)->Name());

        if (dynamic_cast<InterfaceDecl*>(lookup) == NULL)
            nth->RepUndeclaredId(LookingForInterface);
    }
}

void ClassDecl::ExtendedMembersFinder(NamedType *extType) {
    if (extType == NULL)
        return;

    Decl *lookup = scope->GetParent()->table->Lookup(extType->Name());
    ClassDecl *extDecl = dynamic_cast<ClassDecl*>(lookup);
    if (extDecl == NULL)
        return;

    ExtendedMembersFinder(extDecl->extends);
    AgScopeFinder(extDecl->scope);
}

void ClassDecl::ImplementedMembersFinder(NamedType *impType) {
    Decl *lookup = scope->GetParent()->table->Lookup(impType->Name());
    InterfaceDecl *intDecl = dynamic_cast<InterfaceDecl*>(lookup);
    if (intDecl == NULL)
        return;

    AgScopeFinder(intDecl->GetScope());
}

void ClassDecl::AgScopeFinder(Scope *other) {
    Iterator<Decl*> iter = scope->table->GetIterator();
    Decl *d;
    while ((d = iter.GetNextValue()) != NULL) {
        Decl *lookup = other->table->Lookup(d->Name());

        if (lookup == NULL)
            continue;

        if (dynamic_cast<VarDecl*>(lookup) != NULL)
            ReportError::DeclConflict(d, lookup);

        if (dynamic_cast<FnDecl*>(lookup) != NULL &&
            !d->AreEquiv(lookup))
            ReportError::OverrideMismatch(d);
    }
}

void ClassDecl::ImplementsFinderInterfaces() {
    Scope *s = scope->GetParent();

    for (int i = 0, n = implements->NumElements(); i < n; ++i) {
        NamedType *nth = implements->Nth(i);
        Decl *lookup = s->table->Lookup(implements->Nth(i)->Name());
        InterfaceDecl *intDecl = dynamic_cast<InterfaceDecl*>(lookup);

        if (intDecl == NULL)
            continue;

        List<Decl*> *intMembers = intDecl->GetMembers();

        for (int i = 0, n = intMembers->NumElements(); i < n; ++i) {
            Decl *d = intMembers->Nth(i);

            ClassDecl *classDecl = this;
            Decl *classLookup;
            while (classDecl != NULL) {
                classLookup = classDecl->GetScope()->table->Lookup(d->Name());

                if (classLookup != NULL)
                    break;

                if (classDecl->GetExtends() == NULL) {
                    classDecl = NULL;
                } else {
                    const char *extName = classDecl->GetExtends()->Name();
                    Decl *ext = Program::G_Scope->table->Lookup(extName);
                    classDecl = dynamic_cast<ClassDecl*>(ext);
                }
            }

            if (classLookup == NULL) {
                ReportError::InterfaceNotImplemented(this, nth);
                return;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/// InterfaceDecl
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
}

void InterfaceDecl::ScopeMake(Scope *parent) {
    scope->SetParent(parent);

    for (int i = 0, n = members->NumElements(); i < n; ++i)
        scope->Add_Declaration(members->Nth(i));

    for (int i = 0, n = members->NumElements(); i < n; ++i)
        members->Nth(i)->ScopeMake(scope);
}

void InterfaceDecl::Check() {
    for (int i = 0, n = members->NumElements(); i < n; ++i)
        members->Nth(i)->Check();
}
	
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/// FnDecl
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

bool FnDecl::AreEquiv(Decl *other) {
    FnDecl *fnDecl = dynamic_cast<FnDecl*>(other);

    if (fnDecl == NULL)
        return false;

    if (!returnType->AreEquiv(fnDecl->returnType))
        return false;

    if (formals->NumElements() != fnDecl->formals->NumElements())
        return false;

    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        if (!formals->Nth(i)->AreEquiv(fnDecl->formals->Nth(i)))
            return false;

    return true;
}

void FnDecl::ScopeMake(Scope *parent) {
    scope->SetParent(parent);
    scope->SetFnDecl(this);

    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        scope->Add_Declaration(formals->Nth(i));

    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        formals->Nth(i)->ScopeMake(scope);

    if (body)
        body->ScopeMake(scope);
}

void FnDecl::Check() {
    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        formals->Nth(i)->Check();

    if (body)
        body->Check();
}

