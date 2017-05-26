

#ifndef _H_mips
#define _H_mips

#include "tac.h"
#include "list.h"

class Location;

class Mips
{
  private:
    typedef enum {
        zero, at, v0, v1, a0, a1, a2, a3,
        s0, s1, s2, s3, s4, s5, s6, s7,
        t0, t1, t2, t3, t4, t5, t6, t7,
        t8, t9, k0, k1, gp, sp, fp, ra, NumRegs
    } Register;

    struct RegContents {
        bool isDirty;
        Location *var;
        const char *name;
        bool isGeneralPurpose;
    } regs[NumRegs];

    Register rs, rt, rd;

    typedef enum { ForRead, ForWrite } Reason;

    void FillRegister(Location *src, Register reg);
    void SpillRegister(Location *dst, Register reg);

    void EmitCallInstr(Location *dst, const char *fn, bool isL);

    static const char *mipsName[BinaryOp::NumOps];
    static const char *NameForTac(BinaryOp::OpCode code);

    Instruction* currentInstruction;

 public:
    Mips();

    static void Emit(const char *fmt, ...);

    void EmitLoadConstant(Location *dst, int val);
    void EmitLoadStringConstant(Location *dst, const char *str);
    void EmitLoadLabel(Location *dst, const char *label);

    void EmitLoad(Location *dst, Location *reference, int offset);
    void EmitStore(Location *reference, Location *value, int offset);
    void EmitCopy(Location *dst, Location *src);

    void EmitBinaryOp(BinaryOp::OpCode code, Location *dst,
            Location *op1, Location *op2);

    void EmitLabel(const char *label);
    void EmitGoto(const char *label);
    void EmitIfZ(Location *test, const char*label);
    void EmitReturn(Location *returnVal);

    void EmitBeginFunction(int frameSize);
    void EmitEndFunction();

    void EmitParam(Location *arg);
    void EmitLCall(Location *result, const char* label);
    void EmitACall(Location *result, Location *fnAddr);
    void EmitPopParams(int bytes);

    void EmitVTable(const char *label, List<const char*> *methodLabels);

    void EmitPreamble();

    class CurrentInstruction;
};


class Mips::CurrentInstruction
{
  public:
    CurrentInstruction(Mips& mips, Instruction* instr) : mips( mips ) {
        mips.currentInstruction= instr;
    }

    ~CurrentInstruction() {
        mips.currentInstruction= NULL;
    }

  private:
    Mips& mips;
};

#endif

