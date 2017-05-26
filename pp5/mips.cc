

#include <stdarg.h>
#include <cstring>
#include "mips.h"



static bool LocationsAreSame(Location *var1, Location *var2) {
    return (var1 == var2 ||
            (var1 && var2
             && !strcmp(var1->GetName(), var2->GetName())
             && var1->GetSegment()  == var2->GetSegment()
             && var1->GetOffset() == var2->GetOffset()));
}


void Mips::SpillRegister(Location *dst, Register reg) {
    Assert(dst);
    const char *offsetFromWhere = dst->GetSegment() == fpRelative
        ? regs[fp].name : regs[gp].name;
    Assert(dst->GetOffset() % 4 == 0); 
    Emit("sw %s, %d(%s)\t# spill %s from %s to %s%+d", regs[reg].name,
            dst->GetOffset(), offsetFromWhere, dst->GetName(), regs[reg].name,
            offsetFromWhere,dst->GetOffset());
}


void Mips::FillRegister(Location *src, Register reg) {
    Assert(src);
    const char *offsetFromWhere = src->GetSegment() == fpRelative
        ? regs[fp].name : regs[gp].name;
    Assert(src->GetOffset() % 4 == 0); 
    Emit("lw %s, %d(%s)\t# fill %s to %s from %s%+d", regs[reg].name,
            src->GetOffset(), offsetFromWhere, src->GetName(), regs[reg].name,
            offsetFromWhere,src->GetOffset());
}


void Mips::Emit(const char *fmt, ...) {
    va_list args;
    char buf[1024];

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    if (buf[strlen(buf) - 1] != ':') printf("\t"); 
    if (buf[0] != '#') printf("  ");   
    printf("%s", buf);
    if (buf[strlen(buf)-1] != '\n') printf("\n"); 
}


void Mips::EmitLoadConstant(Location *dst, int val) {
    Register r = rd;
    Emit("li %s, %d\t\t# load constant value %d into %s", regs[r].name,
            val, val, regs[r].name);
    SpillRegister(dst, rd);
}


void Mips::EmitLoadStringConstant(Location *dst, const char *str) {
    static int strNum = 1;
    char label[16];
    sprintf(label, "_string%d", strNum++);
    Emit(".data\t\t\t# create string constant marked with label");
    Emit("%s: .asciiz %s", label, str);
    Emit(".text");
    EmitLoadLabel(dst, label);
}


void Mips::EmitLoadLabel(Location *dst, const char *label) {
    Emit("la %s, %s\t# load label", regs[rd].name, label);
    SpillRegister(dst, rd);
}


void Mips::EmitCopy(Location *dst, Location *src) {
    FillRegister(src, rd);
    SpillRegister(dst, rd);
}


void Mips::EmitLoad(Location *dst, Location *reference, int offset) {
    FillRegister(reference, rs);
    Emit("lw %s, %d(%s) \t# load with offset", regs[rd].name,
            offset, regs[rs].name);
    SpillRegister(dst, rd);
}


void Mips::EmitStore(Location *reference, Location *value, int offset) {
    FillRegister(value, rs);
    FillRegister(reference, rd);
    Emit("sw %s, %d(%s) \t# store with offset",
            regs[rs].name, offset, regs[rd].name);
}


void Mips::EmitBinaryOp(BinaryOp::OpCode code, Location *dst,
        Location *op1, Location *op2)
{
    FillRegister(op1, rs);
    FillRegister(op2, rt);
    Emit("%s %s, %s, %s\t", NameForTac(code), regs[rd].name,
            regs[rs].name, regs[rt].name);
    SpillRegister(dst, rd);
}


void Mips::EmitLabel(const char *label) {
    Emit("%s:", label);
}


void Mips::EmitGoto(const char *label) {
    Emit("b %s\t\t# unconditional branch", label);
}


void Mips::EmitIfZ(Location *test, const char *label) {
    FillRegister(test, rs);
    Emit("beqz %s, %s\t# branch if %s is zero ", regs[rs].name, label,
            test->GetName());
}


void Mips::EmitParam(Location *arg) {
    Emit("subu $sp, $sp, 4\t# decrement sp to make space for param");
    FillRegister(arg, rs);
    Emit("sw %s, 4($sp)\t# copy param value to stack", regs[rs].name);
}


void Mips::EmitCallInstr(Location *result, const char *fn, bool isLabel) {
    Emit("%s %-15s\t# jump to function", isLabel? "jal": "jalr", fn);
    if (result != NULL) {
        Emit("move %s, %s\t\t# copy function return value from $v0",
                regs[rd].name, regs[v0].name);
        SpillRegister(result, rd);
    }
}


void Mips::EmitLCall(Location *dst, const char *label) {
    EmitCallInstr(dst, label, true);
}

void Mips::EmitACall(Location *dst, Location *fn) {
    FillRegister(fn, rs);
    EmitCallInstr(dst, regs[rs].name, false);
}


void Mips::EmitPopParams(int bytes) {
    if (bytes != 0)
        Emit("add $sp, $sp, %d\t# pop params off stack", bytes);
}


void Mips::EmitReturn(Location *returnVal) {
    if (returnVal != NULL) {
        FillRegister(returnVal, rd);
        Emit("move $v0, %s\t\t# assign return value into $v0",
                regs[rd].name);
    }
    Emit("move $sp, $fp\t\t# pop callee frame off stack");
    Emit("lw $ra, -4($fp)\t# restore saved ra");
    Emit("lw $fp, 0($fp)\t# restore saved fp");
    Emit("jr $ra\t\t# return from function");
}


void Mips::EmitBeginFunction(int stackFrameSize) {
    Assert(stackFrameSize >= 0);
    Emit("subu $sp, $sp, 8\t# decrement sp to make space to save ra, fp");
    Emit("sw $fp, 8($sp)\t# save fp");
    Emit("sw $ra, 4($sp)\t# save ra");
    Emit("addiu $fp, $sp, 8\t# set up new fp");

    if (stackFrameSize != 0)
        Emit(
            "subu $sp, $sp, %d\t# decrement sp to make space for locals/temps",
            stackFrameSize);
}



void Mips::EmitEndFunction() {
    Emit("# (below handles reaching end of fn body with no explicit return)");
    EmitReturn(NULL);
}




void Mips::EmitVTable(const char *label, List<const char*> *methodLabels) {
    Emit(".data");
    Emit(".align 2");
    Emit("%s:\t\t# label for class %s vtable", label, label);
    for (int i = 0; i < methodLabels->NumElements(); i++)
        Emit(".word %s\n", methodLabels->Nth(i));
    Emit(".text");
}


void Mips::EmitPreamble() {
    Emit("# standard Decaf preamble ");
    Emit(".text");
    Emit(".align 2");
    Emit(".globl main");
}


const char *Mips::NameForTac(BinaryOp::OpCode code) {
    Assert(code >=0 && code < BinaryOp::NumOps);
    const char *name = mipsName[code];
    Assert(name != NULL);
    return name;
}


Mips::Mips() {
    mipsName[BinaryOp::Add] = "add";
    mipsName[BinaryOp::Sub] = "sub";
    mipsName[BinaryOp::Mul] = "mul";
    mipsName[BinaryOp::Div] = "div";
    mipsName[BinaryOp::Mod] = "rem";
    mipsName[BinaryOp::Eq] = "seq";
    mipsName[BinaryOp::Ne] = "sne";
    mipsName[BinaryOp::Lt] = "slt";
    mipsName[BinaryOp::Le] = "sle";
    mipsName[BinaryOp::Gt] = "sgt";
    mipsName[BinaryOp::Ge] = "sge";
    mipsName[BinaryOp::And] = "and";
    mipsName[BinaryOp::Or] = "or";
    regs[zero] = (RegContents){false, NULL, "$zero", false};
    regs[at] = (RegContents){false, NULL, "$at", false};
    regs[v0] = (RegContents){false, NULL, "$v0", false};
    regs[v1] = (RegContents){false, NULL, "$v1", false};
    regs[a0] = (RegContents){false, NULL, "$a0", false};
    regs[a1] = (RegContents){false, NULL, "$a1", false};
    regs[a2] = (RegContents){false, NULL, "$a2", false};
    regs[a3] = (RegContents){false, NULL, "$a3", false};
    regs[k0] = (RegContents){false, NULL, "$k0", false};
    regs[k1] = (RegContents){false, NULL, "$k1", false};
    regs[gp] = (RegContents){false, NULL, "$gp", false};
    regs[sp] = (RegContents){false, NULL, "$sp", false};
    regs[fp] = (RegContents){false, NULL, "$fp", false};
    regs[ra] = (RegContents){false, NULL, "$ra", false};
    regs[t0] = (RegContents){false, NULL, "$t0", true};
    regs[t1] = (RegContents){false, NULL, "$t1", true};
    regs[t2] = (RegContents){false, NULL, "$t2", true};
    regs[t3] = (RegContents){false, NULL, "$t3", true};
    regs[t4] = (RegContents){false, NULL, "$t4", true};
    regs[t5] = (RegContents){false, NULL, "$t5", true};
    regs[t6] = (RegContents){false, NULL, "$t6", true};
    regs[t7] = (RegContents){false, NULL, "$t7", true};
    regs[t8] = (RegContents){false, NULL, "$t8", true};
    regs[t9] = (RegContents){false, NULL, "$t9", true};
    regs[s0] = (RegContents){false, NULL, "$s0", true};
    regs[s1] = (RegContents){false, NULL, "$s1", true};
    regs[s2] = (RegContents){false, NULL, "$s2", true};
    regs[s3] = (RegContents){false, NULL, "$s3", true};
    regs[s4] = (RegContents){false, NULL, "$s4", true};
    regs[s5] = (RegContents){false, NULL, "$s5", true};
    regs[s6] = (RegContents){false, NULL, "$s6", true};
    regs[s7] = (RegContents){false, NULL, "$s7", true};
    rs = t0; rt = t1; rd = t2;
}

const char *Mips::mipsName[BinaryOp::NumOps];

