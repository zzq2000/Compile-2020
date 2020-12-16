#ifndef REG
#define REG
#include "Intermediate.h"
typedef struct {
	int id;			//通过id调用寄存器的名字
	char* name;		//所存储的变量/临时变量的名字
	int count;		//使用次数计数
	int dirty;		//脏位
	int valid;		//正在使用的寄存器，主要用在临时寄存器上，当发生函数调用时，调用函数将正在使用的临时寄存器压栈
	//int addr;		//记录寄存器存储的变量的地址偏移，如果在脏位为1时发生进行覆写，则回写寄存器的值
	//int level;		
	Operand op;
} Reg;
extern Reg tempReg[10];
extern int tempRegNum;
extern char tempRegName[10][4];
extern Reg globalReg[8];
extern int globalRegNum;
extern char globalRegName[8][4];
extern Reg paraReg[4];
extern int paraRegNum;
extern char paraRegName[4][4];
int searchReg(Operand);
int setReg(Operand);
int setReg();
int setParaReg(Operand);
//int searchParaReg(Operand);
void clearTempReg();
void clearTempReg(int);
void clearGlobalReg();
void clearGlobalReg(int);
void clearParaReg();
void setNotValid(int);
void changed(Operand);
#endif // !REG

