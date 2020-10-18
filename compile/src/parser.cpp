#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "parser.h"
#include "symTables.h"
#include "error.h"
FILE* inFile;
gtNodePointer grammerTree = NULL;
char vnList[][40] = {
	"<程序>",
	"<常量说明>",
	"<常量说明>",
	"<常量定义>",
	"<标识符>",
	"<整数>",
	"<无符号整数>",
	"<字母>",
	"<数字>",
	"<变量说明>",
	"<变量定义>",
	"<变量定义无初始化>",
	"<变量定义及初始化>",
	"<类型标识符>",
	"<常量>",
	"<字符>",
	"<加法运算符>",
	"<乘法运算符>",
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
	"<关系运算符>",
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

void program(SymTable symTable, gtNodePointer* gtNodep) {
	if (symTable.symbol == CONSTTK) {
		gtNodePointer gtNodepSon = createTreeNode(CONSTSTATE);
		insert(*gtNodep, gtNodepSon);
		conststate(symTable, &gtNodepSon);
	}
}

void conststate(SymTable symTable, gtNodePointer* gtNodep) {
	gtNodePointer gtNodepSon = createTreeNode(symTable);
	insert(*gtNodep, gtNodepSon);
	SymTable symTable1 = getsym_(inFile);
	if (symTable1.symbol == INTCON || symTable1.symbol == CHARCON) {
		gtNodePointer gtNodepSon = createTreeNode(CONSTDEF);
		insert(*gtNodep, gtNodepSon);
		constdef(symTable1, &gtNodepSon);
	}
}

void constdef(SymTable symTable, gtNodePointer* gtNodep) {

}

void parsering(FILE* in) {
	inFile = in;
	SymTable symTable = getsym_(inFile);
	if (strcmp(symTable.value,"") != 0) {
		grammerTree = createTreeNode(PROGRAM);
		program(symTable, &grammerTree);
	}
	else {
		err(line);
	}
}