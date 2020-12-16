#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"idTable.h"
#include"Intermediate.h"
#include"objectCode.h"
#include"str.h"
#include"register.h"
FILE* OCfile = fopen("mips.txt", "w");

int paraNum = 0;	//在调用函数的参数数目
char** paraName = NULL;//在调用函数时会将其设置为函数的参数表，在调用结束以后置空
Operand paralist[10][32];//储存作为参数的操作数，当发生函数调用时，在保存现场结束以后，将操作数放入寄存器以及压进栈中
int callParaNum[10] = {0,};
int callNum = 0;
void IC2OC(IC);
int searchPara(Operand op) {
	if (paraName != NULL) {
		for (int i = 0; i < paraNum; i++) {
			if (strcmp(paraName[i], op.name) == 0) {
				return i;
			}
		}
	}
	return -1;
}
char* searchOp(Operand op) {
	if (op.category == con) {
		return NULL;
	}
	int result;
	result = searchReg(op);
	if (paraName != NULL) {
		for (int i = 0; i < paraNum; i++) {
			if (strcmp(paraName[i], op.name) == 0) {
				if (i < 3) {
					return paraRegName[i+1];
				}
				else {
					int t = setReg(op);
					fprintf(OCfile, "lw %s, %d($fp)\n", globalRegName[t], 4*(i-2));//
					return  globalRegName[t];
				}
			}
		}
	}
	if (op.category == temp) {
		if (result == -1 || tempReg[result].name == NULL) {
			printf("%s\n", op.name);
			return NULL;
		}
		return tempRegName[result];
	}
	else if (op.category == var) {
		//当查找的操作数是变量时，如果查找失败，则给变量分配一个全局寄存器，然后返回该全局寄存器
		if (result == -1) {
			result = setReg(op);
			if (op.type == Integer) {
				if (op.addr >= 0) {
					fprintf(OCfile, "lw %s, %d($v1)\n", globalRegName[result], op.addr);
				}
				else {
					fprintf(OCfile, "lw %s, %d($fp)\n", globalRegName[result], op.addr);
				}
			}
			else {
				if (op.addr >= 0) {
					fprintf(OCfile, "lb %s, %d($v1)\n", globalRegName[result], op.addr);
				}
				else {
					fprintf(OCfile, "lb %s, %d($fp)\n", globalRegName[result], op.addr);
				}
			}
		}
		else if (globalReg[result].dirty == 1) {
			if (op.type == Integer) {
				if (op.addr >= 0) {
					fprintf(OCfile, "lw %s, %d($v1)\n", globalRegName[result], op.addr);
				}
				else {
					fprintf(OCfile, "lw %s, %d($fp)\n", globalRegName[result], op.addr);
				}
			}
			else {
				if (op.addr >= 0) {
					fprintf(OCfile, "lb %s, %d($v1)\n", globalRegName[result], op.addr);
				}
				else {
					fprintf(OCfile, "lb %s, %d($fp)\n", globalRegName[result], op.addr);
				}
			}
		}
		return globalRegName[result];
	}
	return NULL;
}

char* searchParaOp(Operand op) {
	if (op.category == con) {
		return NULL;
	}
	int result;
	result = searchReg(op);
	if (paraName != NULL) {
		for (int i = 0; i < paraNum; i++) {
			if (strcmp(paraName[i], op.name) == 0) {
				if (i < 3) {
					return paraRegName[i + 1];
				}
				else {
					int temp = setReg(op);
					fprintf(OCfile, "lw %s, %d($fp)\n", globalRegName[temp], 4 * (i - 2));//
					clearGlobalReg(temp);
					return  globalRegName[temp];
				}
			}
		}
	}
	if (op.category == temp) {
		if (result == -1 || tempReg[result].name == NULL) {
			printf("%s\n", op.name);
			return NULL;
		}
		return tempRegName[result];
	}
	else if (op.category == var) {
		//当查找的操作数是变量时，如果查找失败，则给变量分配一个全局寄存器，然后返回该全局寄存器
		if (result == -1) {
			result = setReg(op);
			if (op.type == Integer) {
				if (op.addr >= 0) {
					fprintf(OCfile, "lw %s, %d($v1)\n", globalRegName[result], op.addr);
				}
				else {
					fprintf(OCfile, "lw %s, %d($fp)\n", globalRegName[result], op.addr);
				}
			}
			else {
				if (op.addr >= 0) {
					fprintf(OCfile, "lb %s, %d($v1)\n", globalRegName[result], op.addr);
				}
				else {
					fprintf(OCfile, "lb %s, %d($fp)\n", globalRegName[result], op.addr);
				}
			}
			clearGlobalReg(result);
		}
		else if (globalReg[result].dirty == 1) {
			if (op.type == Integer) {
				if (op.addr >= 0) {
					fprintf(OCfile, "lw %s, %d($v1)\n", globalRegName[result], op.addr);
				}
				else {
					fprintf(OCfile, "lw %s, %d($fp)\n", globalRegName[result], op.addr);
				}
			}
			else {
				if (op.addr >= 0) {
					fprintf(OCfile, "lb %s, %d($v1)\n", globalRegName[result], op.addr);
				}
				else {
					fprintf(OCfile, "lb %s, %d($fp)\n", globalRegName[result], op.addr);
				}
			}
		}
		return globalRegName[result];
	}
	return NULL;
}

void addPara(Operand op) {
	paralist[callNum - 1][callParaNum[callNum - 1]] = op;
	callParaNum[callNum - 1]++;
}

void setPara() {
	if (callParaNum[callNum - 1] <= 3) {
		for (int i = 0; i < callParaNum[callNum - 1]; i++) {
			if (paralist[callNum - 1][i].category == con) {
				fprintf(OCfile, "addi $a%d, $0, %d\n", i + 1, paralist[callNum - 1][i].value);
			}
			else {
				fprintf(OCfile, "move $a%d, %s\n", i + 1, searchParaOp(paralist[callNum - 1][i]));
			}
		}
	}
	else {
		for (int i = 0; i < 3; i++) {
			if (paralist[callNum - 1][i].category == con) {
				fprintf(OCfile, "addi $a%d, $0, %d\n", i + 1, paralist[callNum - 1][i].value);
			}
			else {

				fprintf(OCfile, "move $a%d, %s\n", i + 1, searchParaOp(paralist[callNum - 1][i]));
			}
		}
		fprintf(OCfile, "subi $sp, $sp, %d\n", 4*(callParaNum[callNum - 1] -3));
		for (int i = callParaNum[callNum - 1] - 1; i >= 3; i--) {
			if (paralist[callNum - 1][i].category == con) {
				int temp = setReg();
				fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], paralist[callNum - 1][i].value);
				fprintf(OCfile, "sw %s, ($sp)\n", tempRegName[temp]);
				clearTempReg(temp);
			}
			else {
				fprintf(OCfile, "sw %s, ($sp)\n", searchParaOp(paralist[callNum - 1][i]));	//给每个参数分配四个字节的空间
			}
		}
	}
}

void releasePara() {
	if (callParaNum[callNum - 1] <= 3) {
		return;
	}
	else {
		fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * (callParaNum[callNum - 1] - 3));
	}
}

void outputOC() {
	fprintf(OCfile, "lui $v1, 0x1001\n");
	if (strlist.size() != 0) {
		fprintf(OCfile, ".data\n");
		for (int i = 0; i < strlist.size(); i++) {
			fprintf(OCfile, "str%d: .asciiz\"%s\"\n", i, strlist[i].c_str());
		}
		fprintf(OCfile, ".text\n");
		fprintf(OCfile, "addi $v1, $v1, %d\n", strlenSum%4 == 0?strlenSum:strlenSum+4-strlenSum%4);
	}
	fprintf(OCfile, "main:\n");
	fprintf(OCfile, "la $fp, ($sp)\n");
	for (int i = 0; i < ICnum; i++) {
		IC2OC(IClists[i]);
	}
	fprintf(OCfile, "li $v0, 10\nsyscall\n");
	clearTempReg();
	clearGlobalReg();
	for (int i = 0; i < funcnum; i++) {
		fprintf(OCfile, "%s:\n", funclists[i].name);
		int searchResult = searchId(funclists[i].name);
		if (searchResult != -1) {
			paraName = idTables[searchResult].paraName;
			paraNum = idTables[searchResult].paraNum;
			for (int j = 0; j < funclists[i].num; j++) {
				IC2OC(funclists[i].lists[j]);
			}
			clearTempReg();
			clearGlobalReg();
			clearParaReg();
			paraName = NULL;
		}
	}
}

void IC2OC(IC ic) {
	if (ic.op == add) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value + ic.rt.value);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, %s, %d\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), ic.rt.value);
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
		}
		else if (ic.rs.category != con && ic.rt.category != con) {
			fprintf(OCfile, "addu %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
			if (ic.rt.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
		}
	}
	else if (ic.op == sub) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value - ic.rt.value);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			fprintf(OCfile, "subi %s, %s, %d\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), ic.rt.value);
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
		}
		else if (ic.rs.category == con && ic.rt.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rs.value);
			fprintf(OCfile, "sub %s, %s, %s\n", tempRegName[setReg(ic.rd)], tempRegName[temp], searchOp(ic.rt));
			setNotValid(temp);
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
		else if (ic.rs.category != con && ic.rt.category != con) {
			char* tempRt = searchOp(ic.rt);
			char* tempRs = searchOp(ic.rs);
			fprintf(OCfile, "sub %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
			if (ic.rt.category == temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
	}
	else if (ic.op == mult) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value * ic.rt.value);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			fprintf(OCfile, "mul %s, %s, %d\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), ic.rt.value);
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
		}
		else if (ic.rs.category != con && ic.rt.category != con) {
			fprintf(OCfile, "mul %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
			if (ic.rt.category == temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
	}
	else if (ic.op == OP::divi) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value / ic.rt.value);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			fprintf(OCfile, "div %s, %s, %d\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), ic.rt.value);
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
		}
		else if (ic.rs.category == con && ic.rt.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rs.value);
			fprintf(OCfile, "div %s, %s, %s\n", tempRegName[setReg(ic.rd)], tempRegName[temp], searchOp(ic.rt));
			setNotValid(temp);
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
		else if (ic.rs.category != con && ic.rt.category != con) {
			fprintf(OCfile, "div %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
			if (ic.rt.category == temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
	}
	else if (ic.op == read) {
		if (ic.rd.type == Integer) {
			fprintf(OCfile, "li $v0, 5\nsyscall\n");
		}
		else if (ic.rd.type == Char) {
			fprintf(OCfile, "li $v0, 12\nsyscall\n");
		}
		int para = searchPara(ic.rd);
		if (para != -1) {
			if (para < 3)
				fprintf(OCfile, "move %s, $v0\n", paraRegName[para + 1]);
			else
				fprintf(OCfile, "sw $v0, %d($fp)\n", 4 * (para - 2));
		}
		else if (ic.rd.type == Integer) {
			if (ic.rd.addr >= 0) {
				fprintf(OCfile, "sw $v0, %d($v1)\n", ic.rd.addr);
			}
			else {
				fprintf(OCfile, "sw $v0, %d($fp)\n", ic.rd.addr);
			}
		}
		else if (ic.rd.type == Char) {
			if (ic.rd.addr >= 0) {
				fprintf(OCfile, "sb $v0, %d($v1)\n", ic.rd.addr);
			}
			else {
				fprintf(OCfile, "sb $v0, %d($fp)\n", ic.rd.addr);
			}
		}
		changed(ic.rd);
	}
	else if (ic.op == print) {
		if (ic.rd.type == Integer) {
			fprintf(OCfile, "li $v0, 1\n");
		}
		else if (ic.rd.type == Char) {
			fprintf(OCfile, "li $v0, 11\n");
		}
		else if (ic.rd.type == String) {
			fprintf(OCfile, "la $a0, %s\n", ic.rd.name);
			fprintf(OCfile, "li $v0, 4\nsyscall\n");
			return ;
		}
		if (ic.rd.category == con) {
			fprintf(OCfile, "addi $a0, $0, %d\nsyscall\n", ic.rd.value);
		}
		else {
			fprintf(OCfile, "move $a0, %s\n", searchOp(ic.rd));
			fprintf(OCfile, "syscall\n");
		}
		clearTempReg();
		clearGlobalReg();
	}
	else if (ic.op == OP::move) {
		int para = searchPara(ic.rd);
		if (para != -1) {
			char* temp;
			if (ic.rs.category == con) {
				temp = tempRegName[setReg()];
				fprintf(OCfile, "addi %s, $0, %d\n", temp, ic.rs.value);
			}
			else {
				temp = searchOp(ic.rs);
			}
			if (para < 3)
				fprintf(OCfile, "move %s, %s\n", paraRegName[para + 1], temp);
			else
				fprintf(OCfile, "sw %s, %d($fp)\n", temp, 4 * (para - 2));
		}
		else if (ic.rs.category == con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rs.value);
			if (ic.rd.type == Integer) {
				if (ic.rd.addr >= 0) {
					fprintf(OCfile, "sw %s, %d($v1)\n", tempRegName[temp], ic.rd.addr);
				}
				else {
					fprintf(OCfile, "sw %s, %d($fp)\n", tempRegName[temp], ic.rd.addr);
				}
			}
			else {
				if (ic.rd.addr >= 0) {
					fprintf(OCfile, "sb %s, %d($v1)\n", tempRegName[temp], ic.rd.addr);
				}
				else {
					fprintf(OCfile, "sb %s, %d($fp)\n", tempRegName[temp], ic.rd.addr);
				}
			}
		}
		else {
			char* regName = searchOp(ic.rs);
			if (ic.rd.type == Integer) {
				if (ic.rd.addr >= 0) {
					fprintf(OCfile, "sw %s, %d($v1)\n", regName, ic.rd.addr);
				}
				else {
					fprintf(OCfile, "sw %s, %d($fp)\n", regName, ic.rd.addr);
				}
			}
			else {
				if (ic.rd.addr >= 0) {
					fprintf(OCfile, "sb %s, %d($v1)\n", regName, ic.rd.addr);
				}
				else {
					fprintf(OCfile, "sb %s, %d($fp)\n", regName, ic.rd.addr);
				}
			}
		}
		changed(ic.rd);
		clearTempReg();
		clearGlobalReg();
	}
	else if (ic.op == array_read) {
		//rd = rs[rt]
		//rs的地址为数组的起始地址，rt为偏移量，rt的catgory可能是con, var, temp
		if (ic.rt.category == con) {
			if (ic.rs.type == Integer) {
				if (ic.rs.addr >= 0) {
					fprintf(OCfile, "lw %s, %d($v1)\n", tempRegName[setReg(ic.rd)], ic.rs.addr + ic.rt.value);
				}
				else {
					fprintf(OCfile, "lw %s, %d($fp)\n", tempRegName[setReg(ic.rd)], ic.rs.addr + ic.rt.value);
				}
			}
			else {
				if (ic.rs.addr >= 0) {
					fprintf(OCfile, "lb %s, %d($v1)\n", tempRegName[setReg(ic.rd)], ic.rs.addr + ic.rt.value);
				}
				else {
					fprintf(OCfile, "lb %s, %d($fp)\n", tempRegName[setReg(ic.rd)], ic.rs.addr + ic.rt.value);
				}
			}
		}
		else {
			char* RegNameRt = searchOp(ic.rt);
			int temp = setReg();
			if (ic.rs.addr >= 0) {
				fprintf(OCfile, "addi %s, %s, %d\n", tempRegName[temp], RegNameRt, ic.rs.addr);
				fprintf(OCfile, "addu %s, %s, $v1\n", tempRegName[temp], tempRegName[temp]);
				if (ic.rs.type == Integer) 
					fprintf(OCfile, "lw %s, (%s)\n", tempRegName[setReg(ic.rd)], tempRegName[temp]);
				else 
					fprintf(OCfile, "lb %s, (%s)\n", tempRegName[setReg(ic.rd)], tempRegName[temp]);
			}
			else {
				fprintf(OCfile, "addi %s, %s, %d\n", tempRegName[temp], RegNameRt, ic.rs.addr);
				fprintf(OCfile, "addu %s, $fp, %s\n", tempRegName[temp], tempRegName[temp]);
				if (ic.rs.type == Integer)
					fprintf(OCfile, "lw %s, (%s)\n", tempRegName[setReg(ic.rd)], tempRegName[temp]);
				else
				{
					fprintf(OCfile, "lb %s, (%s)\n", tempRegName[setReg(ic.rd)], tempRegName[temp]);
				}
			}
			if (ic.rt.category == temp) {
				setNotValid(searchReg(ic.rt));
			}
			setNotValid(temp);
		}
		changed(ic.rd);
	}
	else if (ic.op == array_write) {
		//rd[rs] = rt 
		char* RegNameRt = searchOp(ic.rt);
		if (RegNameRt == NULL) {//说明此时rt为常数
			RegNameRt = tempRegName[setReg()];
			fprintf(OCfile, "addi %s, $0, %d\n", RegNameRt, ic.rt.value);
		}
		//当数组下标为常数时
		if (ic.rs.category == con) {
			if (ic.rd.type == Integer) {
				if (ic.rd.addr >= 0) {
					fprintf(OCfile, "sw %s, %d($v1)\n", RegNameRt, ic.rd.addr + ic.rs.value);
				}
				else {
					fprintf(OCfile, "sw %s, %d($fp)\n", RegNameRt, ic.rd.addr + ic.rs.value);
				}	
			}
			else {
				if (ic.rd.addr >= 0) {
					fprintf(OCfile, "sb %s, %d($v1)\n", RegNameRt, ic.rd.addr + ic.rs.value);
				}
				else {
					fprintf(OCfile, "sb %s, %d($fp)\n", RegNameRt, ic.rd.addr + ic.rs.value);
				}
			}
		}
		else {//当数组下标需要计算时
			char* RegNameRs = searchOp(ic.rs);
			int temp = setReg();
			if (ic.rd.addr >= 0) {
				fprintf(OCfile, "addi %s, %s, %d\n", tempRegName[temp], RegNameRs, ic.rd.addr);
				fprintf(OCfile, "addu %s, %s, $v1\n", tempRegName[temp], tempRegName[temp]);
				if (ic.rd.type == Integer) {
					fprintf(OCfile, "sw %s, (%s)\n", RegNameRt, tempRegName[temp]);
				}
				else {
					fprintf(OCfile, "sb %s, (%s)\n", RegNameRt, tempRegName[temp]);
				}
			}
			else {
				fprintf(OCfile, "addi %s, %s, %d\n", tempRegName[temp], RegNameRs, ic.rd.addr);
				fprintf(OCfile, "addu %s, $fp, %s\n", tempRegName[temp], tempRegName[temp]);
				if (ic.rd.type == Integer) {
					fprintf(OCfile, "sw %s, (%s)\n", RegNameRt, tempRegName[temp]);
				}
				else {
					fprintf(OCfile, "sb %s, (%s)\n", RegNameRt, tempRegName[temp]);
				}
			}
		}
		clearTempReg();
		clearGlobalReg();
	}
	else if (ic.op == enter) {
		fprintf(OCfile, "li $v0, 11\nli $a0, 10\nsyscall\n");
	}
	else if (ic.op == slt) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value < ic.rt.value ? 1:0);
		}
		else if(ic.rs.category != con && ic.rt.category == con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rt.value);
			fprintf(OCfile, "slt %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), tempRegName[temp]);
			setNotValid(temp);
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
		}
		else if (ic.rs.category == con && ic.rt.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rs.value);
			fprintf(OCfile, "slt %s, %s, %s\n", tempRegName[setReg(ic.rd)], tempRegName[temp], searchOp(ic.rt));
			setNotValid(temp);
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
		else {
			fprintf(OCfile, "slt %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
	}
	else if (ic.op == sle) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value <= ic.rt.value ? 1 : 0);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			fprintf(OCfile, "sle %s, %s, %d\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), ic.rt.value);
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
		}
		else if (ic.rs.category == con && ic.rt.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rs.value);
			fprintf(OCfile, "sle %s, %s, %s\n", tempRegName[setReg(ic.rd)], tempRegName[temp], searchOp(ic.rt));
			setNotValid(temp);
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
		else {
			fprintf(OCfile, "sle %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
	}
	else if (ic.op == sgt) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value > ic.rt.value ? 1 : 0);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			fprintf(OCfile, "sgt %s, %s, %d\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), ic.rt.value);
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
		}
		else if (ic.rs.category == con && ic.rt.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rs.value);
			fprintf(OCfile, "sgt %s, %s, %s\n", tempRegName[setReg(ic.rd)], tempRegName[temp], searchOp(ic.rt));
			setNotValid(temp);
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
		else {
			fprintf(OCfile, "sgt %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
	}
	else if (ic.op == sge) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value >= ic.rt.value ? 1 : 0);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			fprintf(OCfile, "sge %s, %s, %d\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), ic.rt.value);
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
		}
		else if (ic.rs.category == con && ic.rt.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rs.value);
			fprintf(OCfile, "sge %s, %s, %s\n", tempRegName[setReg(ic.rd)], tempRegName[temp], searchOp(ic.rt));
			setNotValid(temp);
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
		else {
			fprintf(OCfile, "sge %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
	}
	else if (ic.op == sne) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value != ic.rt.value ? 1 : 0);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			fprintf(OCfile, "sne %s, %s, %d\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), ic.rt.value);
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
		}
		else if (ic.rs.category == con && ic.rt.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rs.value);
			fprintf(OCfile, "sne %s, %s, %s\n", tempRegName[setReg(ic.rd)], tempRegName[temp], searchOp(ic.rt));
			setNotValid(temp);
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
		else {
			fprintf(OCfile, "sne %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
	}
	else if (ic.op == seq) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value == ic.rt.value ? 1 : 0);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			fprintf(OCfile, "seq %s, %s, %d\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), ic.rt.value);
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
		}
		else if (ic.rs.category == con && ic.rt.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rs.value);
			fprintf(OCfile, "seq %s, %s, %s\n", tempRegName[setReg(ic.rd)], tempRegName[temp], searchOp(ic.rt));
			setNotValid(temp);
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
		else {
			fprintf(OCfile, "seq %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			if (ic.rs.category == temp) {
				setNotValid(searchReg(ic.rs));
			}
			if (ic.rt.category == Category::temp) {
				setNotValid(searchReg(ic.rt));
			}
		}
	}
	else if (ic.op == setlab) {
		for (int i = 0; i < globalRegNum; i++) {
			if (globalReg[i].dirty == 1) {
				int para = searchPara(globalReg[i].op);
				if (para != 0) {
					if (para < 3)
						fprintf(OCfile, "move %s, %s\n", paraRegName[para + 1], globalRegName[globalReg[i].id]);
					else
						fprintf(OCfile, "sw %s, %d($fp)\n", globalRegName[globalReg[i].id], 4 * (para - 2));
				}
				else {
					if (globalReg[i].op.type == Integer) {
						if (globalReg[i].op.addr >= 0) {
							fprintf(OCfile, "sw %s, %d($v1)\n", globalRegName[globalReg[i].id], globalReg[i].op.addr);
						}
						else {
							fprintf(OCfile, "sw %s, %d($fp)\n", globalRegName[globalReg[i].id], globalReg[i].op.addr);
						}
					}
					else {
						if (globalReg[i].op.addr >= 0) {
							fprintf(OCfile, "sb %s, %d($v1)\n", globalRegName[globalReg[i].id], globalReg[i].op.addr);
						}
						else {
							fprintf(OCfile, "sb %s, %d($fp)\n", globalRegName[globalReg[i].id], globalReg[i].op.addr);
						}
					}
				}
			}
		}
		clearGlobalReg();
		fprintf(OCfile, "%s:\n", ic.rd.name);
	}
	else if (ic.op == beq) {//目前第二个操作数均为常数0
		if (ic.rs.category == con && ic.rs.value == 0) {
			fprintf(OCfile, "beq %s, $0, %s\n", searchOp(ic.rd), ic.rt.name);
		}
		else {
			fprintf(OCfile, "beq %s, %s, %s\n", searchOp(ic.rd), searchOp(ic.rs), ic.rt.name);
		}
		clearTempReg();
		clearGlobalReg();
	}
	else if (ic.op == bne) {//目前只用于switch语句，因此不清寄存器池, 但是别人会清，因此需要将switch的判断表达式赋值给一个变量
		if (ic.rs.category == con) {
			char* temp = tempRegName[setReg()];
			fprintf(OCfile, "addi %s, $0, %d\n", temp, ic.rs.value);
			fprintf(OCfile, "bne %s, %s, %s\n", searchOp(ic.rd), temp, ic.rt.name);
		}
		clearTempReg();
		clearGlobalReg();
	}
	else if (ic.op == j) {
		fprintf(OCfile, "j %s\n", ic.rd.name);
		clearTempReg();
		for (int i = 0; i < globalRegNum; i++) {
			if (globalReg[i].dirty == 1) {
				int para = searchPara(globalReg[i].op);
				if (para != 0) {
					if (para < 3)
						fprintf(OCfile, "move %s, %s\n", paraRegName[para + 1], globalRegName[globalReg[i].id]);
					else
						fprintf(OCfile, "sw %s, %d($fp)\n", globalRegName[globalReg[i].id], 4 * (para - 2));
				}
				else {
					if (globalReg[i].op.type == Integer) {
						if (globalReg[i].op.addr >= 0) {
							fprintf(OCfile, "sw %s, %d($v1)\n", globalRegName[globalReg[i].id], globalReg[i].op.addr);
						}
						else {
							fprintf(OCfile, "sw %s, %d($fp)\n", globalRegName[globalReg[i].id], globalReg[i].op.addr);
						}
					}
					else {
						if (globalReg[i].op.addr >= 0) {
							fprintf(OCfile, "sb %s, %d($v1)\n", globalRegName[globalReg[i].id], globalReg[i].op.addr);
						}
						else {
							fprintf(OCfile, "sb %s, %d($fp)\n", globalRegName[globalReg[i].id], globalReg[i].op.addr);
						}
					}
				}
			}
		}
		clearGlobalReg();
	}
	else if (ic.op == ret) {
		fprintf(OCfile, "jr $ra\n");
		clearTempReg();
		clearGlobalReg();
	}
	else if (ic.op == retv) {
		if (ic.rd.category == con) {
			fprintf(OCfile, "addi $v0, $0, %d\n", ic.rd.value);
		}
		else {
			fprintf(OCfile, "move $v0, %s\n", searchOp(ic.rd));
		}
		fprintf(OCfile, "jr $ra\n");
		clearTempReg();
		clearGlobalReg();
	}
	else if (ic.op == call) {
		//调用前保护寄存器solid
		if (globalRegNum != 0) {
			fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * globalRegNum);
		}
		for (int k = 0; k < globalRegNum; k++) {
			fprintf(OCfile, "sw %s, %d($sp)\n", globalRegName[globalReg[k].id], 4 * k);
		}
		//调用前保护寄存器temp
		if (tempRegNum != 0) {
			fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * tempRegNum);
		}
		for (int i = 0; i < tempRegNum; i++) {
			fprintf(OCfile, "sw %s, %d($sp)\n", tempRegName[tempReg[i].id], 4 * i);
		}
		if (paraName != NULL) {
			//如果调用者是函数需要保存参数寄存器以及返回位置$ra
			if (paraNum > 0 && paraNum < 3) {
				fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * paraNum);
				for (int i = 0; i < callParaNum[callNum - 1]; i++) {
					fprintf(OCfile, "sw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
			}
			else if (paraNum > 3) {
				fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * 3);
				for (int i = 0; i < 3; i++) {
					fprintf(OCfile, "sw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
			}
			fprintf(OCfile, "subi $sp, $sp, 4\n");
			fprintf(OCfile, "sw $ra, ($sp)\n");
		}


		//调用前保存栈顶$fp
		fprintf(OCfile, "subi $sp, $sp, 4\n");
		fprintf(OCfile, "sw $fp, ($sp)\n");
		setPara();
		fprintf(OCfile, "la $fp, -4($sp)\n");	//设置新的栈帧
		fprintf(OCfile, "jal %s\n", ic.rd.name);
		fprintf(OCfile, "la $sp, 4($fp)\n");	//恢复栈帧
		releasePara();
		//恢复栈顶$fp
		fprintf(OCfile, "lw $fp, ($sp)\n");
		fprintf(OCfile, "addi $sp, $sp, 4\n");
		if (paraName != NULL) {
			//调用结束后如果调用者是函数恢复返回位置$ra
			fprintf(OCfile, "lw $ra, ($sp)\n");
			fprintf(OCfile, "addi $sp, $sp, 4\n");
			//恢复参数寄存器
			if (paraNum > 0 && paraNum < 3) {
				for (int i = 0; i < callParaNum[callNum - 1]; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * paraNum);
			}
			else if (paraNum > 3) {
				for (int i = 0; i < 3; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * 3);
			}
		}
		//恢复temp
		for (int i = 0; i < tempRegNum; i++) {
			fprintf(OCfile, "lw %s, %d($sp)\n", tempRegName[tempReg[i].id], 4 * i);
		}
		if (tempRegNum != 0)
			fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * tempRegNum);
		//恢复solid
		for (int i = 0; i < globalRegNum; i++) {
			fprintf(OCfile, "lw %s, %d($sp)\n", globalRegName[globalReg[i].id], 4 * i);
		}
		if (globalRegNum != 0)
			fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * globalRegNum);
		callParaNum[callNum - 1] = 0;
		callNum--;
		}
	else if (ic.op == callv) {
		//调用前保护寄存器solid
		if (globalRegNum != 0) {
			fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * globalRegNum);
		}
		for (int k = 0; k < globalRegNum; k++) {
			fprintf(OCfile, "sw %s, %d($sp)\n", globalRegName[globalReg[k].id], 4 * k);
		}
		//调用前保护寄存器temp
		if (tempRegNum != 0) {
			fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * tempRegNum);
		}
		for (int i = 0; i < tempRegNum; i++) {
			fprintf(OCfile, "sw %s, %d($sp)\n", tempRegName[tempReg[i].id], 4 * i);
		}
		if (paraName != NULL) {
			//如果调用者是函数需要保存参数寄存器以及返回位置$ra
			if (paraNum > 0 && paraNum < 3) {
				fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * paraNum);
				for (int i = 0; i < callParaNum[callNum - 1]; i++) {
					fprintf(OCfile, "sw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
			}
			else if (paraNum > 3) {
				fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * 3);
				for (int i = 0; i < 3; i++) {
					fprintf(OCfile, "sw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
			}
			fprintf(OCfile, "subi $sp, $sp, 4\n");
			fprintf(OCfile, "sw $ra, ($sp)\n");
		}
		

		//调用前保存栈顶$fp
		fprintf(OCfile, "subi $sp, $sp, 4\n");
		fprintf(OCfile, "sw $fp, ($sp)\n");
		setPara();
		fprintf(OCfile, "la $fp, -4($sp)\n");	//设置新的栈帧
		fprintf(OCfile, "jal %s\n", ic.rd.name);
		fprintf(OCfile, "la $sp, 4($fp)\n");	//恢复栈帧
		releasePara();
		//恢复栈顶$fp
		fprintf(OCfile, "lw $fp, ($sp)\n");
		fprintf(OCfile, "addi $sp, $sp, 4\n");
		if (paraName != NULL) {
			//调用结束后如果调用者是函数恢复返回位置$ra
			fprintf(OCfile, "lw $ra, ($sp)\n");
			fprintf(OCfile, "addi $sp, $sp, 4\n");
			//恢复参数寄存器
			if (paraNum > 0 && paraNum < 3) {
				for (int i = 0; i < callParaNum[callNum - 1]; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * paraNum);
			}
			else if (paraNum > 3) {
				for (int i = 0; i < 3; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * 3);
			}
		}
		//恢复temp
		for (int i = 0; i < tempRegNum; i++) {
			fprintf(OCfile, "lw %s, %d($sp)\n", tempRegName[tempReg[i].id], 4 *i);
		}
		if (tempRegNum != 0)
			fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * tempRegNum);
		//恢复solid
		for (int i = 0; i < globalRegNum; i++) {
			fprintf(OCfile, "lw %s, %d($sp)\n", globalRegName[globalReg[i].id], 4 * i);
		}
		if (globalRegNum != 0)
			fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * globalRegNum);
		callParaNum[callNum - 1] = 0;
		callNum--;
		fprintf(OCfile, "move %s, $v0\n", tempRegName[setReg(ic.rs)]);
	}
	else if (ic.op == parameter) {
		//将参数按照规则进行保存
		addPara(ic.rd);
	}
	else if (ic.op == alloc) {
		int addr = ic.rd.value;
		if (addr != 0) {
			if (addr % 4 != 0) {
				addr = addr - (4 + addr % 4);
			}
			fprintf(OCfile, "addi $sp, $fp, %d\n", addr);
		}
	}
	else if (ic.op == OP::end) {
		fprintf(OCfile, "li $v0, 10\nsyscall\n");
	}
	else if (ic.op == OP::parabegin) {
		callNum++;
	}
}

//还可以进行的优化：
//1. 全局寄存器的分配策略，使用valid
//2. 乘法除法对特殊值（比如0、1、2的幂次等）