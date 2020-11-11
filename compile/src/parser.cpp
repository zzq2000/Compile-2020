#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include "parser.h"
#include "symTables.h"
#include "error.h"
#include "idTable.h"
FILE* inFile;
gtNodePointer grammerTree = NULL;
SymTable symTable;
SymTable symTableArray[20];
int symCount = 0;
int ungetsym = 0;
int level = 0;
Type returnType = Void;
int isReturned = 0;
void getNextSymTable() {
	if (ungetsym) {
		ungetsym = 0;
	}
	else {
		symTable = getsym_(inFile);
	}
}

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
function functions[100];
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
		insertId(symTable.value, category, type, 0, level);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != ASSIGN) {
			err(line, symTable.value);
		}
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == INTCON || symTable.symbol == PLUS || symTable.symbol == MINU) {
			gtNodePointer gtNodepSon = createTreeNode(INTEGER);
			integer(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
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
		insertId(symTable.value, category, type, 0, level);
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
		getNextSymTable();
	} while (symTable.symbol == COMMA);
	ungetsym = 1;
}

void integer(gtNodePointer* gtNodepp) {
	if (symTable.symbol == PLUS || symTable.symbol == MINU) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	if (symTable.symbol == INTCON) {
		gtNodePointer gtNodepSon = createTreeNode(UNSIGNEDINTEGER);
		unsignedinteger(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line, symTable.value);
	}
}

void unsignedinteger(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
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
	Category category = symCount == 2 ? var : array;
	Type type = symTableArray[0].symbol == INTTK ? Integer : Char;
	int dimention = symCount == 5 ? 1 : 
					symCount == 8 ? 2 : 0;
	insertId(symTableArray[1].value, category, type, dimention, level);
	for (int i = 0; i < symCount; i++) {
		if (symTableArray[i].symbol == INTCON) {
			gtNodepSon = createTreeNode(UNSIGNEDINTEGER);
			gtNodePointer gtNodepSon2 = createTreeNode(symTableArray[i]);
			insert(gtNodepSon, gtNodepSon2);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			gtNodepSon = createTreeNode(symTableArray[i]);
			insert(*gtNodepp, gtNodepSon);
		}
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
			Category category = symCount == 1 ? var : array;
			int dimention = symCount == 4 ? 1 :
				symCount == 7 ? 2 : 0;
			insertId(symTableArray[0].value, category, type, dimention, level);
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
				}
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

void arrayInit(gtNodePointer* gtNodepp, int dim, int length[], int now, Type type) {
	if (dim < now) {
		errorPro(line, 'n');
	}
	int num = 0;
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == LBRACE) {
		arrayInit(gtNodepp, dim, length, now+1, type);
		getNextSymTable();
		num++;
		while (symTable.symbol == COMMA) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			arrayInit(gtNodepp, dim, length, now + 1, type);
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
		const_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		if ((symTable.symbol != CHARCON && type == Char) || (symTable.symbol == CHARCON && type == Integer)) {
			errorPro(line, 'o');
		}
		num++;
		getNextSymTable();
		while (symTable.symbol == COMMA) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			if (symTable.symbol == INTCON || symTable.symbol == CHARCON || symTable.symbol == PLUS || symTable.symbol == MINU) {
				gtNodepSon = createTreeNode(CONST);
				const_(&gtNodepSon);
				insert(*gtNodepp, gtNodepSon);
				if ((symTable.symbol != CHARCON && type == Char) || (symTable.symbol == CHARCON && type == Integer)) {
					errorPro(line, 'o');
				}
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
		insertId(symTableArray[1].value, category, type, dimention, level);
		for (int i = 0; i < symCount; i++) {
			gtNodepSon = createTreeNode(symTableArray[i]);
			insert(*gtNodepp, gtNodepSon);
		}
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == INTCON || symTable.symbol == CHARCON || symTable.symbol == PLUS || symTable.symbol == MINU) {
			gtNodepSon = createTreeNode(CONST);
			const_(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
			if ((symTable.symbol != CHARCON && type == Char) || (symTable.symbol == CHARCON && type == Integer)) {
				errorPro(line, 'o');
			}
		}
		else {
			err(line, symTable.value);
		}
		getNextSymTable();
		if (symTable.symbol != SEMICN) err(line, symTable.value);
	}
	else if (symCount == 5 || symCount == 8) {
		Category category = array;
		Type type = symTableArray[0].symbol == INTTK ? Integer : Char;
		int dimention = symCount == 5 ? 1 : 2;
		insertId(symTableArray[1].value, category, type, dimention, level);
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
				length[dim++] = symTableArray[i].value[0] - '0';
			}
		}
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == LBRACE) {
			arrayInit(gtNodepp, dimention, length, 1, type);
		}
		else {
			err(line, symTable.value);
		}
		getNextSymTable();
		if (symTable.symbol != SEMICN) err(line, symTable.value);
	}
	symCount = 0;
}

void const_(gtNodePointer* gtNodepp) {
	if (symTable.symbol == CHARCON) {
		symCheck(symTable);
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == PLUS || symTable.symbol == MINU || symTable.symbol == INTCON) {
		gtNodePointer gtNodepSon = createTreeNode(INTEGER);
		integer(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
}

void functiondef(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(STATEHEAD);
	statehead(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	isReturned = 0;
	level++;//进入下一层
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
	clearLevel(level);
	level--;//返回上一层
	if (isReturned == 0) {
		errorPro(line, 'h');
	}
}

void statehead(gtNodePointer* gtNodepp) {
	if (symCount == 2) {
		Category category = func;
		Type type = symTableArray[0].symbol == INTTK ? Integer : Char;
		insertId(symTableArray[1].value, category, type, 0, level);
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
		insertId(symTable.value, category, type, 0, level);
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
	insertId(symTable.value, category, type, 0, level);
	insertPara(loc, type);
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
		insertId(symTable.value, category, type, 0, level);
		insertPara(loc, type);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
}

void voidfunctiondef(gtNodePointer* gtNodepp) {
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
	insertId(symTable.value, category, type, 0, level);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	strcpy(functions[functionNum].name, symTable.value);
	functions[functionNum++].void_ = 1;
	getNextSymTable();
	if (symTable.symbol != LPARENT) err(line, symTable.value);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable(); 
	level++;//进入下一层
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
	clearLevel(level);
	level--;//返回上一层
}

void mainfunction(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
	insert(*gtNodepp, gtNodepSon);
	symCount = 0;
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol != LPARENT) err(line, symTable.value); 
	level++;//进入下一层
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
	clearLevel(level);
	level--;//返回上一层
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
						functioncall(&gtNodepSon);
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
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != LPARENT) err(line, symTable.value); 
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(CONDITION);
		condition(&gtNodepSon);
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
		gtNodePointer gtNodepSon = createTreeNode(STATEMENT);
		statement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == FORTK) {
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
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != ASSIGN) err(line, symTable.value);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(EXP);
		exp(&gtNodepSon);
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
		gtNodepSon = createTreeNode(CONDITION);
		condition(&gtNodepSon);
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
		getNextSymTable();
		if (symTable.symbol != ASSIGN) err(line, symTable.value);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol != IDENFR) err(line, symTable.value);
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == PLUS || symTable.symbol == MINU) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else err(line, symTable.value);
		getNextSymTable();
		if (symTable.symbol == INTCON) {
			gtNodepSon = createTreeNode(STEPSIZE);
			stepsize(&gtNodepSon);
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
	}
	else {
		err(line, symTable.value);
	}
}

void stepsize(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(UNSIGNEDINTEGER);
	unsignedinteger(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
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
	condition(&gtNodepSon);
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
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodePointer gtNodepSon = createTreeNode(STATEMENT);
		statement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		ungetsym = 1;
	}
}

void condition(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(EXP);
	exp(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	if (getExpType(gtNodepSon) != Integer) {
		errorPro(line, 'f');
	}
	getNextSymTable();
	if (symTable.symbol == LSS|| symTable.symbol == LEQ|| symTable.symbol == GRE||
		symTable.symbol == GEQ|| symTable.symbol == EQL|| symTable.symbol == NEQ) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else err(line, symTable.value);
	getNextSymTable();
	gtNodepSon = createTreeNode(EXP);
	exp(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	if (getExpType(gtNodepSon) != Integer) {
		errorPro(line, 'f');
	}
}

void functioncall(gtNodePointer* gtNodepp) {
	int searchResult = searchId(symTableArray[0].value);
	if (searchResult == -1) {
		errorPro(line, 'c');
	}
	IdTable func = idTables[searchResult];
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
}

void voidfunctioncall(gtNodePointer* gtNodepp) {
	int searchResult = searchId(symTableArray[0].value);
	if (searchResult == -1) {
		errorPro(line, 'c');
	}
	IdTable func = idTables[searchResult];
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
	exp(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	if (realparaNum < func.paraNum && getExpType(gtNodepSon) != func.paralist[realparaNum]) {
		errorPro(line, 'e');
	}
	realparaNum++;
	getNextSymTable();
	while (symTable.symbol == COMMA)
	{
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodePointer gtNodepSon = createTreeNode(EXP);
		exp(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		if (realparaNum <= func.paraNum && getExpType(gtNodepSon) != func.paralist[realparaNum]) {
			errorPro(line, 'e');
		}
		realparaNum++;
		getNextSymTable();
	}
	if (realparaNum > func.paraNum) {
		errorPro(line, 'd');
	}
}

void arrayValueAt(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	gtNodepSon = createTreeNode(EXP);
	exp(&gtNodepSon);
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
	symCount = 0;
	insert(*gtNodepp, gtNodepSon);
	if (symTable.symbol == ASSIGN && idTables[searchResult].category == array) {
		errorPro(line, 'j');
	}
	if (symTable.symbol == LBRACK) {
		while (symTable.symbol == LBRACK) {
			arrayValueAt(gtNodepp);
			getNextSymTable();
		}
	}
	if (symTable.symbol == ASSIGN) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else err(line, symTable.value);
	getNextSymTable();
	gtNodepSon = createTreeNode(EXP);
	exp(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
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
		else if (idTables[searchResult].category == con || idTables[searchResult].category == array) {
			errorPro(line, 'j');
		}
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
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
			return;
		}
	}
	gtNodepSon = createTreeNode(EXP);
	exp(&gtNodepSon);
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
	exp(&gtNodepSon);
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
		caselist(&gtNodepSon, type);
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
}

void caselist(gtNodePointer* gtNodepp, Type type) {
	while (symTable.symbol == CASETK) {
		gtNodePointer gtNodepSon = createTreeNode(CASE);
		case_(&gtNodepSon, type);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	ungetsym = 1;
}

void case_(gtNodePointer* gtNodepp, Type type) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == PLUS || symTable.symbol == MINU || symTable.symbol == INTCON || symTable.symbol == CHARCON) {
		gtNodePointer gtNodepSon = createTreeNode(CONST);
		const_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		if ((symTable.symbol != CHARCON && type == Char) || (symTable.symbol == CHARCON && type == Integer)) {
			errorPro(line, 'o');
		}
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
		exp(&gtNodepSon);
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
	}
	else {
		if (returnType != Void) {
			errorPro(line, 'h');
		}
		ungetsym = 1;
	}
}

void exp(gtNodePointer* gtNodepp) {
	if (symTable.symbol == PLUS || symTable.symbol == MINU) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	gtNodePointer gtNodepSon = createTreeNode(ITEM);
	item(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	while (symTable.symbol == PLUS || symTable.symbol == MINU) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(ITEM);
		item(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	ungetsym = 1;
}

void item(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(FACTOR);
	factor(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	while (symTable.symbol == MULT || symTable.symbol == DIV) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		gtNodepSon = createTreeNode(FACTOR);
		factor(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	ungetsym = 1;
}

void factor(gtNodePointer* gtNodepp) {
	if (symTable.symbol == IDENFR) {
		int searchResult = searchId(symTable.value);
		if (searchResult == -1) {
			errorPro(line, 'c');
		}
		symTableArray[symCount++] = symTable;
		getNextSymTable();
		if (symTable.symbol == LBRACK) {
			gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
			insert(*gtNodepp, gtNodepSon);
			symCount = 0;
			while (symTable.symbol == LBRACK) {
				arrayValueAt(gtNodepp);
				getNextSymTable();
			}
			ungetsym = 1;
		}
		else if (symTable.symbol == LPARENT) {
			gtNodePointer gtNodepSon = createTreeNode(FUNCTIONCALL);
			functioncall(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
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
		exp(&gtNodepSon);
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
		integer(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == CHARCON) {
		symCheck(symTable);
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line, symTable.value);
	}
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
