

#ifndef _H_tac
#define _H_tac

#include "list.h" 

class Mips;









typedef enum {fpRelative, gpRelative} Segment;

class Location
{
  protected:
    const char *variableName;
    Segment segment;
    int offset;
    Location* base;

  public:
    Location(Segment seg, int offset, const char *name);
    Location(Segment seg, int offset, const char *name, Location *base);

    const char *GetName() const     { return variableName; }
    Segment GetSegment() const      { return segment; }
    int GetOffset() const           { return offset; }
    Location* GetBase() const       { return base; }

    void Print();
};




class Instruction {
  protected:
    char printed[128];

  public:
    virtual void Print();
    virtual void EmitSpecific(Mips *mips) = 0;
    void Emit(Mips *mips);
};




class LoadConstant;
class LoadStringConstant;
class LoadLabel;
class Assign;
class Load;
class Store;
class BinaryOp;
class Label;
class Goto;
class IfZ;
class BeginFunc;
class EndFunc;
class Return;
class PushParam;
class PopParams;
class LCall;
class ACall;
class VTable;

class LoadConstant: public Instruction
{
    Location *dst;
    int val;
  public:
    LoadConstant(Location *dst, int val);
    void EmitSpecific(Mips *mips);
};

class LoadStringConstant: public Instruction
{
    Location *dst;
    char *str;
  public:
    LoadStringConstant(Location *dst, const char *s);
    void EmitSpecific(Mips *mips);
};

class LoadLabel: public Instruction
{
    Location *dst;
    const char *label;
  public:
    LoadLabel(Location *dst, const char *label);
    void EmitSpecific(Mips *mips);
};

class Assign: public Instruction
{
    Location *dst, *src;
  public:
    Assign(Location *dst, Location *src);
    void EmitSpecific(Mips *mips);
};

class Load: public Instruction
{
    Location *dst, *src;
    int offset;
  public:
    Load(Location *dst, Location *src, int offset = 0);
    void EmitSpecific(Mips *mips);
};

class Store: public Instruction
{
    Location *dst, *src;
    int offset;
  public:
    Store(Location *d, Location *s, int offset = 0);
    void EmitSpecific(Mips *mips);
};

class BinaryOp: public Instruction
{
  public:
    typedef enum {
        Add, Sub, Mul, Div, Mod,
        Eq, Ne, Lt, Le, Gt, Ge,
        And, Or,
        NumOps
    } OpCode;
    static const char * const opName[NumOps];
    static OpCode OpCodeForName(const char *name);

  protected:
    OpCode code;
    Location *dst, *op1, *op2;
  public:
    BinaryOp(OpCode c, Location *dst, Location *op1, Location *op2);
    void EmitSpecific(Mips *mips);
};

class Label: public Instruction
{
    const char *label;
  public:
    Label(const char *label);
    void Print();
    void EmitSpecific(Mips *mips);
    const char* text() const { return label; }
};

class Goto: public Instruction
{
    const char *label;
  public:
    Goto(const char *label);
    void EmitSpecific(Mips *mips);
    const char* branch_label() const { return label; }
};

class IfZ: public Instruction
{
    Location *test;
    const char *label;
  public:
    IfZ(Location *test, const char *label);
    void EmitSpecific(Mips *mips);
    const char* branch_label() const { return label; }
};

class BeginFunc: public Instruction
{
    int frameSize;
  public:
    BeginFunc();
    
    void SetFrameSize(int numBytesForAllLocalsAndTemps);
    void EmitSpecific(Mips *mips);
};

class EndFunc: public Instruction
{
  public:
    EndFunc();
    void EmitSpecific(Mips *mips);
};

class Return: public Instruction
{
    Location *val;
  public:
    Return(Location *val);
    void EmitSpecific(Mips *mips);
};

class PushParam: public Instruction
{
    Location *param;
  public:
    PushParam(Location *param);
    void EmitSpecific(Mips *mips);
};

class PopParams: public Instruction
{
    int numBytes;
  public:
    PopParams(int numBytesOfParamsToRemove);
    void EmitSpecific(Mips *mips);
};

class LCall: public Instruction
{
    const char *label;
    Location *dst;
  public:
    LCall(const char *labe, Location *result);
    void EmitSpecific(Mips *mips);
};

class ACall: public Instruction
{
    Location *dst, *methodAddr;
  public:
    ACall(Location *meth, Location *result);
    void EmitSpecific(Mips *mips);
};

class VTable: public Instruction
{
    List<const char *> *methodLabels;
    const char *label;
 public:
    VTable(const char *labelForTable, List<const char *> *methodLabels);
    void Print();
    void EmitSpecific(Mips *mips);
};

#endif

