

#include "ast_decl.h"
#include "ast_expr.h"
#include "ast_stmt.h"
#include "ast_type.h"

Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    printf("\n");
}

void Program::Check() {
    
    if (IsDebugOn("ast")) { this->Print(0); }

    
    ScopeM = new scopeST(); decls->CheckAll(E_BuildST);
    if (IsDebugOn("st")) { ScopeM->Print(); }
    
    if (IsDebugOn("ast+")) { this->Print(0); }

    
    ScopeM->ReEnter(); decls->CheckAll(E_CheckDecl);
    
    if (IsDebugOn("ast+")) { this->Print(0); }

    
    ScopeM->ReEnter(); decls->CheckAll(E_CheckInherit);
    
    if (IsDebugOn("ast+")) { this->Print(0); }

    
    ScopeM->ReEnter(); decls->CheckAll(E_CheckType);
    
    if (IsDebugOn("ast+")) { this->Print(0); }
}

void Program::Emit() {
    

    
    bool has_main = false;
    for (int i = 0; i < decls->NumElements(); i++) {
        Decl *d = decls->Nth(i);
        if (d->IsFnDecl()) {
            if (!strcmp(d->GetId()->GetIdName(), "main")) {
                has_main = true;
                break;
            }
        }
    }
    if (!has_main) {
        ReportError::NoMainFound();
        return;
    }

    
    
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->AssignOffset();
    }
    
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->AddPrefixToMethods();
    }
    if (IsDebugOn("tac+")) { this->Print(0); }

    
    decls->EmitAll();
    if (IsDebugOn("tac+")) { this->Print(0); }

    
    CG->DoFinalCodeGen();
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

void StmtBlock::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    stmts->PrintAll(indentLevel+1);
}

void StmtBlock::BuildST() {
    ScopeM->BuildScope();
    decls->CheckAll(E_BuildST);
    stmts->CheckAll(E_BuildST);
    ScopeM->ExitScope();
}

void StmtBlock::Check(checkT c) {
    if (c == E_BuildST) {
        this->BuildST();
    } else {
        ScopeM->EnterScope();
        decls->CheckAll(c);
        stmts->CheckAll(c);
        ScopeM->ExitScope();
    }
}

void StmtBlock::Emit() {
    decls->EmitAll();
    stmts->EmitAll();
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) {
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this);
    (body=b)->SetParent(this);
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) {
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
}

void ForStmt::PrintChildren(int indentLevel) {
    init->Print(indentLevel+1, "(init) ");
    test->Print(indentLevel+1, "(test) ");
    step->Print(indentLevel+1, "(step) ");
    body->Print(indentLevel+1, "(body) ");
}

void ForStmt::BuildST() {
    ScopeM->BuildScope();
    body->Check(E_BuildST);
    ScopeM->ExitScope();
}

void ForStmt::CheckType() {
    init->Check(E_CheckType);
    test->Check(E_CheckType);
    if (test->GetType() && test->GetType() != Type::boolType) {
        ReportError::TestNotBoolean(test);
    }
    step->Check(E_CheckType);
    ScopeM->EnterScope();
    body->Check(E_CheckType);
    ScopeM->ExitScope();
}

void ForStmt::Check(checkT c) {
    switch (c) {
        case E_BuildST:
            this->BuildST(); break;
        case E_CheckType:
            this->CheckType(); break;
        default:
            init->Check(c);
            test->Check(c);
            step->Check(c);
            ScopeM->EnterScope();
            body->Check(c);
            ScopeM->ExitScope();
    }
}

void ForStmt::Emit() {
    init->Emit();

    const char *l0 = CG->NewLabel();
    CG->GenLabel(l0);
    test->Emit();
    Location *t0 = test->GetEmitLocDeref();
    const char *l1 = CG->NewLabel();
    end_loop_label = l1;
    CG->GenIfZ(t0, l1);

    body->Emit();
    step->Emit();
    CG->GenGoto(l0);

    CG->GenLabel(l1);
}

void WhileStmt::PrintChildren(int indentLevel) {
    test->Print(indentLevel+1, "(test) ");
    body->Print(indentLevel+1, "(body) ");
}

void WhileStmt::BuildST() {
    ScopeM->BuildScope();
    body->Check(E_BuildST);
    ScopeM->ExitScope();
}

void WhileStmt::CheckType() {
    test->Check(E_CheckType);
    if (test->GetType() && test->GetType() != Type::boolType) {
        ReportError::TestNotBoolean(test);
    }
    ScopeM->EnterScope();
    body->Check(E_CheckType);
    ScopeM->ExitScope();
}

void WhileStmt::Check(checkT c) {
    switch (c) {
        case E_BuildST:
            this->BuildST(); break;
        case E_CheckType:
            this->CheckType(); break;
        default:
            test->Check(c);
            ScopeM->EnterScope();
            body->Check(c);
            ScopeM->ExitScope();
    }
}

void WhileStmt::Emit() {
    const char *l0 = CG->NewLabel();
    CG->GenLabel(l0);

    test->Emit();
    Location *t0 = test->GetEmitLocDeref();
    const char *l1 = CG->NewLabel();
    end_loop_label = l1;
    CG->GenIfZ(t0, l1);

    body->Emit();
    CG->GenGoto(l0);

    CG->GenLabel(l1);
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) {
    Assert(t != NULL && tb != NULL); 
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

void IfStmt::PrintChildren(int indentLevel) {
    test->Print(indentLevel+1, "(test) ");
    body->Print(indentLevel+1, "(then) ");
    if (elseBody) elseBody->Print(indentLevel+1, "(else) ");
}

void IfStmt::BuildST() {
    ScopeM->BuildScope();
    body->Check(E_BuildST);
    ScopeM->ExitScope();
    if (elseBody) {
        ScopeM->BuildScope();
        elseBody->Check(E_BuildST);
        ScopeM->ExitScope();
    }
}

void IfStmt::CheckType() {
    test->Check(E_CheckType);
    if (test->GetType() && test->GetType() != Type::boolType) {
        ReportError::TestNotBoolean(test);
    }
    ScopeM->EnterScope();
    body->Check(E_CheckType);
    ScopeM->ExitScope();
    if (elseBody) {
        ScopeM->EnterScope();
        elseBody->Check(E_CheckType);
        ScopeM->ExitScope();
    }
}

void IfStmt::Check(checkT c) {
    switch (c) {
        case E_BuildST:
            this->BuildST(); break;
        case E_CheckType:
            this->CheckType(); break;
        default:
            test->Check(c);
            ScopeM->EnterScope();
            body->Check(c);
            ScopeM->ExitScope();
            if (elseBody) {
                ScopeM->EnterScope();
                elseBody->Check(c);
                ScopeM->ExitScope();
            }
    }
}

void IfStmt::Emit() {
    test->Emit();
    Location *t0 = test->GetEmitLocDeref();
    const char *l0 = CG->NewLabel();
    CG->GenIfZ(t0, l0);

    body->Emit();
    const char *l1 = CG->NewLabel();
    CG->GenGoto(l1);

    CG->GenLabel(l0);
    if (elseBody) elseBody->Emit();
    CG->GenLabel(l1);
}

void BreakStmt::Check(checkT c) {
    if (c == E_CheckType) {
        Node *n = this;
        while (n->GetParent()) {
            if (n->IsLoopStmt() || n->IsCaseStmt()) return;
            n = n->GetParent();
        }
        ReportError::BreakOutsideLoop(this);
    }
}

void BreakStmt::Emit() {
    
    Node *n = this;
    while (n->GetParent()) {
        if (n->IsLoopStmt()) {
            const char *l = dynamic_cast<LoopStmt*>(n)->GetEndLoopLabel();
            
            CG->GenGoto(l);
            return;
        } else if (n->IsSwitchStmt()) {
            const char *l = dynamic_cast<SwitchStmt*>(n)->GetEndSwitchLabel();
            
            CG->GenGoto(l);
            return;
        }
        n = n->GetParent();
    }
}

CaseStmt::CaseStmt(IntConstant *v, List<Stmt*> *s) {
    Assert(s != NULL);
    value = v;
    if (value) value->SetParent(this);
    (stmts=s)->SetParentAll(this);
    case_label = NULL;
}

void CaseStmt::PrintChildren(int indentLevel) {
    if (value) value->Print(indentLevel+1);
    stmts->PrintAll(indentLevel+1);
}

void CaseStmt::BuildST() {
    ScopeM->BuildScope();
    stmts->CheckAll(E_BuildST);
    ScopeM->ExitScope();
}

void CaseStmt::Check(checkT c) {
    if (c == E_BuildST) {
        this->BuildST();
    } else {
        if (value) value->Check(c);
        ScopeM->EnterScope();
        stmts->CheckAll(c);
        ScopeM->ExitScope();
    }
}

void CaseStmt::GenCaseLabel() {
    case_label = CG->NewLabel();
}

void CaseStmt::Emit() {
    CG->GenLabel(case_label);
    stmts->EmitAll();
}

SwitchStmt::SwitchStmt(Expr *e, List<CaseStmt*> *c) {
    Assert(e != NULL && c != NULL);
    (expr=e)->SetParent(this);
    (cases=c)->SetParentAll(this);
    end_switch_label = NULL;
}

void SwitchStmt::PrintChildren(int indentLevel) {
    expr->Print(indentLevel+1);
    cases->PrintAll(indentLevel+1);
}

void SwitchStmt::BuildST() {
    ScopeM->BuildScope();
    cases->CheckAll(E_BuildST);
    ScopeM->ExitScope();
}

void SwitchStmt::Check(checkT c) {
    if (c == E_BuildST) {
        this->BuildST();
    } else {
        expr->Check(c);
        ScopeM->EnterScope();
        cases->CheckAll(c);
        ScopeM->ExitScope();
    }
}

void SwitchStmt::Emit() {
    expr->Emit();

    
    end_switch_label = CG->NewLabel();

    Location *switch_value = expr->GetEmitLocDeref();

    
    
    
    for (int i = 0; i < cases->NumElements(); i++) {
        CaseStmt *c = cases->Nth(i);

        
        c->GenCaseLabel();
        const char *cl = c->GetCaseLabel();

        
        IntConstant *cv = c->GetCaseValue();

        
        if (cv) {
            
            cv->Emit();
            Location *cvl = cv->GetEmitLocDeref();
            Location *t = CG->GenBinaryOp("!=", switch_value, cvl);
            CG->GenIfZ(t, cl);
        } else {
            
            CG->GenGoto(cl);
        }
    }

    
    cases->EmitAll();

    
    CG->GenLabel(end_switch_label);
}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) {
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}

void ReturnStmt::PrintChildren(int indentLevel) {
    expr->Print(indentLevel+1);
}

void ReturnStmt::Check(checkT c) {
    expr->Check(c);
    if (c == E_CheckType) {
        Node *n = this;
        
        while (n->GetParent()) {
            if (dynamic_cast<FnDecl*>(n) != NULL) break;
            n = n->GetParent();
        }
        Type *t_given = expr->GetType();
        Type *t_expected = dynamic_cast<FnDecl*>(n)->GetType();
        if (t_given && t_expected) {
            if (!t_expected->IsCompatibleWith(t_given)) {
                ReportError::ReturnMismatch(this, t_given, t_expected);
            }
        }
    }
}

void ReturnStmt::Emit() {
    if (expr->IsEmptyExpr()) {
        CG->GenReturn();
    } else {
        expr->Emit();
        CG->GenReturn(expr->GetEmitLocDeref());
    }
}

PrintStmt::PrintStmt(List<Expr*> *a) {
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}

void PrintStmt::PrintChildren(int indentLevel) {
    args->PrintAll(indentLevel+1, "(args) ");
}

void PrintStmt::Check(checkT c) {
    args->CheckAll(c);
    if (c == E_CheckType) {
        for (int i = 0; i < args->NumElements(); i++) {
            Type *t = args->Nth(i)->GetType();
            if (t != NULL && t != Type::stringType && t != Type::intType
                     && t != Type::boolType) {
                ReportError::PrintArgMismatch(args->Nth(i), i + 1, t);
            }
        }
    }
}

void PrintStmt::Emit() {
    for (int i = 0; i < args->NumElements(); i++) {
        args->Nth(i)->Emit();
        
        Type *t = args->Nth(i)->GetType();
        BuiltIn f;
        if (t == Type::intType) {
            f = PrintInt;
        } else if (t == Type::stringType) {
            f = PrintString;
        } else {
            f = PrintBool;
        }
        Location *l = args->Nth(i)->GetEmitLocDeref();
        Assert(l);
        CG->GenBuiltInCall(f, l);
    }
}

