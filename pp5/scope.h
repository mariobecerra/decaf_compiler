

#ifndef _H_ScopeM
#define _H_ScopeM

#include <list>
#include <vector>
#include "hashtable.h"



class Decl;
class Identifier;
class Scope;

class scopeST
{
  protected:
    std::vector<Scope *> *scopes;
    std::vector<int> *activeScopes;
    int cur_scope;  
    int scope_cnt;  
    int id_cnt;

  public:
    scopeST();

    
    void BuildScope();
    
    void BuildScope(const char *key);
    
    void EnterScope();
    
    Decl *Lookup(Identifier *id);
    
    Decl *LookupParent(Identifier *id);
    
    Decl *LookupInterface(Identifier *id);
    
    Decl *LookupField(Identifier *base, Identifier *field);
    
    Decl *LookupThis();
    
    int InsertSymbol(Decl *decl);
    
    bool LocalLookup(Identifier *id);
    
    void ExitScope();

    
    void SetScopeParent(const char *key);
    
    void SetInterface(const char *key);

    
    void ReEnter();

    
    void Print();

  protected:
    int FindScopeFromOwnerName(const char *owner);

};

extern scopeST *ScopeM;

#endif

