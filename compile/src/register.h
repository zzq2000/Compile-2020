#ifndef REG
#define REG
#include "Intermediate.h"
typedef struct {
	int id;			//ͨ��id���üĴ���������
	char* name;		//���洢�ı���/��ʱ����������
	int count;		//ʹ�ô�������
	int dirty;		//��λ
	int valid;		//����ʹ�õļĴ�������Ҫ������ʱ�Ĵ����ϣ���������������ʱ�����ú���������ʹ�õ���ʱ�Ĵ���ѹջ
	//int addr;		//��¼�Ĵ����洢�ı����ĵ�ַƫ�ƣ��������λΪ1ʱ�������и�д�����д�Ĵ�����ֵ
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

