#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "parser.h"
#include "symTables.h"
#include "error.h"
FILE* inFile;
gtNodePointer grammerTree = NULL;
SymTable symTable;
SymTable symTableArray[20];
int symCount = 0;
int ungetsym = 0;
void getNextSymTable() {
	if (ungetsym) {
		ungetsym = 0;
	}
	else {
		symTable = getsym_(inFile);
	}
}

typedef struct {
	char* name;
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
	"<变量定义无初始化>",
	"<变量定义及初始化>",
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
		fprintf(outFile, "%s %s", sym[gtNodep->symTable->symbol], gtNodep->symTable->value);
	}
	else {
		fprintf(outFile, "%s", vnList[gtNodep->vn]);
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
			else {
				err(line);
			}
		}
	}
	if (symTable.symbol == VOIDTK) {
		symTableArray[symCount++] = symTable;
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			gtNodePointer gtNodepSon = createTreeNode(VOIDFUNCTIONDEF);
			voidfunctiondef(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			while (symTable.symbol == INTTK || symTable.symbol == CHARTK || symTable.symbol == VOIDTK) {
				if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
					gtNodePointer gtNodepSon = createTreeNode(FUNCTIONDEF);
					functiondef(&gtNodepSon);
					insert(*gtNodepp, gtNodepSon);
				}
				else
				{
					gtNodePointer gtNodepSon = createTreeNode(VOIDFUNCTIONDEF);
					functiondef(&gtNodepSon);
					insert(*gtNodepp, gtNodepSon);
				}
				getNextSymTable();
			}
		} 
		else if (symTable.symbol == MAINTK) {
			gtNodePointer gtNodepSon = createTreeNode(MAINFUNCTION);
			mainfunction(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			if (symTable.symbol != nullsym) err(line);
			return;
		}
		else {
			err(line);
		}
	}
	if (symTable.symbol == MAINTK) {
		gtNodePointer gtNodepSon = createTreeNode(MAINFUNCTION);
		mainfunction(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol != nullsym) err(line);
}

void conststate(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == INTTK) {
		gtNodePointer gtNodepSon = createTreeNode(CONSTDEF);
		intconstdef(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == CHARTK) {
		gtNodePointer gtNodepSon = createTreeNode(CONSTDEF);
		charconstdef(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else
	{
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == SEMICN) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == CONSTTK) {
			conststate(gtNodepp);
		}
	}
	else {
		err(line);
	}
}

void intconstdef(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == IDENFR) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == ASSIGN) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == INTCON || symTable.symbol == PLUS || symTable.symbol == MINU) {
		gtNodePointer gtNodepSon = createTreeNode(INTEGER);
		integer(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	getNextSymTable();
	while (symTable.symbol == COMMA) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == ASSIGN) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == INTCON || symTable.symbol == PLUS || symTable.symbol == MINU) {
			gtNodePointer gtNodepSon = createTreeNode(INTEGER);
			integer(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
		}
		getNextSymTable();
	}
}

void charconstdef(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == IDENFR) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == ASSIGN) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == CHARCON) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
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
		err(line);
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
		err(line);
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
				else if (symTable.symbol == LPARENT) break;
				else {
					err(line);
				}
				getNextSymTable();
			}
			else {
				err(line);
			}
		}
		else {
			err(line);
		}
	}
}

void vardef(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon;
	if (symCount == 0) {
		symTableArray[symCount++] = symTable;
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			symTableArray[symCount++] = symTable;
			getNextSymTable();
		}
		else {
			err(line);
		}
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
	else if (symTable.symbol == LBRACK) {
		symTableArray[symCount++] = symTable;
		getNextSymTable();
		if (symTable.symbol == INTCON) {
			symTableArray[symCount++] = symTable;
			getNextSymTable();
			if (symTable.symbol == RBRACK) {
				symTableArray[symCount++] = symTable;
			}
			else {
				err(line);
			}
			getNextSymTable();
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
			else if(symTable.symbol == LBRACK) {
				symTableArray[symCount++] = symTable;
				getNextSymTable();
				if (symTable.symbol == INTCON) {
					symTableArray[symCount++] = symTable;
					getNextSymTable();
					if (symTable.symbol == RBRACK) {
						symTableArray[symCount++] = symTable;
						getNextSymTable();
					}
					else {
						err(line);
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
					else {
						err(line);
					}
				}
				else {
					err(line);
				}
			}
			else {
				err(line);
			}
		}
		else {
			err(line);
		}
	}
}

void vardefnoinit(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon;
	for (int i = 0; i < symCount; i++) {
		gtNodepSon = createTreeNode(symTableArray[i]);
		insert(*gtNodepp, gtNodepSon);
	}
	symCount = 0;
	while (symTable.symbol == COMMA) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			if (symTable.symbol == LBRACK) {
				gtNodepSon = createTreeNode(symTable);
				insert(*gtNodepp, gtNodepSon);
				getNextSymTable();
				if (symTable.symbol == INTCON) {
					gtNodepSon = createTreeNode(symTable);
					insert(*gtNodepp, gtNodepSon);
				}
				else {
					err(line);
				}
				getNextSymTable();
				if (symTable.symbol == RBRACK) {
					gtNodepSon = createTreeNode(symTable);
					insert(*gtNodepp, gtNodepSon);
				}
				else {
					err(line);
				}
				getNextSymTable();
				if (symTable.symbol == LBRACK) {
					gtNodepSon = createTreeNode(symTable);
					insert(*gtNodepp, gtNodepSon);
					getNextSymTable();
					if (symTable.symbol == INTCON) {
						gtNodepSon = createTreeNode(symTable);
						insert(*gtNodepp, gtNodepSon);
					}
					else {
						err(line);
					}
					getNextSymTable();
					if (symTable.symbol == RBRACK) {
						gtNodepSon = createTreeNode(symTable);
						insert(*gtNodepp, gtNodepSon);
					}
					else {
						err(line);
					}
					getNextSymTable();
				}
			}
			else {
				err(line);
			}
		}
		else {
			err(line);
		}
	}
}

void vardefinit(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon;
	for (int i = 0; i < symCount; i++) {
		gtNodepSon = createTreeNode(symTableArray[i]);
		insert(*gtNodepp, gtNodepSon);
	}
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symCount == 2) {
		if (symTable.symbol == INTCON) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol != SEMICN) err(line);
	}
	else if (symCount == 5) {
		if (symTable.symbol == LBRACE) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
			gtNodepSon = createTreeNode(CONST);
			const_(&gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		while (symTable.symbol == COMMA) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
				gtNodepSon = createTreeNode(CONST);
				const_(&gtNodepSon);
			}
			else {
				err(line);
			}
			getNextSymTable();
		}
		if (symTable.symbol == RBRACE) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol != SEMICN) err(line);
	}
	else if (symCount == 8) {
		if (symTable.symbol == LBRACE) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == LBRACE) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
			gtNodepSon = createTreeNode(CONST);
			const_(&gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		while (symTable.symbol == COMMA) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
				gtNodepSon = createTreeNode(CONST);
				const_(&gtNodepSon);
			}
			else {
				err(line);
			}
			getNextSymTable();
		}
		if (symTable.symbol == RBRACE) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		while (symTable.symbol == COMMA) {
			if (symTable.symbol == LBRACE) {
				gtNodepSon = createTreeNode(symTable);
				insert(*gtNodepp, gtNodepSon);
			}
			else {
				err(line);
			}
			getNextSymTable();
			if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
				gtNodepSon = createTreeNode(CONST);
				const_(&gtNodepSon);
			}
			else {
				err(line);
			}
			getNextSymTable();
			while (symTable.symbol == COMMA) {
				gtNodepSon = createTreeNode(symTable);
				insert(*gtNodepp, gtNodepSon);
				getNextSymTable();
				if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
					gtNodepSon = createTreeNode(CONST);
					const_(&gtNodepSon);
				}
				else {
					err(line);
				}
				getNextSymTable();
			}
			if (symTable.symbol == RBRACE) {
				gtNodepSon = createTreeNode(symTable);
				insert(*gtNodepp, gtNodepSon);
			}
			else {
				err(line);
			}
			getNextSymTable();
		}
		if (symTable.symbol == RBRACE) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol != SEMICN) err(line);
	}
	else
	{
		err(line);
	}
	symCount = 0;
}

void const_(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
}

void functiondef(gtNodePointer* gtNodepp) {
	functions[functionNum].name = symTableArray[1].value;
	functions[functionNum++].void_ = 0;
	gtNodePointer gtNodepSon = createTreeNode(STATEHEAD);
	statehead(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
		gtNodepSon = createTreeNode(PARALIST);
		paralist(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol == RPARENT) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == LBRACE) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == CONSTTK || symTable.symbol == INTTK || symTable.symbol == CHARTK ||
		symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
		symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
		symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) {
		gtNodepSon = createTreeNode(COMSTATEMENT);
		comstatement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == RBRACE) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
}

void statehead(gtNodePointer* gtNodepp) {
	if (symCount == 2) {
		gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
		insert(*gtNodepp, gtNodepSon);
		gtNodepSon = createTreeNode(symTableArray[1]);
		insert(*gtNodepp, gtNodepSon);
		symCount = 0;
	}
	else {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(0);
		}
		getNextSymTable();
		if (symTable.symbol != LPARENT) err(line);
	}
}

void paralist(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == IDENFR) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	while (symTable.symbol == COMMA) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
	}
}

void voidfunctiondef(gtNodePointer* gtNodepp) {
	functions[functionNum].name = symTableArray[1].value;
	functions[functionNum++].void_ = 1;
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
	if (symTable.symbol == IDENFR) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == LPARENT) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
		gtNodepSon = createTreeNode(PARALIST);
		paralist(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol == RPARENT) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == LBRACE) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == CONSTTK || symTable.symbol == INTTK || symTable.symbol == CHARTK ||
		symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
		symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
		symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) {
		gtNodepSon = createTreeNode(COMSTATEMENT);
		comstatement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == RBRACE) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
}

void mainfunction(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
	insert(*gtNodepp, gtNodepSon);
	symCount = 0;
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == LPARENT) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == RPARENT) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == LBRACE) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	getNextSymTable();
	if (symTable.symbol == CONSTTK || symTable.symbol == INTTK || symTable.symbol == CHARTK ||
		symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
		symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
		symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) {
		gtNodepSon = createTreeNode(COMSTATEMENT);
		comstatement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == RBRACE) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
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
	if (symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
		symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
		symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(STATEMENTLIST);
		statementlist(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
}

void statementlist(gtNodePointer* gtNodepp) {
	while (symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
			symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
			symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) 
	{
		gtNodePointer gtNodepSon = createTreeNode(STATEMENT);
		statement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
}

void statement(gtNodePointer* gtNodepp) {
	if (symTable.symbol == WHILETK || symTable.symbol == FORTK) {
		gtNodePointer gtNodepSon = createTreeNode(LOOPSTATEMENT);
		loopstatement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol == IFTK) {
		gtNodePointer gtNodepSon = createTreeNode(CONDITIONSTATEMENT);
		conditionstatement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol == IDENFR) {
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
			err(line);
		}
	}
	if (symTable.symbol == SCANFTK) {
		gtNodePointer gtNodepSon = createTreeNode(SCAN);
		scan(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol == PRINTFTK) {
		gtNodePointer gtNodepSon = createTreeNode(PRINT);
		print_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol == SWITCHTK) {
		gtNodePointer gtNodepSon = createTreeNode(SWITCH);
		switch_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol == SEMICN) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol == RETURNTK) {
		gtNodePointer gtNodepSon = createTreeNode(RETURN);
		return_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol == LBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
			symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
			symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) {
			gtNodePointer gtNodepSon = createTreeNode(STATEMENTLIST);
			statementlist(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == RBRACE) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
	}
}

void loopstatement(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon;
	if (symTable.symbol == WHILETK) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == LPARENT) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
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
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
			symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
			symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) {
			gtNodePointer gtNodepSon = createTreeNode(STATEMENT);
			statement(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
	}
	else if (symTable.symbol == FORTK) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == LPARENT) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == ASSIGN) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		gtNodepSon = createTreeNode(EXP);
		exp(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		if (symTable.symbol == SEMICN) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		gtNodepSon = createTreeNode(CONDITION);
		condition(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		if (symTable.symbol == SEMICN) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == ASSIGN) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == PLUS || symTable.symbol == MINU) {
			gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == INTCON) {
			gtNodepSon = createTreeNode(STEPSIZE);
			stepsize(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
			symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
			symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) {
			gtNodepSon = createTreeNode(STATEMENT);
			statement(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
	}
	else {
		err(line);
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
	else {
		err(line);
	}
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
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
		symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
		symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(STATEMENT);
		statement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == ELSETK) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
			symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
			symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) {
			gtNodePointer gtNodepSon = createTreeNode(STATEMENT);
			statement(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
	}
	else {
		ungetsym = 1;
	}
}

void condition(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(EXP);
	exp(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == LSS|| symTable.symbol == LEQ|| symTable.symbol == GRE||
		symTable.symbol == GEQ|| symTable.symbol == EQL|| symTable.symbol == NEQ) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	gtNodepSon = createTreeNode(EXP);
	exp(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
}

void functioncall(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
	symCount = 0;
	insert(*gtNodepp, gtNodepSon);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable(); 
	if (symTable.symbol != RPARENT) {
		gtNodePointer gtNodepSon = createTreeNode(VALUEPARALIST);
		valueparalist(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol == RPARENT) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
}

void voidfunctioncall(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
	symCount = 0;
	insert(*gtNodepp, gtNodepSon);
	gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol != RPARENT) {
		gtNodePointer gtNodepSon = createTreeNode(VALUEPARALIST);
		valueparalist(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	if (symTable.symbol == RPARENT) {
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
}

void valueparalist(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(EXP);
	exp(&gtNodepSon);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	while (symTable.symbol == COMMA)
	{
		gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		gtNodePointer gtNodepSon = createTreeNode(EXP);
		exp(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}

}

void assignstatement(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTableArray[0]);
	symCount = 0;
	insert(*gtNodepp, gtNodepSon);
	if (symTable.symbol == ASSIGN) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == LBRACK) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		gtNodepSon = createTreeNode(EXP);
		exp(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == RBRACK) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == ASSIGN) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else if (symTable.symbol == LBRACK) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
			gtNodepSon = createTreeNode(EXP);
			exp(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			if (symTable.symbol == RBRACK) {
				gtNodePointer gtNodepSon = createTreeNode(symTable);
				insert(*gtNodepp, gtNodepSon);
			}
			else {
				err(line);
			}
			getNextSymTable();
			if (symTable.symbol == ASSIGN) {
				gtNodePointer gtNodepSon = createTreeNode(symTable);
				insert(*gtNodepp, gtNodepSon);
			}
			else {
				err(line);
			}
		}
		else {
			err(line);
		}
	}
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
		getNextSymTable();
		if (symTable.symbol == IDENFR) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
		getNextSymTable();
		if (symTable.symbol == RPARENT) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
	}
	else {
		err(line);
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
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == STRCON) {
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
		err(line);
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
		err(line);
	}
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
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == LBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == CASETK) {
		gtNodePointer gtNodepSon = createTreeNode(CASELIST);
		caselist(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == DEFAULTTK) {
		gtNodePointer gtNodepSon = createTreeNode(DEFAULT);
		default_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == RBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
}

void caselist(gtNodePointer* gtNodepp) {
	while (symTable.symbol == CASETK) {
		gtNodePointer gtNodepSon = createTreeNode(CASE);
		case_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
	}
	ungetsym = 1;
}

void case_(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == INTTK || symTable.symbol == CHARTK) {
		gtNodePointer gtNodepSon = createTreeNode(CONST);
		const_(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
		symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
		symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(STATEMENT);
		statement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
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
		err(line);
	}
	getNextSymTable();
	if (symTable.symbol == WHILETK || symTable.symbol == FORTK || symTable.symbol == IFTK || symTable.symbol == IDENFR ||
		symTable.symbol == SCANFTK || symTable.symbol == PRINTFTK || symTable.symbol == SWITCHTK ||
		symTable.symbol == SEMICN || symTable.symbol == RETURNTK || symTable.symbol == LBRACE) {
		gtNodePointer gtNodepSon = createTreeNode(STATEMENT);
		statement(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
}

void return_(gtNodePointer* gtNodepp) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodepp, gtNodepSon);
	getNextSymTable();
	if (symTable.symbol == LPARENT) {
		gtNodepSon = createTreeNode(EXP);
		exp(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == RPARENT) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
		}
		else {
			err(line);
		}
	}
	else {
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
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
		getNextSymTable();
		if (symTable.symbol == LBRACK) {
			gtNodePointer gtNodepSon = createTreeNode(symTable);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			gtNodepSon = createTreeNode(EXP);
			exp(&gtNodepSon);
			insert(*gtNodepp, gtNodepSon);
			getNextSymTable();
			if (symTable.symbol == RBRACK) {
				gtNodePointer gtNodepSon = createTreeNode(symTable);
				insert(*gtNodepp, gtNodepSon);
			}
			else {
				err(line);
			}
			getNextSymTable();
			if (symTable.symbol == LBRACK) {
				gtNodePointer gtNodepSon = createTreeNode(symTable);
				insert(*gtNodepp, gtNodepSon);
				getNextSymTable();
				gtNodepSon = createTreeNode(EXP);
				exp(&gtNodepSon);
				insert(*gtNodepp, gtNodepSon);
				getNextSymTable();
				if (symTable.symbol == RBRACK) {
					gtNodePointer gtNodepSon = createTreeNode(symTable);
					insert(*gtNodepp, gtNodepSon);
				}
				else {
					err(line);
				}
			}
			else {
				ungetsym = 1;
			}
		}
		else {
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
			err(line);
		}
	}
	else if (symTable.symbol == PLUS || symTable.symbol == MINU || symTable.symbol == INTCON) {
		gtNodePointer gtNodepSon = createTreeNode(INTEGER);
		integer(&gtNodepSon);
		insert(*gtNodepp, gtNodepSon);
	}
	else if (symTable.symbol == CHARTK) {
		gtNodePointer gtNodepSon = createTreeNode(symTable);
		insert(*gtNodepp, gtNodepSon);
	}
	else {
		err(line);
	}
}

void parsering(FILE* in) {
	inFile = in;
	getNextSymTable();
	if (symTable.symbol == CONSTTK || symTable.symbol == INTTK || symTable.symbol == CHARTK || symTable.symbol == VOIDTK) {
		grammerTree = createTreeNode(PROGRAM);
		program(&grammerTree);
	}
	else {
		err(line);
	}
}