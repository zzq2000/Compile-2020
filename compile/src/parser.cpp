#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include "parser.h"
#include "symTables.h"
#include "error.h"
#include "idTable.h"
#include "str.h"
#include "Intermediate.h"
FILE* inFile;
//语法树的树根
gtNodePointer grammerTree = NULL;
//词法分析得到的下一个符号
SymTable symTable;

//因为实验中的文法存在回溯问题，因此需要提取公因式来消除回溯
//将公因式存放在symTableArray数组中，并在恰当的时刻放回到语法树中
SymTable symTableArray[20];
int symCount = 0;



int infunc = 0;			//记录当前是否正在定义一个函数，当读到空的返回符号时，如果在进行函数定义，则生成函数返回的中间代码，否则生成main函数结束的中间代码
Type returnType = Void;	//记录函数的返回值类型，进行函数返回类的错误处理
int isReturned = 0;		//记录当前函数是否已经发生返回，如果有返回值的函数到结尾时仍然无法返回，则报错

//得到下一个符号
//使用ungetsym标记来实现回退功能
//实际上没有进行回退，而是通过标记来取消下一次的取词
int ungetsym = 0;
void getNextSymTable() {
	if (ungetsym) {
		ungetsym = 0;
	}
	else {
		symTable = getsym_(inFile);
	}
}

//检查字符型常量和字符串是否满足要求，进行错误处理
void symCheck(SymTable symTable) {
	if (symTable.symbol == CHARCON) {
		if (symTable.value == NULL || (symTable.value != NULL &&
			symTable.value[0] != '+' && symTable.value[0] != '-' && symTable.value[0] != '*' &&
			symTable.value[0] != '/' && symTable.value[0] != '_' && !isalnum(symTable.value[0])))
			errorPro(line, 'a');
	}
	else if (symTable.symbol == STRCON) {
		if (symTable.value == NULL) {
			errorPro(line, 'a');
		}
		else {
			for (int i = 0; i < strlen(symTable.value); i++) {
				if (symTable.value[i] < 32 || symTable.value[i] == 34 || symTable.value[i] > 126) {
					errorPro(line, 'a');
					break;
				}
			}
		}
	}
}

typedef struct {
	char name[1024];
	int void_;
}function; 
function functions[256];
int functionNum = 0;

char vnList[][40] = {
	"<程序>",
	"<常量说明>",
	"<常量定义>",
	"<整数>",
	"<无符号整数>",
	"<变量说明>",
	"<变量定义>",
	"<变量定义及初始化>",
	"<变量定义无初始化>",
	"<常量>",
	"<有返回值函数定义>",
	"<无返回值函数定义>",
	"<声明头部>",
	"<参数表>",
	"<主函数>",
	"<复合语句>",
	"<语句列>",
	"<语句>",
	"<循环语句>",
	"<步长>",
	"<条件语句>",
	"<条件>",
	"<有返回值函数调用语句>",
	"<无返回值函数调用语句>",
	"<值参数表>",
	"<赋值语句>",
	"<读语句>",
	"<写语句>",
	"<字符串>",
	"<情况语句>",
	"<情况表>",
	"<情况子语句>",
	"<缺省>",
	"<返回语句>",
	"<表达式>",
	"<项>",
	"<因子>"
};
gtNodePointer createTreeNode(SymTable symTable) {
	gtNodePointer gtNodep = (gtNodePointer)malloc(sizeof(gtNode));
	if (gtNodep != NULL) {
		gtNodep->symTable = (SymTable*)malloc(sizeof(symTable));
		if(gtNodep->symTable != NULL) *(gtNodep->symTable) = symTable;
		for (int i = 0; i < 100; i++) {
			gtNodep->sonNode[i] = NULL;
		}
	}
	return gtNodep;
}

gtNodePointer createTreeNode(Vn vn) {
	gtNodePointer gtNodep = (gtNodePointer)malloc(sizeof(gtNode));
	if (gtNodep != NULL) {
		gtNodep->vn = vn;
		gtNodep->symTable = NULL;
		for (int i = 0; i < 100; i++) {
			gtNodep->sonNode[i] = NULL;
		}
	}
	return gtNodep;
}

bool insert(gtNodePointer gtNodep, gtNodePointer son) {
	if (gtNodep != NULL) {
		int i = 0;
		for (i = 0; gtNodep->sonNode[i] != NULL && i < 100; i++);
		if (i == 100) return false;
		gtNodep->sonNode[i] = son;
		return true;
	}
	return false;
}

void gt2file(FILE* outFile, gtNodePointer gtNodep) {
	if (gtNodep == NULL) return;
	for (int i = 0; gtNodep->sonNode[i] != NULL; i++) {
		gt2file(outFile, gtNodep->sonNode[i]);
	}
	if (gtNodep->symTable != NULL) {
		fprintf(outFile, "%s %s\n", sym[gtNodep->symTable->symbol], gtNodep->symTable->value);
	}
	else {
		fprintf(outFile, "%s\n", vnList[gtNodep->vn]);
	}
}

void program(gtNodePointer* gtNodepp) {
	if (symTable.symbol == CONSTTK) {
		gtNodePointer gtNodepSon = createTreeNode(CONSTSTATE);
		conststate(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
		symTableArray[symCount++] = symTable;
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			symTableArray[symCount++] = symTable;
			getNextSymTable();
			if (symTable.symbol == ASSIGN || symTable.symbol == SEMICN || symTable.symbol == COMMA || symTable.symbol == LBRACK) {
				gtNodePointer gtNodepSon = createTreeNode(VARSTATE);
				varstate(&gtNodepSon);
				insert(*gtNodepp, gtNodepSon);
				getNextSymTable();
			}
			if (symTable.symbol == LPARENT) {
				gtNodePointer gtNodepSon = createTreeNode(FUNCTIONDEF);
				functiondef(&gtNodepSon);
				insert(*gtNodepp, gtNodepSon);
				getNextSymTable();
				while (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
					gtNodePointer gtNodepSon = createTreeNode(FUNCTIONDEF);
					functiondef(&gtNodepSon);
					insert(*gtNodepp, gtNodepSon);
					getNextSymTable();
				}
			}
			else if (symTable.symbol != VOIDTK) {
				err(line, symTable.value);
			}
		}
	}
	while (symTable.symbol == VOIDTK) {
		symTableArray[symCount++] = symTable;
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			gtNodePointer gtNodepSon = createTreeNode(VOIDFUNCTIONDEF);
			voidfunctiondef(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			while (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
				gtNodePointer gtNodepSon = createTreeNode(FUNCTIONDEF);
				functiondef(&gtNodepSon);
				insert(*gtNodepp, gtNodepSon);
				getNextSymTable();
			}
		} 
		else if (symTable.symbol == MAINTK) {
			gtNodePointer gtNodepSon = createTreeNode(MAINFUNCTION);
			mainfunction(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			break;
		}
		else {
			err(line, symTable.value);
		}
	}
	if (symTable.symbol != nullsym) err(line, symTable.value);
}

void conststate(gtNodePointer* gtNodepp) {
	do {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(CONSTDEF);
		if (symTable.symbol == INTTK) {
			intconstdef(&gtNodepSon);
		}
		else if (symTable.symbol == CHARTK) {
			charconstdef(&gtNodepSon);
		}
		else
		{
			err(line, symTable.value);
		}
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != SEMICN) {
			if (lineend == 1) {
				errorPro(line-1, 'k');
			}
			else {
				errorPro(line, 'k');
			}
			err(line, symTable.value);
			ungetsym = 1;
		}
		else {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		getNextSymTable();
	} while (symTable.symbol == CONSTTK);
	ungetsym = 1;
}

void intconstdef(gtNodePointer* gtNodepp) {
	Category category = con;
	Type type = Integer;
	do {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != IDENFR) {
			err(line, symTable.value);
		}
		char* name = symTable.value;
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != ASSIGN) {
			err(line, symTable.value);
		}
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		int value;
		if (symTable.symbol == INTCON || symTable.symbol == PLUS || symTable.symbol == MINU) {
			gtNodePointer gtNodepSon = createTreeNode(INTEGER);
			Operand T = integerA(&gtNodepSon);
			value = T.value;
			insert(*gtNodepp, gtNodepSon);
			insertId(name, category, type, 0, value);
		}
		else {
			err(line, symTable.value);
		}
		getNextSymTable();
	} while (symTable.symbol == COMMA);
	ungetsym = 1;
}

void charconstdef(gtNodePointer* gtNodepp) {
	Category category = con;
	Type type = Char;
	do {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != IDENFR) {
			err(line, symTable.value);
		}
		char* name = symTable.value;
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != ASSIGN) {
			err(line, symTable.value);
		}
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != CHARCON) {
			err(line, symTable.value);
		}
		symCheck(symTable);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		insertId(name, category, type, 0, symTable.value[0]);
		getNextSymTable();
	} while (symTable.symbol == COMMA);
	ungetsym = 1;
}

//void integer(gtNodePointer* gtNodepp) {
//	if (symTable.symbol == PLUS || symTable.symbol == MINU) {
//		gtNodePointer gtNodepSon = createTreeNode(symTable);
//		insert(*gtNodepp, gtNodepSon);
//		getNextSymTable();
//	}
//	if (symTable.symbol == INTCON) {
//		gtNodePointer gtNodepSon = createTreeNode(UNSIGNEDINTEGER);
//		unsignedinteger(&gtNodepSon);
//		insert(*gtNodepp, gtNodepSon);
//	}
//	else {
//		err(line, symTable.value);
//	}
//}
//
//void unsignedinteger(gtNodePointer* gtNodepp) {
//	gtNodePointer gtNodepSon = createTreeNode(symTable);
//	insert(*gtNodepp, gtNodepSon);
//}

Operand unsignedintegerA(gtNodePointer* gtNodepp) {
	int value = atoi(symTable.value);
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	return *newOperand(value, Integer);
}

Operand integerA(gtNodePointer* gtNodepp) {
	int value = 0, sym = 0;
	if (symTable.symbol == PLUS || symTable.symbol == MINU) {
		if (symTable.symbol == MINU) sym = 1;
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	if (symTable.symbol == INTCON) {
		value = atoi(symTable.value);
		gtNodePointer gtNodepSon = createTreeNode(UNSIGNEDINTEGER);
		unsignedintegerA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line, symTable.value);
	}
	return *newOperand(sym ? -value : value, Integer);
}

void varstate(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(VARDEF);
	vardef(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	if (symTable.symbol == SEMICN) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		if (lineend == 1) {
			errorPro(line - 1, 'k');
		}
		else {
			errorPro(line, 'k');
		}
		err(line, symTable.value);
		ungetsym = 1;
	}
	getNextSymTable();
	while (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
		symTableArray[symCount++] = symTable;
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			symTableArray[symCount++] = symTable;
			getNextSymTable();
			if (symTable.symbol == ASSIGN || symTable.symbol == SEMICN || symTable.symbol == COMMA || symTable.symbol == LBRACK) {
				gtNodePointer gtNodepSon = createTreeNode(VARDEF);
				vardef(&gtNodepSon);
				insert(*gtNodepp, gtNodepSon);
				if (symTable.symbol == SEMICN) {
					gtNodepSon = createTreeNode(symTable);
					insert(*gtNodepp, gtNodepSon);
				}
				else {
					if (lineend == 1) {
						errorPro(line - 1, 'k');
					}
					else {
						errorPro(line, 'k');
					}
					err(line, symTable.value);
					ungetsym = 1;
				}
				getNextSymTable();
			}
			else if (symTable.symbol == LPARENT) break;
			else {
				err(line, symTable.value);
			}
		}
		else {
			err(line, symTable.value);
		}
	}
	ungetsym = 1;
}

void arrayLength() {
	symTableArray[symCount++] = symTable;
	getNextSymTable();
	if (symTable.symbol == INTCON) {
		symTableArray[symCount++] = symTable;
	}
	else {
		errorPro(line, 'i');
		err(line, symTable.value);
	}
	getNextSymTable();
	if (symTable.symbol == RBRACK) {
		symTableArray[symCount++] = symTable;
	}
	else {
		symTableArray[symCount++] = symTable;//TODO
		errorPro(line, 'm');
		err(line, symTable.value);
		ungetsym = 1;
	}
}

void vardef(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon;
	if (symCount == 0) {
		symTableArray[symCount++] = symTable;
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			symTableArray[symCount++] = symTable;
		}
		else {
			err(line, symTable.value);
		}
		getNextSymTable();
	}
	while (symTable.symbol == LBRACK) {
		arrayLength();
		getNextSymTable();
	}
	if (symTable.symbol == SEMICN || symTable.symbol == COMMA) {
		gtNodepSon = createTreeNode(VARDEFNOINIT);
		vardefnoinit(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == ASSIGN) {
		gtNodepSon = createTreeNode(VARDEFINIT);
		vardefinit(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else err(line, symTable.value);
}

void vardefnoinit(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon;
	Category category = symCount == 2 ? var : ::array;
	Type type = symTableArray[0].symbol == INTTK ? Integer : Char;
	int dimention = symCount == 5 ? 1 : 
					symCount == 8 ? 2 : 0;
	if (dimention == 0) {
		insertId(symTableArray[1].value, category, type, dimention);
		for (int i = 0; i < symCount; i++) {
			gtNodepSon = createTreeNode(symTableArray[i]);
			insert(*gtNodepp, gtNodepSon);
		}
	}
	else {
		int length[10];
		int dim = 0;
		for (int i = 0; i < symCount; i++) {
			if (symTableArray[i].symbol != INTCON) {
				gtNodepSon = createTreeNode(symTableArray[i]);
				insert(*gtNodepp, gtNodepSon);
			}
			else {
				gtNodepSon = createTreeNode(UNSIGNEDINTEGER);
				gtNodePointer gtNodepSon2 = createTreeNode(symTableArray[i]);
				insert(gtNodepSon, gtNodepSon2);
				insert(*gtNodepp, gtNodepSon);
				length[dim++] = atoi(symTableArray[i].value);
			}
		}
		insertId(symTableArray[1].value, category, type, dimention, length);
	}
	symCount = 0;
	while (symTable.symbol == COMMA) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			symTableArray[symCount++] = symTable;
			getNextSymTable();
			while (symTable.symbol == LBRACK) {
				arrayLength();
				getNextSymTable();
			}
			Category category = symCount == 1 ? var : ::array;
			int dimention = symCount == 4 ? 1 :
				symCount == 7 ? 2 : 0;
			if (dimention == 0) {
				insertId(symTableArray[0].value, category, type, dimention);
				for (int i = 0; i < symCount; i++) {
					gtNodepSon = createTreeNode(symTableArray[i]);
					insert(*gtNodepp, gtNodepSon);
				}
			}
			else {
				int length[10];
				int dim = 0;
				for (int i = 0; i < symCount; i++) {
					if (symTableArray[i].symbol != INTCON) {
						gtNodepSon = createTreeNode(symTableArray[i]);
						insert(*gtNodepp, gtNodepSon);
					}
					else {
						gtNodepSon = createTreeNode(UNSIGNEDINTEGER);
						gtNodePointer gtNodepSon2 = createTreeNode(symTableArray[i]);
						insert(gtNodepSon, gtNodepSon2);
						insert(*gtNodepp, gtNodepSon);
						length[dim++] = atoi(symTableArray[i].value);
					}
				}
				insertId(symTableArray[0].value, category, type, dimention, length);
			}
			symCount = 0;
			if (symTable.symbol == SEMICN) return;
			else if (symTable.symbol == COMMA) continue;
			else {
				err(line, symTable.value);
			}
		}
		else {
			err(line, symTable.value);
		}
	}
}

void arrayInit(gtNodePointer* gtNodepp, int dim, int length[], int now, Type type, IdTable idtable, int* count) {
	if (dim < now) {
		errorPro(line, 'n');
	}
	int num = 0;
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == LBRACE) {
		arrayInit(gtNodepp, dim, length, now+1, type, idtable, count);
		getNextSymTable();
		num++;
		while (symTable.symbol == COMMA) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			arrayInit(gtNodepp, dim, length, now + 1, type, idtable, count);
			num++;
			getNextSymTable();
		}
		if (num != length[now-1]) errorPro(line, 'n');
		if (symTable.symbol != RBRACE) err(line, symTable.value);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == INTCON || symTable.symbol == CHARCON || symTable.symbol == PLUS || symTable.symbol == MINU) {
		gtNodepSon = createTreeNode(CONST);
		Operand rs = constA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		if ((symTable.symbol != CHARCON && type == Char) || (symTable.symbol == CHARCON && type == Integer)) {
			errorPro(line, 'o');
		}
		num++;
		getNextSymTable();
		Operand rd = *newOperand(idtable, *count);
		*count = *count + 1;
		ICMove(rd, rs);
		while (symTable.symbol == COMMA) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			if (symTable.symbol == INTCON || symTable.symbol == CHARCON || symTable.symbol == PLUS || symTable.symbol == MINU) {
				gtNodepSon = createTreeNode(CONST);
				Operand rs = constA(&gtNodepSon);
				insert(*gtNodepp, gtNodepSon);
				if ((symTable.symbol != CHARCON && type == Char) || (symTable.symbol == CHARCON && type == Integer)) {
					errorPro(line, 'o');
				}
				Operand rd = *newOperand(idtable, *count);
				*count = *count + 1;
				ICMove(rd, rs);
			}
			else err(line, symTable.value);
			num++;
			getNextSymTable();
		}
		if (num != length[now-1]) errorPro(line, 'n');
		if (symTable.symbol != RBRACE) err(line, symTable.value);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else err(line, symTable.value);
}

void vardefinit(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon;
	if (symCount == 2) {
		Category category = var;
		Type type = symTableArray[0].symbol == INTTK ? Integer : Char;
		int dimention = 0;
		for (int i = 0; i < symCount; i++) {
			gtNodepSon = createTreeNode(symTableArray[i]);
			insert(*gtNodepp, gtNodepSon);
		}
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == INTCON || symTable.symbol == CHARCON || symTable.symbol == PLUS || symTable.symbol == MINU) {
			gtNodepSon = createTreeNode(CONST);
			Operand rs = constA(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
			if ((symTable.symbol != CHARCON && type == Char) || (symTable.symbol == CHARCON && type == Integer)) {
				errorPro(line, 'o');
			}
			if (symTable.symbol == CHARCON) {
				insertId(symTableArray[1].value, category, type, dimention, symTable.value[0]);
				Operand rd = *newOperand(idTables[idNum - 1]);
				ICMove(rd, rs);
			}
			else if (symTable.symbol == INTCON || symTable.symbol == PLUS || symTable.symbol == MINU) {
				insertId(symTableArray[1].value, category, type, dimention, atoi(symTable.value));
				Operand rd = *newOperand(idTables[idNum - 1]);
				ICMove(rd, rs);
			}
		}
		else {
			err(line, symTable.value);
		}
		getNextSymTable();
		if (symTable.symbol != SEMICN) err(line, symTable.value);
	}
	else if (symCount == 5 || symCount == 8) {
		Category category = ::array;
		Type type = symTableArray[0].symbol == INTTK ? Integer : Char;
		int dimention = symCount == 5 ? 1 : 2;
		int length[10], dim = 0;
		for (int i = 0; i < symCount; i++) {
			if (symTableArray[i].symbol != INTCON) {
				gtNodepSon = createTreeNode(symTableArray[i]);
				insert(*gtNodepp, gtNodepSon);
			}
			else {
				gtNodepSon = createTreeNode(UNSIGNEDINTEGER);
				gtNodePointer gtNodepSon2 = createTreeNode(symTableArray[i]);
				insert(gtNodepSon, gtNodepSon2);
				insert(*gtNodepp, gtNodepSon);
				length[dim++] = atoi(symTableArray[i].value);
			}
		}
		insertId(symTableArray[1].value, category, type, dimention, length);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == LBRACE) {
			int count = 0;
			arrayInit(gtNodepp, dimention, length, 1, type, idTables[idNum-1], &count);
		}
		else {
			err(line, symTable.value);
		}
		getNextSymTable();
		if (symTable.symbol != SEMICN) err(line, symTable.value);
	}
	symCount = 0;
}

//void const_(gtNodePointer* gtNodepp) {
//	if (symTable.symbol == CHARCON) {
//		symCheck(symTable);
//		gtNodePointer gtNodepSon = createTreeNode(symTable);
//		insert(*gtNodepp, gtNodepSon);
//	}
//	else if (symTable.symbol == PLUS || symTable.symbol == MINU || symTable.symbol == INTCON) {
//		gtNodePointer gtNodepSon = createTreeNode(INTEGER);
//		integer(&gtNodepSon);
//		insert(*gtNodepp, gtNodepSon);
//	}
//}

Operand constA(gtNodePointer* gtNodepp) {
	Operand op;
	if (symTable.symbol == CHARCON) {
		symCheck(symTable);
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		op = *newOperand(symTable.value[0], Char);
	}
	else if (symTable.symbol == PLUS || symTable.symbol == MINU || symTable.symbol == INTCON) {
		gtNodePointer gtNodepSon = createTreeNode(INTEGER);
		op = integerA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	return op;
}

void functiondef(gtNodePointer* gtNodepp) {
	infunc = 1;
	gtNodePointer gtNodepSon = createTreeNode(STATEHEAD);
	statehead(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	isReturned = 0;
	addLevel();//进入下一层
	if (symTable.symbol == INTTK || symTable.symbol == CHARTK || symTable.symbol == RPARENT) {
		gtNodepSon = createTreeNode(PARALIST);
		paralist(&gtNodepSon, idNum-1);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol != RPARENT) {
		errorPro(line, 'l');
		err(line, symTable.value);
		ungetsym = 1;
	}
	else {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}

	getNextSymTable();
	if (symTable.symbol != LBRACE) err(line, symTable.value);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	gtNodepSon = createTreeNode(COMSTATEMENT);
	comstatement(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol != RBRACE) err(line, symTable.value);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	clearLevel();//返回上一层
	if (isReturned == 0) {
		errorPro(line, 'h');
	}
	FunctionDefEnd();
	infunc = 0;
}

void statehead(gtNodePointer* gtNodepp) {
	if (symCount == 2) {
		Category category = func;
		Type type = symTableArray[0].symbol == INTTK ? Integer : Char;
		insertId(symTableArray[1].value, category, type, 0);
		FunctionDefBegin(symTableArray[1].value);
		returnType = type;
		gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
		insert(*gtNodepp, gtNodepSon);
		gtNodepSon = createTreeNode(symTableArray[1]);
		insert(*gtNodepp, gtNodepSon);
		strcpy(functions[functionNum].name, symTableArray[1].value);
		functions[functionNum++].void_ = 0;
		symCount = 0;
	}
	else {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		Type type = symTable.symbol == INTTK ? Integer : Char;
		Category category = func;
		getNextSymTable();
		if (symTable.symbol != IDENFR) err(line, symTable.value);
		insertId(symTable.value, category, type, 0);
		FunctionDefBegin(symTable.value);
		returnType = type;
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		strcpy(functions[functionNum].name, symTable.value);
		functions[functionNum++].void_ = 0;
		getNextSymTable();
		if (symTable.symbol != LPARENT) err(line, symTable.value);
	}
}

void paralist(gtNodePointer* gtNodepp, int loc) {
	if (symTable.symbol == RPARENT) return;
	Category category = var;
	Type type = symTable.symbol == INTTK ? Integer : Char;
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol != IDENFR) err(line, symTable.value);
	insertId(symTable.value, category, type, 0);
	insertPara(loc, type, symTable.value);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	while (symTable.symbol == COMMA) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != INTTK && symTable.symbol != CHARTK) err(line, symTable.value);
		Category category = var;
		Type type = symTable.symbol == INTTK ? Integer : Char;
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != IDENFR) err(line, symTable.value);
		insertId(symTable.value, category, type, 0);
		insertPara(loc, type, symTable.value);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
}

void voidfunctiondef(gtNodePointer* gtNodepp) {
	infunc = 1;
	gtNodePointer gtNodepSon;
	if (symCount == 1) {
		gtNodepSon = createTreeNode(symTableArray[0]);
		insert(*gtNodepp, gtNodepSon);
		symCount = 0;
	}
	else {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	if (symTable.symbol != IDENFR) err(line, symTable.value);
	Category category = voidfunc;
	Type type = Void;
	returnType = type;
	isReturned = 0;
	insertId(symTable.value, category, type, 0);
	FunctionDefBegin(symTable.value);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	strcpy(functions[functionNum].name, symTable.value);
	functions[functionNum++].void_ = 1;
	getNextSymTable();
	if (symTable.symbol != LPARENT) err(line, symTable.value);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable(); 
	addLevel();//进入下一层
	if (symTable.symbol == INTTK || symTable.symbol == CHARTK || symTable.symbol == RPARENT) {
		gtNodepSon = createTreeNode(PARALIST);
		paralist(&gtNodepSon, idNum-1);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol != RPARENT) {
		errorPro(line, 'l');
		err(line, symTable.value);
		ungetsym = 1;
	}
	else {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	getNextSymTable();
	if (symTable.symbol != LBRACE) err(line, symTable.value);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	gtNodepSon = createTreeNode(COMSTATEMENT);
	comstatement(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol != RBRACE) err(line, symTable.value);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	clearLevel();//返回上一层
	if (isReturned == 0) {
		ICRET();
	}
	FunctionDefEnd();
	infunc = 0;
}

void mainfunction(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
	insert(*gtNodepp, gtNodepSon);
	symCount = 0;
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol != LPARENT) err(line, symTable.value); 
	addLevel();//进入下一层
	returnType = Void;
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol != RPARENT) {
		errorPro(line, 'l');
		err(line, symTable.value);
		ungetsym = 1;
	}
	else {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	getNextSymTable();
	if (symTable.symbol != LBRACE) err(line, symTable.value);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	gtNodepSon = createTreeNode(COMSTATEMENT);
	comstatement(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol != RBRACE) err(line, symTable.value);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	clearLevel();//返回上一层
}

void comstatement(gtNodePointer* gtNodepp) {
	if (symTable.symbol == CONSTTK) {
		gtNodePointer gtNodepSon = createTreeNode(CONSTSTATE);
		conststate(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
		gtNodePointer gtNodepSon = createTreeNode(VARSTATE);
		varstate(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	Operand op = *newOperand(addr, Integer);
	ICAlloc(op);//调整sp指针
	if (symTable.symbol == RBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(STATEMENTLIST);
		insert(*gtNodepp, gtNodepSon);
		ungetsym = 1;
	}
	else {
		gtNodePointer gtNodepSon = createTreeNode(STATEMENTLIST);
		statementlist(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
}

void statementlist(gtNodePointer* gtNodepp) {
	while (symTable.symbol != RBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(STATEMENT);
		statement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	ungetsym = 1;
}

void statement(gtNodePointer* gtNodepp) {
	if (symTable.symbol == WHILETK || symTable.symbol == FORTK) {
		gtNodePointer gtNodepSon = createTreeNode(LOOPSTATEMENT);
		loopstatement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == IFTK) {
		gtNodePointer gtNodepSon = createTreeNode(CONDITIONSTATEMENT);
		conditionstatement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == IDENFR) {
		symTableArray[symCount++] = symTable;
		int searchResult = searchId(symTableArray[0].value);
		if (searchResult == -1) {
			errorPro(line, 'c');
		}
		getNextSymTable();
		if (symTable.symbol == ASSIGN || symTable.symbol == LBRACK) {
			gtNodePointer gtNodepSon = createTreeNode(ASSIGNSTATEMENT);
			assignstatement(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
		}
		else if (symTable.symbol == LPARENT) {
			for (int i = 0; i < functionNum; i++) {
				if (strcmp(functions[i].name, symTableArray[0].value) == 0) {
					if (functions[i].void_) {
						gtNodePointer gtNodepSon = createTreeNode(VOIDFUNCTIONCALL);
						voidfunctioncall(&gtNodepSon);
						insert(*gtNodepp, gtNodepSon);
					}
					else {
						gtNodePointer gtNodepSon = createTreeNode(FUNCTIONCALL);
						functioncallA(&gtNodepSon);
						insert(*gtNodepp, gtNodepSon);
					}
					break;
				}
			}
		}
		else {
			err(line, symTable.value);
		}
		getNextSymTable();
		if (symTable.symbol == SEMICN) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			if (lineend == 1) {
				errorPro(line - 1, 'k');
			}
			else {
				errorPro(line, 'k');
			}
			err(line, symTable.value);
			ungetsym = 1;
		}
	}
	else if (symTable.symbol == SCANFTK) {
		gtNodePointer gtNodepSon = createTreeNode(SCAN);
		scan(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == SEMICN) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			if (lineend == 1) {
				errorPro(line - 1, 'k');
			}
			else {
				errorPro(line, 'k');
			}
			err(line, symTable.value);
			ungetsym = 1;
		}
	}
	else if (symTable.symbol == PRINTFTK) {
		gtNodePointer gtNodepSon = createTreeNode(PRINT);
		print_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == SEMICN) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			if (lineend == 1) {
				errorPro(line - 1, 'k');
			}
			else {
				errorPro(line, 'k');
			}
			err(line, symTable.value);
			ungetsym = 1;
		}
	}
	else if (symTable.symbol == SWITCHTK) {
		gtNodePointer gtNodepSon = createTreeNode(SWITCH);
		switch_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == SEMICN) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == RETURNTK) {
		gtNodePointer gtNodepSon = createTreeNode(RETURN);
		return_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == SEMICN) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			if (lineend == 1) {
				errorPro(line - 1, 'k');
			}
			else {
				errorPro(line, 'k');
			}
			err(line, symTable.value);
			ungetsym = 1;
		}
	}
	else if (symTable.symbol == LBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == RBRACE) {
			gtNodePointer gtNodepSon = createTreeNode(STATEMENTLIST);
			insert(*gtNodepp, gtNodepSon);
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			gtNodePointer gtNodepSon = createTreeNode(STATEMENTLIST);
			statementlist(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			if (symTable.symbol != RBRACE) err(line, symTable.value);
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
	}
	else {
		err(line, symTable.value);
	}
}

void loopstatement(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon;
	if (symTable.symbol == WHILETK) {
		//生成开始标号label0
		Operand label0 = *getLab();
		//生成结束标号label1
		Operand label1 = *getLab();
		//设置开始标号
		ICSetLab(label0);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != LPARENT) err(line, symTable.value); 
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(CONDITION);
		Operand con = conditionA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		//条件跳转至label1
		ICBeq(con, *newOperand(0, Integer), label1);
		if (symTable.symbol != RPARENT) {
			errorPro(line, 'l');
			err(line, symTable.value);
			ungetsym = 1;
		}
		else {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		getNextSymTable();
		gtNodePointer gtNodepSon = createTreeNode(STATEMENT);
		statement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		//无条件跳转至label0
		ICJ(label0);
		//设置label1
		ICSetLab(label1);
	}
	else if (symTable.symbol == FORTK) {
		//生成开始标号label0
		Operand label0 = *getLab();
		//生成结束标号label1
		Operand label1 = *getLab();
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != LPARENT) err(line, symTable.value); 
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != IDENFR) err(line, symTable.value); 
		int searchResult = searchId(symTable.value);
		if (searchResult == -1) {
			errorPro(line, 'c');
		}
		else if (idTables[searchResult].category == con) {
			errorPro(line, 'j');
		}
		Operand rd = *newOperand(idTables[searchResult]);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != ASSIGN) err(line, symTable.value);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(EXP);
		Operand rs = expA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		ICMove(rd, rs);
		//设置开始标号
		ICSetLab(label0);
		if (symTable.symbol != SEMICN) {
			errorPro(line, 'k');
			err(line, symTable.value);
			ungetsym = 1;
		}
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(CONDITION);
		Operand Con = conditionA(&gtNodepSon);
		//条件跳转至label1
		ICBeq(Con, *newOperand(0, Integer), label1);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != SEMICN) {
			errorPro(line, 'k');
			err(line, symTable.value);
			ungetsym = 1;
		}
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != IDENFR) err(line, symTable.value);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		searchResult = searchId(symTable.value);
		if (searchResult == -1) {
			errorPro(line, 'c');
		}
		else if (idTables[searchResult].category == con) {
			errorPro(line, 'j');
		}
		rd = *newOperand(idTables[searchResult]);
		getNextSymTable();
		if (symTable.symbol != ASSIGN) err(line, symTable.value);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != IDENFR) err(line, symTable.value);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		searchResult = searchId(symTable.value);
		if (searchResult == -1) {
			errorPro(line, 'c');
		}
		else if (idTables[searchResult].category == con) {
			errorPro(line, 'j');
		}
		rs = *newOperand(idTables[searchResult]);
		getNextSymTable();
		Symbol sym = symTable.symbol;
		if (symTable.symbol == PLUS || symTable.symbol == MINU) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else err(line, symTable.value);
		getNextSymTable();
		Operand rt;
		if (symTable.symbol == INTCON) {
			gtNodepSon = createTreeNode(STEPSIZE);
			rt = stepsizeA(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
		}
		else err(line, symTable.value);
		getNextSymTable();
		if (symTable.symbol != RPARENT) {
			errorPro(line, 'l');
			err(line, symTable.value);
			ungetsym = 1;
		}
		else {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		getNextSymTable();
		gtNodepSon = createTreeNode(STATEMENT);
		statement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		//更新指示变量
		Operand T = *tempOperand();
		if (sym == PLUS) {
			ICAdd(T, rs, rt);
			ICMove(rd, T);
		}
		else {
			ICSub(T, rs, rt);
			ICMove(rd, T);
		}
		//无条件跳转至label0
		ICJ(label0);
		//设置label1
		ICSetLab(label1);
	}
	else {
		err(line, symTable.value);
	}
}

//void stepsize(gtNodePointer* gtNodepp) {
//	gtNodePointer gtNodepSon = createTreeNode(UNSIGNEDINTEGER);
//	unsignedinteger(&gtNodepSon);
//	insert(*gtNodepp, gtNodepSon);
//}

Operand stepsizeA(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(UNSIGNEDINTEGER);
	Operand op = unsignedintegerA(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	return op;
}

void conditionstatement(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == LPARENT) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else err(line, symTable.value);
	getNextSymTable();
	gtNodepSon = createTreeNode(CONDITION);
	//生成标号LABEL1
	Operand label1 = *getLab();
	Operand con = conditionA(&gtNodepSon);
	//条件跳转至LABEL1
	ICBeq(con, *newOperand(0, Integer), label1);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == RPARENT) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		errorPro(line, 'l');
		err(line, symTable.value);
		ungetsym = 1;
	}
	getNextSymTable();
	gtNodepSon = createTreeNode(STATEMENT);
	statement(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == ELSETK) {
		//生成标号LABEL2
		Operand label2 = *getLab();
		//无条件跳转到LABEL2
		ICJ(label2);
		//设置标号LABEL1
		ICSetLab(label1);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodePointer gtNodepSon = createTreeNode(STATEMENT);
		statement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		//设置标号LABEL2
		ICSetLab(label2);
	}
	else {
		//设置标号LABEL1
		ICSetLab(label1);
		ungetsym = 1;
	}
}

//void condition(gtNodePointer* gtNodepp) {
//	gtNodePointer gtNodepSon = createTreeNode(EXP);
//	exp(&gtNodepSon);
//	insert(*gtNodepp, gtNodepSon);
//	if (getExpType(gtNodepSon) != Integer) {
//		errorPro(line, 'f');
//	}
//	getNextSymTable();
//	if (symTable.symbol == LSS|| symTable.symbol == LEQ|| symTable.symbol == GRE||
//		symTable.symbol == GEQ|| symTable.symbol == EQL|| symTable.symbol == NEQ) {
//		gtNodepSon = createTreeNode(symTable);
//		insert(*gtNodepp, gtNodepSon);
//	}
//	else err(line, symTable.value);
//	getNextSymTable();
//	gtNodepSon = createTreeNode(EXP);
//	exp(&gtNodepSon);
//	insert(*gtNodepp, gtNodepSon);
//	if (getExpType(gtNodepSon) != Integer) {
//		errorPro(line, 'f');
//	}
//}

Operand conditionA(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(EXP);
	Operand rs = expA(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	if (getExpType(gtNodepSon) != Integer) {
		errorPro(line, 'f');
	}
	getNextSymTable();
	Symbol sym = EQL;
	if (symTable.symbol == LSS || symTable.symbol == LEQ || symTable.symbol == GRE ||
		symTable.symbol == GEQ || symTable.symbol == EQL || symTable.symbol == NEQ) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		sym = symTable.symbol;
	}
	else err(line, symTable.value);
	getNextSymTable();
	gtNodepSon = createTreeNode(EXP);
	Operand rt = expA(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	if (getExpType(gtNodepSon) != Integer) {
		errorPro(line, 'f');
	}
	Operand rd = *tempOperand();
	if (sym == LSS) {
		ICSlt(rd, rs, rt);
	}
	else if (sym == LEQ) {
		ICSle(rd, rs, rt);
	}
	else if (sym == GRE) {
		ICSgt(rd, rs, rt);
	}
	else if (sym == GEQ) {
		ICSge(rd, rs, rt);
	}
	else if (sym == EQL) {
		ICSeq(rd, rs, rt);
	}
	else if (sym == NEQ) {
		ICSne(rd, rs, rt);
	}
	return rd;
}

//void functioncall(gtNodePointer* gtNodepp) {
//	int searchResult = searchId(symTableArray[0].value);
//	if (searchResult == -1) {
//		errorPro(line, 'c');
//	}
//	IdTable func = idTables[searchResult];
//	Operand funcop = *newOperand(func);
//	gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
//	symCount = 0;
//	insert(*gtNodepp, gtNodepSon);
//	gtNodepSon = createTreeNode(symTable);
//	insert(*gtNodepp, gtNodepSon);
//	getNextSymTable();
//	gtNodepSon = createTreeNode(VALUEPARALIST);
//	valueparalist(&gtNodepSon, func);
//	insert(*gtNodepp, gtNodepSon);
//	if (symTable.symbol == RPARENT) {
//		gtNodepSon = createTreeNode(symTable);
//		insert(*gtNodepp, gtNodepSon);
//	}
//	else {
//		errorPro(line, 'l');
//		err(line, symTable.value);
//		ungetsym = 1;
//	}
//	ICCall(funcop);
//}

Operand functioncallA(gtNodePointer* gtNodepp) {
	ICParaBegin();	//函数调用开始，保护寄存器
	int searchResult = searchId(symTableArray[0].value);
	if (searchResult == -1) {
		errorPro(line, 'c');
	}
	IdTable func = idTables[searchResult];
	Operand funcop = *newOperand(func);
	gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
	symCount = 0;
	insert(*gtNodepp, gtNodepSon);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	gtNodepSon = createTreeNode(VALUEPARALIST);
	valueparalist(&gtNodepSon, func);
	insert(*gtNodepp, gtNodepSon);
	if (symTable.symbol == RPARENT) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		errorPro(line, 'l');
		err(line, symTable.value);
		ungetsym = 1;
	}
	Operand T = *tempOperand();
	ICCallV(funcop, T); //跳转与返回
	return T;
}

void voidfunctioncall(gtNodePointer* gtNodepp) {
	ICParaBegin();
	int searchResult = searchId(symTableArray[0].value);
	if (searchResult == -1) {
		errorPro(line, 'c');
	}
	IdTable func = idTables[searchResult];
	Operand funcop = *newOperand(func);
	gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
	symCount = 0;
	insert(*gtNodepp, gtNodepSon);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	gtNodepSon = createTreeNode(VALUEPARALIST);
	valueparalist(&gtNodepSon, func);
	insert(*gtNodepp, gtNodepSon);
	if (symTable.symbol == RPARENT) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		errorPro(line, 'l');
		err(line, symTable.value);
		ungetsym = 1;
	}
	ICCall(funcop);
}

void valueparalist(gtNodePointer* gtNodepp, IdTable func) {
	if (symTable.symbol == RPARENT) {
		if (func.paraNum != 0) {
			errorPro(line, 'd');
		}
		return;
	}
	else if (symTable.symbol == SEMICN) {
		return;
	}
	int realparaNum = 0;
	gtNodePointer gtNodepSon = createTreeNode(EXP);
	Operand para = expA(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	if (realparaNum < func.paraNum && getExpType(gtNodepSon) != func.paralist[realparaNum]) {
		errorPro(line, 'e');
	}
	realparaNum++;
	ICPara(para);
	getNextSymTable();
	while (symTable.symbol == COMMA)
	{
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodePointer gtNodepSon = createTreeNode(EXP);
		Operand para = expA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		if (realparaNum <= func.paraNum && getExpType(gtNodepSon) != func.paralist[realparaNum]) {
			errorPro(line, 'e');
		}
		realparaNum++;
		getNextSymTable();
		ICPara(para);	//压入参数
	}
	if (realparaNum > func.paraNum) {
		errorPro(line, 'd');
	}
}

//void arrayValueAt(gtNodePointer* gtNodepp) {
//	gtNodePointer gtNodepSon = createTreeNode(symTable);
//	insert(*gtNodepp, gtNodepSon);
//	getNextSymTable();
//	gtNodepSon = createTreeNode(EXP);
//	exp(&gtNodepSon);
//	insert(*gtNodepp, gtNodepSon);
//	getNextSymTable();
//	if (getExpType(gtNodepSon) != Integer) {
//		errorPro(line, 'i');
//	}
//	if (symTable.symbol == RBRACK) {
//		gtNodePointer gtNodepSon = createTreeNode(symTable);
//		insert(*gtNodepp, gtNodepSon);
//	}
//	else {
//		errorPro(line, 'm');
//		err(line, symTable.value);
//		ungetsym = 1;
//	}
//}

Operand arrayValueAtA(gtNodePointer* gtNodepp) {
	Operand op;
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	gtNodepSon = createTreeNode(EXP);
	op = expA(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (getExpType(gtNodepSon) != Integer) {
		errorPro(line, 'i');
	}
	if (symTable.symbol == RBRACK) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		errorPro(line, 'm');
		err(line, symTable.value);
		ungetsym = 1;
	}
	return op;
}

void assignstatement(gtNodePointer* gtNodepp) {
	int searchResult = searchId(symTableArray[0].value);
	if (searchResult == -1) {
		errorPro(line, 'c');
	}
	else if (idTables[searchResult].category == con) {
		errorPro(line, 'j');
	}
	gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
	insert(*gtNodepp, gtNodepSon);
	Operand rd = *newOperand(idTables[searchResult]);
	symCount = 0;
	if (symTable.symbol == ASSIGN && idTables[searchResult].category == ::array) {
		errorPro(line, 'j');
	}
	if (symTable.symbol == LBRACK) {
		Operand T, op[2];
		int i = 0;
		while (symTable.symbol == LBRACK) {
			op[i] = arrayValueAtA(gtNodepp);
			getNextSymTable();
			i++;
		}
		if (i == 1) {
			if (idTables[searchResult].type == Integer) {
				Operand T1 = *tempOperand();
				ICMult(T1, op[0], *newOperand(4, Integer));
				T = T1;
			}
			else {
				T = op[0];
			}
		}
		else {
			Operand T1 = *tempOperand();
			ICMult(T1, op[0], *newOperand(idTables[searchResult].length[1], Integer));
			Operand T2 = *tempOperand();
			ICAdd(T2, op[1], T1);
			if (idTables[searchResult].type == Integer) {
				Operand T3 = *tempOperand();
				ICMult(T3, T2, *newOperand(4, Integer));
				T = T3;
			}
			else {
				T = T2;
			}
		}
		if (symTable.symbol == ASSIGN) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else err(line, symTable.value);
		getNextSymTable();
		gtNodepSon = createTreeNode(EXP);
		Operand rt = expA(&gtNodepSon);
		ICArrayWrite(rd, T, rt);
	}
	else {
		if (symTable.symbol == ASSIGN) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else err(line, symTable.value);
		getNextSymTable();
		gtNodepSon = createTreeNode(EXP);
		Operand rs = expA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		ICMove(rd, rs);
	}
}

void scan(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == LPARENT) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else err(line, symTable.value);
	getNextSymTable();
	if (symTable.symbol == IDENFR) {
		int searchResult = searchId(symTable.value);
		if (searchResult == -1) {
			errorPro(line, 'c');
		}
		else if (idTables[searchResult].category == con || idTables[searchResult].category == ::array) {
			errorPro(line, 'j');
		}
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		ICRead(*newOperand(idTables[searchResult]));
	}
	else err(line, symTable.value);
	getNextSymTable();
	if (symTable.symbol == RPARENT) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		errorPro(line, 'l');
		err(line, symTable.value);
		ungetsym = 1;
	}
}

void print_(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == LPARENT) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else err(line, symTable.value);
	getNextSymTable();
	if (symTable.symbol == STRCON) {
		symCheck(symTable);
		int loc = addstr(symTable.value);
		ICPrint(*newOperand(loc, String));
		gtNodePointer gtNodepSon = createTreeNode(STRING);
		string_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == COMMA) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
		}
		else if (symTable.symbol == RPARENT) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
			ICenter();
			return;
		}
	}
	gtNodepSon = createTreeNode(EXP);
	Operand T = expA(&gtNodepSon);
	T.type = getExpType(gtNodepSon);
	ICPrint(T);
	ICenter();
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == RPARENT) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		errorPro(line, 'l');
		err(line, symTable.value);
		ungetsym = 1;
	}
}

void string_(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
}

void switch_(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == LPARENT) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line, symTable.value);
	}
	getNextSymTable();
	gtNodepSon = createTreeNode(EXP);
	Operand con = expA(&gtNodepSon);
	Operand var = *newOperand(createTempVar(con.type));
	ICMove(var, con);
	insert(*gtNodepp, gtNodepSon);
	Type type = getExpType(gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == RPARENT) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		errorPro(line, 'l');
		err(line, symTable.value);
		ungetsym = 1;
	}
	getNextSymTable();
	//生成LABEL0
	Operand label0 = *getLab();
	if (symTable.symbol == LBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line, symTable.value);
	}
	getNextSymTable();
	if (symTable.symbol == CASETK) {
		gtNodePointer gtNodepSon = createTreeNode(CASELIST);
		caselist(&gtNodepSon, type, var, label0);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line, symTable.value);
	}
	getNextSymTable();
	if (symTable.symbol == DEFAULTTK) {
		gtNodePointer gtNodepSon = createTreeNode(DEFAULT);
		default_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		errorPro(line, 'p');
		err(line, symTable.value);
		ungetsym = 1;
	}
	getNextSymTable();
	if (symTable.symbol == RBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line, symTable.value);
	}
	//设置LABEL0
	ICSetLab(label0);
}

void caselist(gtNodePointer* gtNodepp, Type type, Operand var, Operand label0) {
	while (symTable.symbol == CASETK) {
		gtNodePointer gtNodepSon = createTreeNode(CASE);
		case_(&gtNodepSon, type, var, label0);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	ungetsym = 1;
}

void case_(gtNodePointer* gtNodepp, Type type, Operand var, Operand label0) {
	//生成下一个case的标签
	Operand label_next = *getLab();
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	Operand temp;
	if (symTable.symbol == PLUS || symTable.symbol == MINU || symTable.symbol == INTCON || symTable.symbol == CHARCON) {
		gtNodePointer gtNodepSon = createTreeNode(CONST);
		temp = constA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		if ((symTable.symbol != CHARCON && type == Char) || (symTable.symbol == CHARCON && type == Integer)) {
			errorPro(line, 'o');
		}
		//条件跳转至下一个case
		ICBne(var, temp, label_next);
	}
	else {
		err(line, symTable.value);
	}
	getNextSymTable();
	if (symTable.symbol == COLON) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line, symTable.value);
	}
	getNextSymTable();
	gtNodepSon = createTreeNode(STATEMENT);
	statement(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	//无条件跳转至结尾label0
	ICJ(label0);
	//设置下一个case的标签
	ICSetLab(label_next);
}

void default_(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == COLON) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line, symTable.value);
	}
	getNextSymTable();
	gtNodepSon = createTreeNode(STATEMENT);
	statement(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
}

void return_(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == LPARENT) {
		if (returnType == Void) {
			errorPro(line, 'g');
		}
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(EXP);
		Operand ret = expA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		if (getExpType(gtNodepSon) != returnType && returnType != Void) {
			errorPro(line, 'h');
		}
		getNextSymTable();
		if (symTable.symbol == RPARENT) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			errorPro(line, 'l');
			err(line, symTable.value);
			ungetsym = 1;
		}
		isReturned = 1;
		ICRETV(ret);
	}
	else {
		if (infunc) {
			ICRET();
		}
		else {
			ICEnd();
		}
		if (returnType != Void) {
			errorPro(line, 'h');
		}
		ungetsym = 1;
	}
}

//void exp(gtNodePointer* gtNodepp) {
//	if (symTable.symbol == PLUS || symTable.symbol == MINU) {
//		gtNodePointer gtNodepSon = createTreeNode(symTable);
//		insert(*gtNodepp, gtNodepSon);
//		getNextSymTable();
//	}
//	gtNodePointer gtNodepSon = createTreeNode(ITEM);
//	item(&gtNodepSon);
//	insert(*gtNodepp, gtNodepSon);
//	getNextSymTable();
//	while (symTable.symbol == PLUS || symTable.symbol == MINU) {
//		gtNodePointer gtNodepSon = createTreeNode(symTable);
//		insert(*gtNodepp, gtNodepSon);
//		getNextSymTable();
//		gtNodepSon = createTreeNode(ITEM);
//		item(&gtNodepSon);
//		insert(*gtNodepp, gtNodepSon);
//		getNextSymTable();
//	}
//	ungetsym = 1;
//}
//
//void item(gtNodePointer* gtNodepp) {
//	gtNodePointer gtNodepSon = createTreeNode(FACTOR);
//	factor(&gtNodepSon);
//	insert(*gtNodepp, gtNodepSon);
//	getNextSymTable();
//	while (symTable.symbol == MULT || symTable.symbol == DIV) {
//		gtNodePointer gtNodepSon = createTreeNode(symTable);
//		insert(*gtNodepp, gtNodepSon);
//		getNextSymTable();
//		gtNodepSon = createTreeNode(FACTOR);
//		factor(&gtNodepSon);
//		insert(*gtNodepp, gtNodepSon);
//		getNextSymTable();
//	}
//	ungetsym = 1;
//}
//
//void factor(gtNodePointer* gtNodepp) {
//	if (symTable.symbol == IDENFR) {
//		int searchResult = searchId(symTable.value);
//		if (searchResult == -1) {
//			errorPro(line, 'c');
//		}
//		symTableArray[symCount++] = symTable;
//		getNextSymTable();
//		if (symTable.symbol == LBRACK) {
//			gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
//			insert(*gtNodepp, gtNodepSon);
//			symCount = 0;
//			while (symTable.symbol == LBRACK) {
//				arrayValueAt(gtNodepp);
//				getNextSymTable();
//			}
//			ungetsym = 1;
//		}
//		else if (symTable.symbol == LPARENT) {
//			gtNodePointer gtNodepSon = createTreeNode(FUNCTIONCALL);
//			functioncall(&gtNodepSon);
//			insert(*gtNodepp, gtNodepSon);
//		}
//		else {
//			gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
//			insert(*gtNodepp, gtNodepSon);
//			symCount = 0;
//			ungetsym = 1;
//		}
//	}
//	else if (symTable.symbol == LPARENT) {
//		gtNodePointer gtNodepSon = createTreeNode(symTable);
//		insert(*gtNodepp, gtNodepSon);
//		getNextSymTable();
//		gtNodepSon = createTreeNode(EXP);
//		exp(&gtNodepSon);
//		insert(*gtNodepp, gtNodepSon);
//		getNextSymTable();
//		if (symTable.symbol == RPARENT) {
//			gtNodePointer gtNodepSon = createTreeNode(symTable);
//			insert(*gtNodepp, gtNodepSon);
//		}
//		else {
//			errorPro(line, 'l');
//			err(line, symTable.value);
//			ungetsym = 1;
//		}
//	}
//	else if (symTable.symbol == PLUS || symTable.symbol == MINU || symTable.symbol == INTCON) {
//		gtNodePointer gtNodepSon = createTreeNode(INTEGER);
//		integer(&gtNodepSon);
//		insert(*gtNodepp, gtNodepSon);
//	}
//	else if (symTable.symbol == CHARCON) {
//		symCheck(symTable);
//		gtNodePointer gtNodepSon = createTreeNode(symTable);
//		insert(*gtNodepp, gtNodepSon);
//	}
//	else {
//		err(line, symTable.value);
//	}
//}

Operand expA(gtNodePointer* gtNodepp) {
	Operand T;
	int sym = 0;
	if (symTable.symbol == PLUS || symTable.symbol == MINU) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		if (symTable.symbol == MINU) sym = 1;
		getNextSymTable();
	}
	gtNodePointer gtNodepSon = createTreeNode(ITEM);
	T = itemA(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	if (sym == 1) {
		Operand rs = *newOperand(0, Integer);
		Operand rt = T;
		T = *tempOperand();
		ICSub(T, rs, rt);
	}
	getNextSymTable();
	while (symTable.symbol == PLUS || symTable.symbol == MINU) {
		int sym_ = 0;
		if (symTable.symbol == MINU) sym_ = 1;
		Operand rs = T;
		T = *tempOperand();
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(ITEM);
		Operand rt = itemA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (sym_ == 0) {
			ICAdd(T, rs, rt);
		}
		else {
			ICSub(T, rs, rt);
		}
	}
	ungetsym = 1;
	return T;
}

Operand itemA(gtNodePointer* gtNodepp) {
	Operand T;
	gtNodePointer gtNodepSon = createTreeNode(FACTOR);
	T = factorA(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	while (symTable.symbol == MULT || symTable.symbol == DIV) {
		int sym = 0;
		if (symTable.symbol == DIV) sym = 1;
		Operand rs = T;
		T = *tempOperand();
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(FACTOR);
		Operand rt = factorA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (sym) 
			ICDiv(T, rs, rt);
		else 
			ICMult(T, rs, rt);
	}
	ungetsym = 1;
	return T;
}

Operand factorA(gtNodePointer* gtNodepp) {
	Operand T;
	if (symTable.symbol == IDENFR) {
		int searchResult = searchId(symTable.value);
		if (searchResult == -1) {
			errorPro(line, 'c');
		}
		symTableArray[symCount++] = symTable;
		getNextSymTable();
		if (symTable.symbol == LBRACK) {
			T = *tempOperand();
			gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
			insert(*gtNodepp, gtNodepSon);
			symCount = 0;
			Operand op[2];
			int i = 0;
			while (symTable.symbol == LBRACK) {
				op[i] = arrayValueAtA(gtNodepp);
				getNextSymTable();
				i++;
			}
			ungetsym = 1;
			if (i == 1) {
				if (idTables[searchResult].type == Integer) {
					Operand T1 = *tempOperand();
					ICMult(T1, op[0], *newOperand(4, Integer));
					ICArrayRead(T, *newOperand(idTables[searchResult]), T1);
				}
				else {
					ICArrayRead(T, *newOperand(idTables[searchResult]), op[0]);
				}
			}
			else {
				Operand T1 = *tempOperand();
				ICMult(T1, op[0], *newOperand(idTables[searchResult].length[1], Integer));
				Operand T2 = *tempOperand();
				ICAdd(T2, op[1], T1);
				if (idTables[searchResult].type == Integer) {
					Operand T3 = *tempOperand();
					ICMult(T3, T2, *newOperand(4, Integer));
					ICArrayRead(T, *newOperand(idTables[searchResult]), T3);
				}
				else {
					ICArrayRead(T, *newOperand(idTables[searchResult]), T2);
				}
			}
		}
		else if (symTable.symbol == LPARENT) {
			gtNodePointer gtNodepSon = createTreeNode(FUNCTIONCALL);
			T = functioncallA(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			T = *newOperand(idTables[searchResult]);
			gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
			insert(*gtNodepp, gtNodepSon);
			symCount = 0;
			ungetsym = 1;
		}
	}
	else if (symTable.symbol == LPARENT) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(EXP);
		T = expA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == RPARENT) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			errorPro(line, 'l');
			err(line, symTable.value);
			ungetsym = 1;
		}
	}
	else if (symTable.symbol == PLUS || symTable.symbol == MINU || symTable.symbol == INTCON) {
		gtNodePointer gtNodepSon = createTreeNode(INTEGER);
		T = integerA(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == CHARCON) {
		symCheck(symTable);
		T = *newOperand(symTable.value[0], Char);
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line, symTable.value);
	}
	return T;
}



Type getExpType(gtNodePointer gtNodep) {
	if (gtNodep->sonNode[1] != NULL) return Integer;//唯一项
	gtNodePointer gtNodepSon = gtNodep->sonNode[0];
	if (gtNodepSon->sonNode[1] != NULL) return Integer;//唯一因子
	gtNodepSon = gtNodepSon->sonNode[0];
	gtNodepSon = gtNodepSon->sonNode[0];
	if (gtNodepSon->vn == FUNCTIONCALL) {
		gtNodepSon = gtNodepSon->sonNode[0];
		int searchResult = searchId(gtNodepSon->symTable->value);
		if (idTables[searchResult].type == Char) return Char;
		else return Integer;
	}
	else if (gtNodepSon->symTable != NULL) {
		SymTable* factorSym = gtNodepSon->symTable;
		if (factorSym->symbol == IDENFR) {
			int searchResult = searchId(factorSym->value);
			if (idTables[searchResult].type == Char) return Char;
			else return Integer;
		}
		else if (factorSym->symbol == CHARCON) {
			return Char;
		}
	}
	return Integer;
}

void parsering(FILE* in) {
	inFile = in;
	getNextSymTable();
	if (symTable.symbol == CONSTTK || symTable.symbol == INTTK || symTable.symbol == CHARTK || symTable.symbol == VOIDTK) {
		grammerTree = createTreeNode(PROGRAM);
		program(&grammerTree);
	}
	else {
		err(line, symTable.value);
	}
}
