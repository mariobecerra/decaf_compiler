

#include <stdio.h>  
#include <string.h> 
#include "ast.h"
#include "ast_decl.h"
#include "ast_type.h"
#include "errors.h"


CodeGenerator *CG = new CodeGenerator();

Node::Node(yyltype loc) {
    location = new yyltype(loc);
    parent = NULL;
}

Node::Node() {
    location = NULL;
    parent = NULL;
}


void Node::Print(int indentLevel, const char *label) {
    const int numSpaces = 3;
    printf("\n");
    if (GetLocation())
        printf("%*d", numSpaces, GetLocation()->first_line);
    else
        printf("%*s", numSpaces, "");
    printf("%*s%s%s: ", indentLevel*numSpaces, "",
           label? label : "", GetPrintNameForNode());
    PrintChildren(indentLevel);
}

Identifier::Identifier(yyltype loc, const char *n) : Node(loc) {
    name = strdup(n);
}

void Identifier::PrintChildren(int indentLevel) {
    printf("%s", name);
    if (decl) printf(" ---------------- {%d}", decl->GetIndex());
}

void Identifier::CheckDecl() {
    Decl *d = ScopeM->Lookup(this);
    if (d == NULL) {
        ReportError::IdentifierNotDeclared(this, LookingForVariable);
    } else {
        this->SetDecl(d);
    }
}

void Identifier::Check(checkT c) {
    if (c == E_CheckDecl) {
        this->CheckDecl();
    }
}

bool Identifier::IsEquivalentTo(Identifier *other) {
    bool eq = false;
    if (!strcmp(name, other->GetIdName())) eq = true;
    return eq;
}

void Identifier::Emit() {
    if (decl)
        emit_loc = decl->GetEmitLoc();
}

void Identifier::AddPrefix(const char *prefix) {
    char *s = (char *)malloc(strlen(name) + strlen(prefix) + 1);
    sprintf(s, "%s%s", prefix, name);
    name = s;
}

