

#include <string.h>
#include "ast_decl.h"
#include "ast_type.h"
#include "errors.h"



Type *Type::intType    = new Type("int");
Type *Type::doubleType = new Type("double");
Type *Type::voidType   = new Type("void");
Type *Type::boolType   = new Type("bool");
Type *Type::nullType   = new Type("null");
Type *Type::stringType = new Type("string");
Type *Type::errorType  = new Type("error");

Type::Type(const char *n) {
    Assert(n);
    typeName = strdup(n);
    expr_type = NULL;
}

void Type::PrintChildren(int indentLevel) {
    printf("%s", typeName);
    if (expr_type) std::cout << " <" << expr_type << ">";
    if (emit_loc) emit_loc->Print();
}

void Type::Check(checkT c) {
    if (c == E_CheckDecl) {
        Type::intType->SetSelfType();
        Type::doubleType->SetSelfType();
        Type::voidType->SetSelfType();
        Type::boolType->SetSelfType();
        Type::nullType->SetSelfType();
        Type::stringType->SetSelfType();
        Type::errorType->SetSelfType();
        expr_type = this;
    }
}

NamedType::NamedType(Identifier *i) : Type(*i->GetLocation()) {
    Assert(i != NULL);
    (id=i)->SetParent(this);
}

void NamedType::PrintChildren(int indentLevel) {
    if (expr_type) std::cout << " <" << expr_type << ">";
    if (emit_loc) emit_loc->Print();
    id->Print(indentLevel+1);
}

void NamedType::CheckDecl(reasonT r) {
    Decl *d = ScopeM->Lookup(this->id);
    if (d == NULL || (!d->IsClassDecl() && !d->IsInterfaceDecl())) {
        ReportError::IdentifierNotDeclared(this->id, r);
    } else if (r == LookingForClass && !d->IsClassDecl()) {
        ReportError::IdentifierNotDeclared(this->id, r);
    } else if (r == LookingForInterface && !d->IsInterfaceDecl()) {
        ReportError::IdentifierNotDeclared(this->id, r);
    } else {
        this->id->SetDecl(d);
        expr_type = this;
    }
}

void NamedType::Check(checkT c, reasonT r) {
    if (c == E_CheckDecl) {
        this->CheckDecl(r);
    } else {
        id->Check(c);
    }
}

bool NamedType::IsEquivalentTo(Type *other) {
    Assert(this->GetType() && other->GetType());

    if (!other->IsNamedType()) {
        return false;
    } else {
        NamedType * nt = dynamic_cast<NamedType*>(other);
        return (id->IsEquivalentTo(nt->GetId()));
    }
}


bool NamedType::IsCompatibleWith(Type *other) {
    Assert(this->GetType() && other->GetType());

    if (other == nullType) {
        return true;
    } else if (!other->IsNamedType()) {
        return false;
    } else if (this->IsEquivalentTo(other)) {
        return true;
    } else {
        NamedType * nt = dynamic_cast<NamedType*>(other);
        Decl *decl1 = id->GetDecl();
        Decl *decl2 = nt->GetId()->GetDecl();
        Assert(decl1 && decl2);
        if (!decl2->IsClassDecl()) {
            return false;
        }
        ClassDecl *cdecl2 = dynamic_cast<ClassDecl*>(decl2);
        
        return cdecl2->IsChildOf(decl1);
    }
}

ArrayType::ArrayType(yyltype loc, Type *et) : Type(loc) {
    Assert(et != NULL);
    (elemType=et)->SetParent(this);
}
void ArrayType::PrintChildren(int indentLevel) {
    if (expr_type) std::cout << " <" << expr_type << ">";
    if (emit_loc) emit_loc->Print();
    elemType->Print(indentLevel+1);
}

void ArrayType::CheckDecl() {
    elemType->Check(E_CheckDecl);
    if (elemType->GetType()) {
        expr_type = this;
    }
}

void ArrayType::Check(checkT c) {
    if (c == E_CheckDecl) {
        this->CheckDecl();
    } else {
        elemType->Check(c);
    }
}

bool ArrayType::IsEquivalentTo(Type *other) {
    Assert(this->GetType() && other->GetType());

    if (!other->IsArrayType()) {
        return false;
    } else {
        ArrayType * nt = dynamic_cast<ArrayType*>(other);
        return (elemType->IsEquivalentTo(nt->GetElemType()));
    }
}

bool ArrayType::IsCompatibleWith(Type *other) {
    Assert(this->GetType() && other->GetType());

    if (other == nullType) {
        return elemType->IsCompatibleWith(other);
    } else {
        return this->IsEquivalentTo(other);
    }
}

