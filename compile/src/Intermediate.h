#ifndef INTER
#define INTER
#include "idTable.h"

typedef enum {
	add,
	sub,
	mult,
	divi,
	read,
	print,
	move,
	enter,
	array_read,
	array_write,
	bge,
	bgt,
	ble,
	blt,
	setlab,
	beq,
	bne,
	j,
	ret,
	retv,
	call,
	callv,
	parameter,
	alloc,	//在函数定义变量完成之后，更新栈顶的位置
	end,
	parabegin
} OP;
typedef struct {
	char* name;
	Category category;
	Type type;
	int value;
	int addr;
	int level;
} Operand;

typedef struct {
	OP op;
	Operand rd;
	Operand rs;
	Operand rt;
	int opNum;
} IC;

typedef struct {
	char* name;
	int num;
	IC lists[1024];
} ICFUNC;

extern IC IClists[4096];
extern int ICnum;
extern ICFUNC funclists[256];
extern int funcnum;
Operand* newOperand(IdTable);
Operand* newOperand(IdTable, int);
Operand* newOperand(int, Type);
Operand* tempOperand();
Operand* getLab();
void FunctionDefBegin(char*);
void FunctionDefEnd();
void clearTemp();
void ICAdd(Operand, Operand, Operand);
void ICSub(Operand, Operand, Operand);
void ICMult(Operand, Operand, Operand);
void ICDiv(Operand, Operand, Operand);
void ICRead(Operand);
void ICPrint(Operand);
void ICMove(Operand, Operand);
void ICenter();
void ICArrayRead(Operand, Operand, Operand);
void ICArrayWrite(Operand, Operand, Operand);
void ICBge(Operand, Operand, Operand);
void ICBgt(Operand, Operand, Operand);
void ICBle(Operand, Operand, Operand);
void ICBlt(Operand, Operand, Operand);
void ICSetLab(Operand);
void ICBeq(Operand, Operand, Operand);
void ICBne(Operand, Operand, Operand);
void ICJ(Operand);
void ICRET();
void ICRETV(Operand);
void ICCall(Operand);
void ICCallV(Operand, Operand);
void ICPara(Operand);
void ICAlloc(Operand);
void ICEnd();
void ICParaBegin();
void printIC();
#endif // !INTER

