

#include "ast_decl.h"
#include "ast_stmt.h"
#include "ast_type.h"
#include "list.h"
#include "errors.h"

Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this);
    idx = -1;
    expr_type = NULL;
}

VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
    class_member_ofst = -1;
}

void VarDecl::PrintChildren(int indentLevel) {
    if (expr_type) std::cout << " <" << expr_type << ">";
    if (emit_loc) emit_loc->Print();
    if (class_member_ofst != -1)
        std::cout << " ~~[Ofst: " << class_member_ofst << "]";
    type->Print(indentLevel+1);
    id->Print(indentLevel+1);
    if (id->GetDecl()) printf(" ........ {def}");
}

void VarDecl::BuildST() {
    if (ScopeM->LocalLookup(this->GetId())) {
        Decl *d = ScopeM->Lookup(this->GetId());
        ReportError::DeclConflict(this, d);
    } else {
        idx = ScopeM->InsertSymbol(this);
        id->SetDecl(this);
    }
}

void VarDecl::CheckDecl() {
    type->Check(E_CheckDecl);
    id->Check(E_CheckDecl);

    expr_type = type->GetType();
}

void VarDecl::Check(checkT c) {
    switch (c) {
        case E_CheckDecl:
            this->CheckDecl(); break;
        case E_BuildST:
            this->BuildST(); break;
        default:
            type->Check(c);
            id->Check(c);
    }
}

void VarDecl::AssignOffset() {
    if (this->IsGlobal()) {
        emit_loc = new Location(gpRelative, CG->GetNextGlobalLoc(),
                id->GetIdName());
    }
}

void VarDecl::AssignMemberOffset(bool inClass, int offset) {
    class_member_ofst = offset;
    
    emit_loc = new Location(fpRelative, offset, id->GetIdName(), CG->ThisPtr);
}

void VarDecl::Emit() {
    if (type == Type::doubleType) {
        ReportError::Formatted(this->GetLocation(),
                "Double type is not supported by compiler back end yet.");
        Assert(0);
    }

    if (!emit_loc) {
        
        emit_loc = new Location(fpRelative, CG->GetNextLocalLoc(),
                id->GetIdName());
    }
}

ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp,
        List<Decl*> *m) : Decl(n) {
    
    Assert(n != NULL && imp != NULL && m != NULL);
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
    instance_size = 4;
    vtable_size = 0;
}

void ClassDecl::PrintChildren(int indentLevel) {
    if (expr_type) std::cout << " <" << expr_type << ">";
    if (emit_loc) emit_loc->Print();
    id->Print(indentLevel+1);
    if (id->GetDecl()) printf(" ........ {def}");
    if (extends) extends->Print(indentLevel+1, "(extends) ");
    implements->PrintAll(indentLevel+1, "(implements) ");
    members->PrintAll(indentLevel+1);
}

void ClassDecl::BuildST() {
    if (ScopeM->LocalLookup(this->GetId())) {
        
        Decl *d = ScopeM->Lookup(this->GetId());
        ReportError::DeclConflict(this, d);
    } else {
        idx = ScopeM->InsertSymbol(this);
        id->SetDecl(this);
    }
    
    ScopeM->BuildScope(this->GetId()->GetIdName());
    if (extends) {
        
        ScopeM->SetScopeParent(extends->GetId()->GetIdName());
    }
    
    for (int i = 0; i < implements->NumElements(); i++) {
        ScopeM->SetInterface(implements->Nth(i)->GetId()->GetIdName());
    }
    members->CheckAll(E_BuildST);
    ScopeM->ExitScope();
}

void ClassDecl::CheckDecl() {
    id->Check(E_CheckDecl);
    
    if (extends) {
        extends->Check(E_CheckDecl, LookingForClass);
    }
    
    for (int i = 0; i < implements->NumElements(); i++) {
        implements->Nth(i)->Check(E_CheckDecl, LookingForInterface);
    }
    ScopeM->EnterScope();
    members->CheckAll(E_CheckDecl);
    ScopeM->ExitScope();

    expr_type = new NamedType(id);
    expr_type->SetSelfType();
}

void ClassDecl::CheckInherit() {
    ScopeM->EnterScope();

    for (int i = 0; i < members->NumElements(); i++) {
        Decl *d = members->Nth(i);
        Assert(d != NULL); 

        if (d->IsVarDecl()) {
            
            Decl *t = ScopeM->LookupParent(d->GetId());
            if (t != NULL) {
                
                ReportError::DeclConflict(d, t);
            }
            
            t = ScopeM->LookupInterface(d->GetId());
            if (t != NULL) {
                
                ReportError::DeclConflict(d, t);
            }

        } else if (d->IsFnDecl()) {
            
            Decl *t = ScopeM->LookupParent(d->GetId());
            if (t != NULL) {
                if (!t->IsFnDecl()) {
                    ReportError::DeclConflict(d, t);
                } else {
                    
                    FnDecl *fn1 = dynamic_cast<FnDecl*>(d);
                    FnDecl *fn2 = dynamic_cast<FnDecl*>(t);
                    if (fn1->GetType() && fn2->GetType() 
                            && !fn1->IsEquivalentTo(fn2)) {
                        
                        ReportError::OverrideMismatch(d);
                    }
                }
            }
            
            t = ScopeM->LookupInterface(d->GetId());
            if (t != NULL) {
                
                FnDecl *fn1 = dynamic_cast<FnDecl*>(d);
                FnDecl *fn2 = dynamic_cast<FnDecl*>(t);
                if (fn1->GetType() && fn2->GetType() 
                        && !fn1->IsEquivalentTo(fn2)) {
                    
                    ReportError::OverrideMismatch(d);
                }
            }
            
            d->Check(E_CheckInherit);
        }
    }

    
    for (int i = 0; i < implements->NumElements(); i++) {
        Decl *d = implements->Nth(i)->GetId()->GetDecl();
        if (d != NULL) {
            List<Decl*> *m = dynamic_cast<InterfaceDecl*>(d)->GetMembers();
            
            for (int j = 0; j < m->NumElements(); j++) {
                Identifier *mid = m->Nth(j)->GetId();
                Decl *t = ScopeM->LookupField(this->id, mid);
                if (t == NULL) {
                    ReportError::InterfaceNotImplemented(this,
                            implements->Nth(i));
                    break;
                } else {
                    
                    FnDecl *fn1 = dynamic_cast<FnDecl*>(m->Nth(j));
                    FnDecl *fn2 = dynamic_cast<FnDecl*>(t);
                    if (!fn1 || !fn2 || !fn1->GetType() || !fn2->GetType()
                            || !fn1->IsEquivalentTo(fn2)) {
                        ReportError::InterfaceNotImplemented(this,
                                implements->Nth(i));
                        break;
                    }
                }
            }
        }
    }
    ScopeM->ExitScope();
}

void ClassDecl::Check(checkT c) {
    switch (c) {
        case E_BuildST:
            this->BuildST(); break;
        case E_CheckDecl:
            this->CheckDecl(); break;
        case E_CheckInherit:
            this->CheckInherit(); break;
        default:
            id->Check(c);
            if (extends) extends->Check(c);
            implements->CheckAll(c);
            ScopeM->EnterScope();
            members->CheckAll(c);
            ScopeM->ExitScope();
    }
}

bool ClassDecl::IsChildOf(Decl *other) {
    if (other->IsClassDecl()) {
        if (id->IsEquivalentTo(other->GetId())) {
            
            return true;
        } else if (!extends) {
            return false;
        } else {
            
            Decl *d = extends->GetId()->GetDecl();
            return dynamic_cast<ClassDecl*>(d)->IsChildOf(other);
        }
    } else if (other->IsInterfaceDecl()) {
        for (int i = 0; i < implements->NumElements(); i++) {
            Identifier *iid = implements->Nth(i)->GetId();
            if (iid->IsEquivalentTo(other->GetId())) {
                return true;
            }
        }
        if (!extends) {
            return false;
        } else {
            
            Decl *d = extends->GetId()->GetDecl();
            return dynamic_cast<ClassDecl*>(d)->IsChildOf(other);
        }
    } else {
        return false;
    }
}

void ClassDecl::AddMembersToList(List<VarDecl*> *vars, List<FnDecl*> *fns) {
    for (int i = members->NumElements() - 1; i >= 0; i--) {
        Decl *d = members->Nth(i);
        if (d->IsVarDecl()) {
            vars->InsertAt(dynamic_cast<VarDecl*>(d), 0);
        } else if (d->IsFnDecl()) {
            fns->InsertAt(dynamic_cast<FnDecl*>(d), 0);
        }
    }
}

void ClassDecl::AssignOffset() {
    
    
    var_members = new List<VarDecl*>;
    methods = new List<FnDecl*>;
    ClassDecl *c = this;
    while (c) {
        c->AddMembersToList(var_members, methods);
        NamedType *t = c->GetExtends();
        if (!t) break;
        c = dynamic_cast<ClassDecl*>(t->GetId()->GetDecl());
    }

    
    for (int i = 0; i < methods->NumElements(); i++) {
        FnDecl *f1 = methods->Nth(i);
        for (int j = i + 1; j < methods->NumElements(); j++) {
            FnDecl *f2 = methods->Nth(j);
            if (!strcmp(f1->GetId()->GetIdName(), f2->GetId()->GetIdName())) {
                
                
                methods->RemoveAt(i);
                methods->InsertAt(f2, i);
                methods->RemoveAt(j);
                j--;
            }
        }
    }

    PrintDebug("tac+", "Class Methods of %s:", id->GetIdName());
    for (int i = 0; i < methods->NumElements(); i++) {
        PrintDebug("tac+", "%s", methods->Nth(i)->GetId()->GetIdName());
    }
    PrintDebug("tac+", "Class Vars of %s:", id->GetIdName());
    for (int i = 0; i < var_members->NumElements(); i++) {
        PrintDebug("tac+", "%s", var_members->Nth(i)->GetId()->GetIdName());
    }

    
    instance_size = var_members->NumElements() * 4 + 4;
    vtable_size = methods->NumElements() * 4;

    int var_offset = instance_size;
    for (int i = members->NumElements() - 1; i >= 0; i--) {
        Decl *d = members->Nth(i);
        if (d->IsVarDecl()) {
            var_offset -= 4;
            d->AssignMemberOffset(true, var_offset);
        } else if (d->IsFnDecl()) {
            
            for (int i = 0; i < methods->NumElements(); i++) {
                FnDecl *f1 = methods->Nth(i);
                if (!strcmp(f1->GetId()->GetIdName(), d->GetId()->GetIdName()))
                    d->AssignMemberOffset(true, i * 4);
            }
        }
    }
}

void ClassDecl::AddPrefixToMethods() {
    
    for (int i = 0; i < members->NumElements(); i++) {
        members->Nth(i)->AddPrefixToMethods();
    }
}

void ClassDecl::Emit() {
  int j = 0;
  for(int i = 0; i < 300; i++){
    j++;
  }
}

InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
}

void InterfaceDecl::PrintChildren(int indentLevel) {
  int j = 0;
  for(int i = 0; i < 300; i++){
    j++;
  }
}

void InterfaceDecl::BuildST() {
    if (ScopeM->LocalLookup(this->GetId())) {
        Decl *d = ScopeM->Lookup(this->GetId());
        ReportError::DeclConflict(this, d);
    } else {
        idx = ScopeM->InsertSymbol(this);
        id->SetDecl(this);
    }
    ScopeM->BuildScope(this->GetId()->GetIdName());
    members->CheckAll(E_BuildST);
    ScopeM->ExitScope();
}

void InterfaceDecl::Check(checkT c) {
    
    
    
    
    
    
    
    
    
    
    
    
    
}

void InterfaceDecl::Emit() {
    
    
    
}

FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
    vtable_ofst = -1;
}

void FnDecl::SetFunctionBody(Stmt *b) {
    (body=b)->SetParent(this);
}

void FnDecl::PrintChildren(int indentLevel) {
    if (expr_type) std::cout << " <" << expr_type << ">";
    if (emit_loc) emit_loc->Print();
    if (vtable_ofst != -1)
        std::cout << " ~~[VTable: " << vtable_ofst << "]";
    returnType->Print(indentLevel+1, "(return type) ");
    id->Print(indentLevel+1);
    if (id->GetDecl()) printf(" ........ {def}");
    formals->PrintAll(indentLevel+1, "(formals) ");
    if (body) body->Print(indentLevel+1, "(body) ");
}

void FnDecl::BuildST() {
    if (ScopeM->LocalLookup(this->GetId())) {
        Decl *d = ScopeM->Lookup(this->GetId());
        ReportError::DeclConflict(this, d);
    } else {
        idx = ScopeM->InsertSymbol(this);
        id->SetDecl(this);
    }
    ScopeM->BuildScope();
    formals->CheckAll(E_BuildST);
    if (body) body->Check(E_BuildST); 
    ScopeM->ExitScope();
}

void FnDecl::CheckDecl() {
    returnType->Check(E_CheckDecl);
    id->Check(E_CheckDecl);
    ScopeM->EnterScope();
    formals->CheckAll(E_CheckDecl);
    if (body) body->Check(E_CheckDecl);
    ScopeM->ExitScope();

    
    if (!strcmp(id->GetIdName(), "main")) {
        if (returnType != Type::voidType) {
            ReportError::Formatted(this->GetLocation(),
                    "Return value of 'main' function is expected to be void.");
        }
        if (formals->NumElements() != 0) {
            ReportError::NumArgsMismatch(id, 0, formals->NumElements());
        }
    }

    expr_type = returnType->GetType();
}

void FnDecl::Check(checkT c) {
    switch (c) {
        case E_BuildST:
            this->BuildST(); break;
        case E_CheckDecl:
            this->CheckDecl(); break;
        default:
            returnType->Check(c);
            id->Check(c);
            ScopeM->EnterScope();
            formals->CheckAll(c);
            if (body) body->Check(c);
            ScopeM->ExitScope();
    }
}

bool FnDecl::IsEquivalentTo(Decl *other) {
    Assert(this->GetType() && other->GetType());

    if (!other->IsFnDecl()) {
        return false;
    }
    FnDecl *fn = dynamic_cast<FnDecl*>(other);
    if (!returnType->IsEquivalentTo(fn->GetType())) {
        return false;
    }
    if (formals->NumElements() != fn->GetFormals()->NumElements()) {
        return false;
    }
    for (int i = 0; i < formals->NumElements(); i++) {
        
        Type *var_type1 =
            (dynamic_cast<VarDecl*>(formals->Nth(i)))->GetType();
        Type *var_type2 =
            (dynamic_cast<VarDecl*>(fn->GetFormals()->Nth(i)))->GetType();
        if (!var_type1->IsEquivalentTo(var_type2)) {
            return false;
        }
    }
    return true;
}

void FnDecl::AddPrefixToMethods() {
    
    
    Decl *d = dynamic_cast<Decl*>(this->GetParent());
    if (d && d->IsClassDecl()) {
        id->AddPrefix(".");
        id->AddPrefix(d->GetId()->GetIdName());
        id->AddPrefix("_");
    } else if (strcmp(id->GetIdName(), "main")) {
        id->AddPrefix("_");
    }
}

void FnDecl::AssignMemberOffset(bool inClass, int offset) {
    vtable_ofst = offset;
}

void FnDecl::Emit() {
    PrintDebug("tac+", "Begin Emitting TAC in FnDecl.");
    if (returnType == Type::doubleType) {
        ReportError::Formatted(this->GetLocation(),
                "Double type is not supported by compiler back end yet.");
        Assert(0);
    }

    Decl *d = dynamic_cast<Decl*>(this->GetParent());
    CG->GenLabel(id->GetIdName());

    
    BeginFunc *f = CG->GenBeginFunc();

    
    if (d && d->IsClassDecl()) {
        CG->GetNextParamLoc();
    }

    
    for (int i = 0; i < formals->NumElements(); i++) {
        VarDecl *v = formals->Nth(i);
        if (v->GetType() == Type::doubleType) {
            ReportError::Formatted(this->GetLocation(),
                    "Double type is not supported by compiler back end yet.");
            Assert(0);
        }
        Location *l = new Location(fpRelative, CG->GetNextParamLoc(),
                v->GetId()->GetIdName());
        v->SetEmitLoc(l);
    }

    if (body) body->Emit();

    
    f->SetFrameSize(CG->GetFrameSize());

    CG->GenEndFunc();
}

