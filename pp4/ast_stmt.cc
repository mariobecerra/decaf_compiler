/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"
#include "ast_type.h"

int Scope::Add_Declaration(Decl *d) {
    Decl *lookup = table->Lookup(d->Name());

    if (lookup != NULL) {
        ReportError::DeclConflict(d, lookup); // Conflict Declarations with the same name.
        return 1;
    }

    table->Enter(d->Name(), d);
    return 0;
}

Scope *Program::G_Scope = new Scope();

Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::Check() {
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */
    
    ScopeMake();

    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        decls->Nth(i)->Check();
}

void Program::ScopeMake() {
    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        G_Scope->Add_Declaration(decls->Nth(i));

    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        decls->Nth(i)->ScopeMake(G_Scope);
}

void Stmt::ScopeMake(Scope *parent) {
    scope->SetParent(parent);
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

void StmtBlock::ScopeMake(Scope *parent) {
    scope->SetParent(parent);

    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        scope->Add_Declaration(decls->Nth(i));

    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        decls->Nth(i)->ScopeMake(scope);

    for (int i = 0, n = stmts->NumElements(); i < n; ++i)
        stmts->Nth(i)->ScopeMake(scope);
}

void StmtBlock::Check() {
    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        decls->Nth(i)->Check();

    for (int i = 0, n = stmts->NumElements(); i < n; ++i)
        stmts->Nth(i)->Check();
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}

void ConditionalStmt::ScopeMake(Scope *parent) {
    scope->SetParent(parent);

    test->ScopeMake(scope);
    body->ScopeMake(scope);
}

void ConditionalStmt::Check() {
    test->Check();
    body->Check();

    if (!test->TypeFinder()->AreEquiv(Type::boolType))
        ReportError::TestNotBoolean(test);
}

void LoopStmt::ScopeMake(Scope *parent) {
    scope->SetParent(parent);
    scope->SetLoopStmt(this);

    test->ScopeMake(scope);
    body->ScopeMake(scope);
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

void IfStmt::ScopeMake(Scope *parent) {
    scope->SetParent(parent);

    test->ScopeMake(scope);
    body->ScopeMake(scope);

    if (elseBody != NULL)
        elseBody->ScopeMake(scope);
}

void IfStmt::Check() {
    test->Check();
    body->Check();

    if (!test->TypeFinder()->AreEquiv(Type::boolType))
        ReportError::TestNotBoolean(test);

    if (elseBody != NULL)
        elseBody->Check();
}

void BreakStmt::Check() {
    Scope *s = scope;
    while (s != NULL) {
        if (s->GetLoopStmt() != NULL)
            return;
        if (s->GetSwitchStmt() != NULL)
            return;

        s = s->GetParent();
    }

    ReportError::BreakOutsideLoop(this);
}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}

void ReturnStmt::ScopeMake(Scope *parent) {
    scope->SetParent(parent);

    expr->ScopeMake(scope);
}

void ReturnStmt::Check() {
    expr->Check();

    FnDecl *d = NULL;
    Scope *s = scope;
    while (s != NULL) {
        if ((d = s->GetFnDecl()) != NULL)
            break;

        s = s->GetParent();
    }

    if (d == NULL) {
        ReportError::Formatted(location,
                               "return is only allowed inside a function");
        return;
    }

    Type *expected = d->GetReturnType();
    Type *given = expr->TypeFinder();

    if (!given->AreEquiv(expected))
        ReportError::ReturnMismatch(this, given, expected);
    
    EmptyExpr *ee = dynamic_cast<EmptyExpr*>(expr);
    if (ee != NULL && expected != Type::voidType)
        ReportError::ReturnMismatch(this, Type::voidType, expected);
}
  
PrintStmt::PrintStmt(List<Expr*> *a) {    
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}

void PrintStmt::ScopeMake(Scope *parent) {
    scope->SetParent(parent);

    for (int i = 0, n = args->NumElements(); i < n; ++i)
        args->Nth(i)->ScopeMake(scope);
}

void PrintStmt::Check() {
    for (int i = 0, n = args->NumElements(); i < n; ++i) {
        Type *given = args->Nth(i)->TypeFinder();

        if (!(given->AreEquiv(Type::intType) ||
              given->AreEquiv(Type::boolType) ||
              given->AreEquiv(Type::stringType)))
            ReportError::PrintArgMismatch(args->Nth(i), i+1, given);
    }

    for (int i = 0, n = args->NumElements(); i < n; ++i)
        args->Nth(i)->Check();
}

SwitchStmt::SwitchStmt(Expr *e, List<CaseStmt*> *s) {
    Assert(e != NULL && s != NULL); // DefaultStmt can be NULL
    (expr=e)->SetParent(this);
    (caseStmts=s)->SetParentAll(this);
}

void SwitchStmt::ScopeMake(Scope *parent) {
    scope->SetParent(parent);
    scope->SetSwitchStmt(this);

    expr->ScopeMake(scope);
    for (int i = 0, n = caseStmts->NumElements(); i < n; ++i)
        caseStmts->Nth(i)->ScopeMake(scope);
}

void SwitchStmt::Check() {
    expr->Check();
    for (int i = 0, n = caseStmts->NumElements(); i < n; ++i)
        caseStmts->Nth(i)->Check();
}

SwitchStmt::CaseStmt::CaseStmt(Expr *e, List<Stmt*> *s) {
    Assert(s != NULL);
    
    intConst=e;
    if (intConst != NULL)
        intConst->SetParent(this);
        
    (caseBody=s)->SetParentAll(this);
}

void SwitchStmt::CaseStmt::ScopeMake(Scope *parent) {
    scope->SetParent(parent);
    for (int i = 0, n = caseBody->NumElements(); i < n; ++i)
        caseBody->Nth(i)->ScopeMake(scope);
}

void SwitchStmt::CaseStmt::Check() {
    for (int i = 0, n = caseBody->NumElements(); i < n; ++i)
        caseBody->Nth(i)->Check();
}
