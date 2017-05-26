

#ifndef _H_codegen
#define _H_codegen

#include <cstdlib>
#include <list>
#include "tac.h"


typedef enum { Alloc, ReadLine, ReadInteger, StringEqual,
               PrintInt, PrintString, PrintBool, Halt, NumBuiltIns } BuiltIn;

class CodeGenerator {
  private:
    std::list<Instruction*> code;
    int local_loc;
    int param_loc;
    int globl_loc;

  public:
    
    
    
    
    
    
    
    
    
    
    static const int OffsetToFirstLocal = -8,
                     OffsetToFirstParam = 4,
                     OffsetToFirstGlobal = 0;
    static const int VarSize = 4;

    
    int GetNextLocalLoc();
    int GetNextParamLoc();
    int GetNextGlobalLoc();
    int GetFrameSize();
    void ResetFrameSize();

    static Location* ThisPtr;

    CodeGenerator();

    
    
    char *NewLabel();

    
    
    Location *GenTempVar();

    
    
    
    
    
    
    
    
    
    Location *GenLoadConstant(int value);
    Location *GenLoadConstant(const char *str);
    Location *GenLoadLabel(const char *label);

    
    void GenAssign(Location *dst, Location *src);

    
    
    
    
    
    void GenStore(Location *addr, Location *val, int offset = 0);

    
    
    
    
    
    
    
    Location *GenLoad(Location *addr, int offset = 0);

    
    
    
    
    Location *GenBinaryOp(const char *opName, Location *op1, Location *op2);

    
    
    
    
    void GenPushParam(Location *param);

    
    
    
    void GenPopParams(int numBytesOfParams);

    
    
    
    
    
    
    Location *GenLCall(const char *label, bool fnHasReturnValue);

    
    
    
    
    
    Location *GenACall(Location *fnAddr, bool fnHasReturnValue);

    
    
    
    
    
    
    
    
    
    Location *GenBuiltInCall(BuiltIn b, Location *arg1 = NULL,
            Location *arg2 = NULL);

    
    
    
    
    
    void GenIfZ(Location *test, const char *label);
    void GenGoto(const char *label);
    void GenReturn(Location *val = NULL);
    void GenLabel(const char *label);

    
    
    BeginFunc *GenBeginFunc();
    void GenEndFunc();

    
    
    
    
    
    void GenVTable(const char *className, List<const char*> *methodLabels);

    
    
    
    
    
    
    void DoFinalCodeGen();
};

#endif

