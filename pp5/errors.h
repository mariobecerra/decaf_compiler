

#ifndef _H_errors
#define _H_errors

#include <string>
using std::string;
#include "location.h"

class Type;
class Identifier;
class Expr;
class BreakStmt;
class ReturnStmt;
class This;
class Decl;
class Operator;



typedef enum {LookingForType, LookingForClass, LookingForInterface,
    LookingForVariable, LookingForFunction} reasonT;

typedef enum {
    E_BuildST,
    E_CheckDecl,
    E_CheckInherit,
    E_CheckType
} checkT;

class ReportError
{
  public:

    
    static void UntermComment();
    static void InvalidDirective(int linenum);

    
    static void LongIdentifier(yyltype *loc, const char *ident);
    static void UntermString(yyltype *loc, const char *str);
    static void UnrecogChar(yyltype *loc, char ch);

    
    static void DeclConflict(Decl *newDecl, Decl *prevDecl);
    static void OverrideMismatch(Decl *fnDecl);
    static void InterfaceNotImplemented(Decl *classDecl, Type *intfType);

    
    static void IdentifierNotDeclared(Identifier *ident, reasonT whyNeeded);

    
    static void IncompatibleOperand(Operator *op, Type *rhs); 
    static void IncompatibleOperands
                  (Operator *op, Type *lhs, Type *rhs); 
    static void ThisOutsideClassScope(This *th);

    
    static void BracketsOnNonArray(Expr *baseExpr); 
    static void SubscriptNotInteger(Expr *subscriptExpr);
    static void NewArraySizeNotInteger(Expr *sizeExpr);

    
    static void NumArgsMismatch
                  (Identifier *fnIdentifier, int numExpected, int numGiven);
    static void ArgMismatch
                  (Expr *arg, int argIndex, Type *given, Type *expected);
    static void PrintArgMismatch(Expr *arg, int argIndex, Type *given);

    
    static void FieldNotFoundInBase(Identifier *field, Type *base);
    static void InaccessibleField(Identifier *field, Type *base);

    
    static void TestNotBoolean(Expr *testExpr);
    static void ReturnMismatch(ReturnStmt *rStmt, Type *given, Type *expected);
    static void BreakOutsideLoop(BreakStmt *bStmt);

    
    static void NoMainFound();

    
    static void Formatted(yyltype *loc, const char *format, ...);

    
    static int NumErrors() { return numErrors; }

  private:

    static void UnderlineErrorInLine(const char *line, yyltype *pos);
    static void OutputError(yyltype *loc, string msg);
    static int numErrors;

};


static const char *err_arr_out_of_bounds =
                    "Decaf runtime error: Array subscript out of bounds\\n";
static const char *err_arr_bad_size =
                    "Decaf runtime error: Array size is <= 0\\n";

#endif

