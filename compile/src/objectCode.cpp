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

int label4div = 0; //专门为除法使用的标签

int solidStack[10][8];
int tempStack[10][10];
Reg solidRegStack[10][8];
Reg tempRegStack[10][10];

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
					return paraRegName[i + 1];
				}
				else {
					int t = setReg(op);
					fprintf(OCfile, "lw %s, %d($fp)\n", globalRegName[t], 4*(paraNum - i));//
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
			globalReg[result].dirty = 0;
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
					if (paraReg[i + 1].dirty == 0) {
						return paraRegName[i + 1];
					}
					else {	//函数寄存器为脏位当且仅当发生了嵌套的函数调用时
						int temp = setReg();
						int loc = 0;
						for (int j = 0; j < callNum-1; j++) {
							loc += callParaNum[j] > 3 ? 3 : callParaNum[j] + 1;
						}
						fprintf(OCfile, "lw %s, %d($sp)\n", tempRegName[temp], 4 * (loc + i));//保存的参数顺序是
						setNotValid(Category::temp, temp);
						return tempRegName[temp];
					}
				}
				else {
					int temp = setReg(op);
					fprintf(OCfile, "lw %s, %d($fp)\n", globalRegName[temp], 4 * (paraNum - i));//
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
			globalReg[result].dirty = 0;
		}
		return globalRegName[result];
	}
	return NULL;
}

int pow2(int num) {
	if ((num&(num-1)) != 0) {
		return -1;
	}
	int mi = 0;
	while (num != 1) {
		num = num / 2;
		mi++;
	}
	return mi;
}

char* getLabel4div() {
	char* s = (char*)malloc(10);
	sprintf(s, "LABEL_DIV_%d", label4div++);
	return s;
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
			fprintf(OCfile, "sw $ra, ($fp)\n");
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
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		else if (ic.rs.category != con && ic.rt.category != con) {
			fprintf(OCfile, "addu %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			setNotValid(ic.rs.category, searchReg(ic.rs));
			setNotValid(ic.rt.category, searchReg(ic.rt));
		}
	}
	else if (ic.op == sub) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value - ic.rt.value);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			fprintf(OCfile, "subi %s, %s, %d\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), ic.rt.value);
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		else if (ic.rs.category == con && ic.rt.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rs.value);
			fprintf(OCfile, "sub %s, %s, %s\n", tempRegName[setReg(ic.rd)], tempRegName[temp], searchOp(ic.rt));
			setNotValid(Category::temp, temp);
			setNotValid(ic.rt.category, searchReg(ic.rt));
		}
		else if (ic.rs.category != con && ic.rt.category != con) {
			char* tempRt = searchOp(ic.rt);
			char* tempRs = searchOp(ic.rs);
			fprintf(OCfile, "sub %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			setNotValid(ic.rs.category, searchReg(ic.rs));
			setNotValid(ic.rt.category, searchReg(ic.rt));
		}
	}
	else if (ic.op == mult) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value * ic.rt.value);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			int RdTempReg = setReg(ic.rd);
			if (ic.rt.value == 0) {
				fprintf(OCfile, "move %s $0\n", tempRegName[RdTempReg]);
				setNotValid(ic.rs.category, searchReg(ic.rs));
				return;
			}
			else if (ic.rt.value < 0) {
				fprintf(OCfile, "negu %s %s\n", tempRegName[RdTempReg], searchOp(ic.rs));
				ic.rt.value = -ic.rt.value;
				if (ic.rt.value == 1) {
					return;
				}
				else {
					int pow = pow2(ic.rt.value);
					if (pow != -1) {
						fprintf(OCfile, "sll %s, %s, %d\n", tempRegName[RdTempReg], tempRegName[RdTempReg], pow);
					}
					else {
						fprintf(OCfile, "mul %s, %s, %d\n", tempRegName[RdTempReg], tempRegName[RdTempReg], ic.rt.value);
					}
				}
			}
			else {
				if (ic.rt.value == 1) {
					fprintf(OCfile, "move %s %s\n", tempRegName[RdTempReg], searchOp(ic.rs));
				}
				else{
					int pow = pow2(ic.rt.value);
					if (pow != -1) {
						fprintf(OCfile, "sll %s, %s, %d\n", tempRegName[RdTempReg], searchOp(ic.rs), pow);
					}
					else {
						fprintf(OCfile, "mul %s, %s, %d\n", tempRegName[RdTempReg], searchOp(ic.rs), ic.rt.value);
					}
				}
			}
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		else if (ic.rs.category != con && ic.rt.category != con) {
			fprintf(OCfile, "mul %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			setNotValid(ic.rs.category, searchReg(ic.rs));
			setNotValid(ic.rt.category, searchReg(ic.rt));
		}
	}
	else if (ic.op == OP::divi) {
		if (ic.rs.category == con && ic.rt.category == con) {
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[setReg(ic.rd)], ic.rs.value / ic.rt.value);
		}
		else if (ic.rs.category != con && ic.rt.category == con) {
			if (ic.rt.value == 1) {
				fprintf(OCfile, "move %s %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs));
			}
			else if (ic.rt.value == -1) {
				fprintf(OCfile, "negu %s %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs));
			}
			else if (ic.rt.value == 2) {
				int RdTempReg = setReg(ic.rd);
				char* RsRegName = searchOp(ic.rs);
				fprintf(OCfile, "sra %s, %s, 31\n", tempRegName[RdTempReg], RsRegName);
				fprintf(OCfile, "sub %s, %s, %s\n", tempRegName[RdTempReg], RsRegName, tempRegName[RdTempReg]);
				fprintf(OCfile, "sra %s, %s, 1\n", tempRegName[RdTempReg], tempRegName[RdTempReg]);
			}
			else if (ic.rt.value == -2) {
				int RdTempReg = setReg(ic.rd);
				char* RsRegName = searchOp(ic.rs);
				fprintf(OCfile, "negu %s %s\n", tempRegName[RdTempReg], RsRegName);
				fprintf(OCfile, "sra %s, %s, 31\n", tempRegName[RdTempReg], tempRegName[RdTempReg]);
				fprintf(OCfile, "sub %s, %s, %s\n", tempRegName[RdTempReg], tempRegName[RdTempReg], tempRegName[RdTempReg]);
				fprintf(OCfile, "sra %s, %s, 1\n", tempRegName[RdTempReg], tempRegName[RdTempReg]);
			}
			else {
				int RdTempReg = setReg(ic.rd);
				if (ic.rt.value > 0) {
					int pow = pow2(ic.rt.value);
					if (pow != -1) {
						char* RsRegName = searchOp(ic.rs);
						char* label1 = getLabel4div();
						char* label2 = getLabel4div();
						fprintf(OCfile, "bge %s, $0, %s\n", RsRegName, label1);
						fprintf(OCfile, "negu %s, %s\n", tempRegName[RdTempReg], RsRegName);
						fprintf(OCfile, "sra %s, %s, %d\n", tempRegName[RdTempReg], tempRegName[RdTempReg], pow);
						fprintf(OCfile, "negu %s, %s\n", tempRegName[RdTempReg], tempRegName[RdTempReg]);
						fprintf(OCfile, "j %s\n", label2);
						fprintf(OCfile, "%s:\n", label1);
						fprintf(OCfile, "sra %s, %s, %d\n", tempRegName[RdTempReg], RsRegName, pow);
						fprintf(OCfile, "%s:\n", label2);
					}
					else {
						fprintf(OCfile, "div %s, %s, %d\n", tempRegName[RdTempReg], searchOp(ic.rs), ic.rt.value);
						setNotValid(ic.rs.category, searchReg(ic.rs));
					}
				}
				else {
					int pow = pow2(-ic.rt.value);
					if (pow != -1) {
						char* RsRegName = searchOp(ic.rs);
						char* label1 = getLabel4div();
						char* label2 = getLabel4div();
						fprintf(OCfile, "ble %s, $0, %s\n", RsRegName, label1);
						fprintf(OCfile, "negu %s, %s\n", tempRegName[RdTempReg], RsRegName);
						fprintf(OCfile, "sra %s, %s, %d\n", tempRegName[RdTempReg], tempRegName[RdTempReg], pow);
						fprintf(OCfile, "j %s\n", label2);
						fprintf(OCfile, "%s:\n", label1);
						fprintf(OCfile, "negu %s, %s\n", tempRegName[RdTempReg], RsRegName);
						fprintf(OCfile, "sra %s, %s, %d\n", tempRegName[RdTempReg], tempRegName[RdTempReg], pow);
						fprintf(OCfile, "%s:\n", label2);
					}
					else {
						fprintf(OCfile, "div %s, %s, %d\n", tempRegName[RdTempReg], searchOp(ic.rs), ic.rt.value);
						setNotValid(ic.rs.category, searchReg(ic.rs));
					}
				}
			}
		}
		else if (ic.rs.category == con && ic.rt.category != con) {
			if (ic.rs.value == 0) {
				fprintf(OCfile, "move %s, $0\n", tempRegName[setReg(ic.rd)]);
			}
			else {
				int temp = setReg();
				fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rs.value);
				fprintf(OCfile, "div %s, %s, %s\n", tempRegName[setReg(ic.rd)], tempRegName[temp], searchOp(ic.rt));
				setNotValid(Category::temp, temp);
				setNotValid(ic.rt.category, searchReg(ic.rt));
			}
		}
		else if (ic.rs.category != con && ic.rt.category != con) {
			fprintf(OCfile, "div %s, %s, %s\n", tempRegName[setReg(ic.rd)], searchOp(ic.rs), searchOp(ic.rt));
			setNotValid(ic.rs.category, searchReg(ic.rs));
			setNotValid(ic.rt.category, searchReg(ic.rt));
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
			setNotValid(ic.rs.category, searchReg(ic.rt));
			setNotValid(Category::temp, temp);
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
	}
	else if (ic.op == enter) {
		fprintf(OCfile, "li $v0, 11\nli $a0, 10\nsyscall\n");
	}
	else if (ic.op == bge) {
		if (ic.rd.category == con && ic.rs.category == con) {
			if (ic.rd.value >= ic.rs.value)
				fprintf(OCfile, "j %s\n", ic.rt.name);
		}
		else if(ic.rd.category != con && ic.rs.category == con) {
			fprintf(OCfile, "bge %s, %d, %s\n", searchOp(ic.rd), ic.rs.value, ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
		}
		else if (ic.rd.category == con && ic.rs.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rd.value);
			fprintf(OCfile, "bge %s, %s, %s\n", tempRegName[temp], searchOp(ic.rs), ic.rt.name);
			setNotValid(Category::temp, temp);
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		else {
			fprintf(OCfile, "bge %s, %s, %s\n", searchOp(ic.rd), searchOp(ic.rs), ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		clearTempReg();
	}
	else if (ic.op == bgt) {
		if (ic.rd.category == con && ic.rs.category == con) {
			if (ic.rd.value > ic.rs.value)
				fprintf(OCfile, "j %s\n", ic.rt.name);
		}
		else if (ic.rd.category != con && ic.rs.category == con) {
			fprintf(OCfile, "bgt %s, %d, %s\n", searchOp(ic.rd), ic.rs.value, ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
		}
		else if (ic.rd.category == con && ic.rs.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rd.value);
			fprintf(OCfile, "bgt %s, %s, %s\n", tempRegName[temp], searchOp(ic.rs), ic.rt.name);
			setNotValid(Category::temp, temp);
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		else {
			fprintf(OCfile, "bgt %s, %s, %s\n", searchOp(ic.rd), searchOp(ic.rs), ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		clearTempReg();
	}
	else if (ic.op == ble) {
		if (ic.rd.category == con && ic.rs.category == con) {
			if (ic.rd.value <= ic.rs.value)
				fprintf(OCfile, "j %s\n", ic.rt.name);
		}
		else if (ic.rd.category != con && ic.rs.category == con) {
			fprintf(OCfile, "ble %s, %d, %s\n", searchOp(ic.rd), ic.rs.value, ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
		}
		else if (ic.rd.category == con && ic.rs.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rd.value);
			fprintf(OCfile, "ble %s, %s, %s\n", tempRegName[temp], searchOp(ic.rs), ic.rt.name);
			setNotValid(Category::temp, temp);
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		else {
			fprintf(OCfile, "ble %s, %s, %s\n", searchOp(ic.rd), searchOp(ic.rs), ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		clearTempReg();
	}
	else if (ic.op == blt) {
		if (ic.rd.category == con && ic.rs.category == con) {
			if (ic.rd.value < ic.rs.value)
				fprintf(OCfile, "j %s\n", ic.rt.name);
		}
		else if (ic.rd.category != con && ic.rs.category == con) {
			fprintf(OCfile, "blt %s, %d, %s\n", searchOp(ic.rd), ic.rs.value, ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
		}
		else if (ic.rd.category == con && ic.rs.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rd.value);
			fprintf(OCfile, "blt %s, %s, %s\n", tempRegName[temp], searchOp(ic.rs), ic.rt.name);
			setNotValid(Category::temp, temp);
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		else {
			fprintf(OCfile, "blt %s, %s, %s\n", searchOp(ic.rd), searchOp(ic.rs), ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		clearTempReg();
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
	else if (ic.op == beq) {
		if (ic.rd.category == con && ic.rs.category == con) {
			if (ic.rd.value == ic.rs.value)
				fprintf(OCfile, "j %s\n", ic.rt.name);
		}
		else if (ic.rd.category != con && ic.rs.category == con) {
			fprintf(OCfile, "beq %s, %d, %s\n", searchOp(ic.rd), ic.rs.value, ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
		}
		else if (ic.rd.category == con && ic.rs.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rd.value);
			fprintf(OCfile, "beq %s, %s, %s\n", tempRegName[temp], searchOp(ic.rs), ic.rt.name);
			setNotValid(Category::temp, temp);
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		else {
			fprintf(OCfile, "beq %s, %s, %s\n", searchOp(ic.rd), searchOp(ic.rs), ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		clearTempReg();
	}
	else if (ic.op == bne) {
		if (ic.rd.category == con && ic.rs.category == con) {
			if (ic.rd.value != ic.rs.value)
				fprintf(OCfile, "j %s\n", ic.rt.name);
		}
		else if (ic.rd.category != con && ic.rs.category == con) {
			fprintf(OCfile, "bne %s, %d, %s\n", searchOp(ic.rd), ic.rs.value, ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
		}
		else if (ic.rd.category == con && ic.rs.category != con) {
			int temp = setReg();
			fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rd.value);
			fprintf(OCfile, "bne %s, %s, %s\n", tempRegName[temp], searchOp(ic.rs), ic.rt.name);
			setNotValid(Category::temp, temp);
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		else {
			fprintf(OCfile, "bne %s, %s, %s\n", searchOp(ic.rd), searchOp(ic.rs), ic.rt.name);
			setNotValid(ic.rd.category, searchReg(ic.rd));
			setNotValid(ic.rs.category, searchReg(ic.rs));
		}
		clearTempReg();
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
		fprintf(OCfile, "lw $ra, ($fp)\n");
		fprintf(OCfile, "jr $ra\n");
		clearTempReg();
	}
	else if (ic.op == retv) {
		if (ic.rd.category == con) {
			fprintf(OCfile, "addi $v0, $0, %d\n", ic.rd.value);
		}
		else {
			fprintf(OCfile, "move $v0, %s\n", searchOp(ic.rd));
		}
		fprintf(OCfile, "lw $ra, ($fp)\n");
		fprintf(OCfile, "jr $ra\n");
		clearTempReg();
	}
	else if (ic.op == call) {
		clearGlobalReg();//清除参数设置阶段使用到的全局寄存器
		fprintf(OCfile, "la $fp, -4($sp)\n");	//设置新的栈帧
		fprintf(OCfile, "jal %s\n", ic.rd.name);
		fprintf(OCfile, "la $sp, 4($fp)\n");	//恢复栈帧
		//释放参数
		if (callParaNum[callNum - 1] > 3)
			fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * (callParaNum[callNum - 1] - 3));
		if (paraNum != 0 && callNum == 1) {
			//恢复参数寄存器
			if (paraNum > 0 && paraNum < 3) {
				for (int i = 0; i < paraNum; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
					paraReg[i + 1].dirty = 0;
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * paraNum);
			}
			else if (paraNum >= 3) {
				for (int i = 0; i < 3; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
					paraReg[i + 1].dirty = 0;
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * 3);
			}
		}
		else if (callNum > 1) {
			if (callParaNum[callNum - 2] > 0 && callParaNum[callNum - 2] < 3) {
				for (int i = 0; i < callParaNum[callNum - 2]; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * callParaNum[callNum - 2]);
			}
			else if (callParaNum[callNum - 2] >= 3) {
				for (int i = 0; i < 3; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * 3);
			}
		}
		//恢复栈顶$fp
		fprintf(OCfile, "lw $fp, ($sp)\n");
		fprintf(OCfile, "addi $sp, $sp, 4\n");
		//恢复temp
		int tempNum = 0;
		for (int i = 0; i < 10; i++) {
			if (tempStack[callNum - 1][i] == 1) {
				tempNum++;
			}
		}
		if (tempNum != 0) {
			int count = 0;
			for (int i = 0; i < 10; i++) {
				if (tempStack[callNum - 1][i] == 1) {
					fprintf(OCfile, "lw %s, %d($sp)\n", tempRegName[tempRegStack[callNum - 1][i].id], 4 * count++);
					tempReg[i] = tempRegStack[callNum - 1][i];
					tempRegNum = i + 1;
				}
			}
			fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * tempNum);
		}
		//恢复solid
		int solidNum = 0;
		for (int i = 0; i < 8; i++) {
			if (solidStack[callNum - 1][i] == 1) {
				solidNum++;
			}
		}
		if (solidNum != 0) {
			int count = 0;
			for (int i = 0; i < 8; i++) {
				if (solidStack[callNum - 1][i] == 1) {
					fprintf(OCfile, "lw %s, %d($sp)\n", globalRegName[solidRegStack[callNum - 1][i].id], 4 * count++);
					globalReg[i] = solidRegStack[callNum - 1][i];
					globalRegNum = i + 1;
				}
			}
			fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * solidNum);
		}
		callParaNum[callNum - 1] = 0;
		callNum--;
	}
	else if (ic.op == callv) {
		clearGlobalReg();//清除参数设置阶段使用到的全局寄存器
		fprintf(OCfile, "la $fp, -4($sp)\n");	//设置新的栈帧
		fprintf(OCfile, "jal %s\n", ic.rd.name);
		fprintf(OCfile, "la $sp, 4($fp)\n");	//恢复栈帧
		//释放参数
		if (callParaNum[callNum - 1] > 3) 
			fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * (callParaNum[callNum - 1] - 3));
		if (paraNum != 0 && callNum == 1) {
			//恢复参数寄存器
			if (paraNum > 0 && paraNum < 3) {
				for (int i = 0; i < paraNum; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
					paraReg[i + 1].dirty = 0;
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * paraNum);
			}
			else if (paraNum >= 3) {
				for (int i = 0; i < 3; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
					paraReg[i + 1].dirty = 0;
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * 3);
			}
		}
		else if (callNum > 1) {
			if (callParaNum[callNum - 2] > 0 && callParaNum[callNum - 2] < 3) {
				for (int i = 0; i < callParaNum[callNum - 2]; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * callParaNum[callNum - 2]);
			}
			else if (callParaNum[callNum - 2] >= 3) {
				for (int i = 0; i < 3; i++) {
					fprintf(OCfile, "lw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
				fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * 3);
			}
		}
		//恢复栈顶$fp
		fprintf(OCfile, "lw $fp, ($sp)\n");
		fprintf(OCfile, "addi $sp, $sp, 4\n");
		//恢复temp
		int tempNum = 0;
		for (int i = 0; i < 10; i++) {
			if (tempStack[callNum - 1][i] == 1) {
				tempNum++;
			}
		}
		if (tempNum != 0) {
			int count = 0;
			for (int i = 0; i < 10; i++) {
				if (tempStack[callNum - 1][i] == 1) {
					fprintf(OCfile, "lw %s, %d($sp)\n", tempRegName[tempRegStack[callNum - 1][i].id], 4 * count++);
					tempReg[i] = tempRegStack[callNum - 1][i];
					tempRegNum = i+1;
				}
			}
			fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * tempNum);
		}
		//恢复solid
		int solidNum = 0;
		for (int i = 0; i < 8; i++) {
			if (solidStack[callNum - 1][i] == 1) {
				solidNum++;
			}
		}
		if (solidNum != 0) {
			int count = 0;
			for (int i = 0; i < 8; i++) {
				if (solidStack[callNum - 1][i] == 1) {
					fprintf(OCfile, "lw %s, %d($sp)\n", globalRegName[solidRegStack[callNum - 1][i].id], 4 * count++);
					globalReg[i] = solidRegStack[callNum - 1][i];
					globalRegNum = i+1;
				}
			}
			fprintf(OCfile, "addi $sp, $sp, %d\n", 4 * solidNum);
		}
		callParaNum[callNum - 1] = 0;
		callNum--;
		fprintf(OCfile, "move %s, $v0\n", tempRegName[setReg(ic.rs)]);
	}
	else if (ic.op == parameter) {
		if (callParaNum[callNum - 1] < 3) {
			if (ic.rd.category == con) {
				fprintf(OCfile, "addi $a%d, $0, %d\n", callParaNum[callNum - 1] + 1, ic.rd.value);
			}
			else {
				fprintf(OCfile, "move $a%d, %s\n", callParaNum[callNum - 1] + 1, searchParaOp(ic.rd));
			}
			paraReg[callParaNum[callNum - 1] + 1].dirty = 1;
		}
		else {
			fprintf(OCfile, "subi $sp, $sp, %d\n", 4);
			if (ic.rd.category == con) {
				int temp = setReg();
				fprintf(OCfile, "addi %s, $0, %d\n", tempRegName[temp], ic.rd.value);
				fprintf(OCfile, "sw %s, ($sp)\n", tempRegName[temp]);
				clearTempReg(temp);
			}
			else {
				fprintf(OCfile, "sw %s, ($sp)\n", searchParaOp(ic.rd));	//给每个参数分配四个字节的空间
			}
		}
		callParaNum[callNum - 1]++;
		clearTempReg();
	}
	else if (ic.op == alloc) {
		int addr = ic.rd.value - 4;
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
		for (int i = 0; i < 8; i++) {
			solidStack[callNum - 1][i] = 0;
		}
		//调用前保护寄存器solid
		int solidNum = 0;
		for (int i = 0; i < globalRegNum; i++) {
			if (globalReg[i].valid == 1) {
				solidNum++;
			}
		}
		if (solidNum != 0) {
			fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * solidNum);
			int count = 0;
			for (int i = 0; i < globalRegNum; i++) {
				if (globalReg[i].valid == 1) {
					fprintf(OCfile, "sw %s, %d($sp)\n", globalRegName[globalReg[i].id], 4*count++);
					solidStack[callNum-1][i] = 1;
					solidRegStack[callNum - 1][i] = globalReg[i];
				}
			}
		}
		for (int i = 0; i < 10; i++) {
			tempStack[callNum - 1][i] = 0;
		}
		//调用前保护寄存器temp
		int tempNum = 0;
		for (int i = 0; i < tempRegNum; i++) {
			if (tempReg[i].valid == 1) {
				tempNum++;
			}
		}
		if (tempNum != 0) {
			fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * tempNum);
			int count = 0;
			for (int i = 0; i < tempRegNum; i++) {
				if (tempReg[i].valid == 1) {
					fprintf(OCfile, "sw %s, %d($sp)\n", tempRegName[tempReg[i].id], 4*count++);
					tempStack[callNum - 1][i] = 1;
					tempRegStack[callNum - 1][i] = tempReg[i];
				}
			}
		}
		clearTempReg();
		clearGlobalReg();//以免重复保护
		//调用前保存栈顶$fp
		fprintf(OCfile, "subi $sp, $sp, 4\n");
		fprintf(OCfile, "sw $fp, ($sp)\n");
		if (paraNum != 0 && callNum == 1) {
			//如果调用者是函数需要保存参数寄存器
			if (paraNum > 0 && paraNum < 3) {
				fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * paraNum);
				for (int i = 0; i < paraNum; i++) {
					fprintf(OCfile, "sw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
			}
			else if (paraNum >= 3) {
				fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * 3);
				for (int i = 0; i < 3; i++) {
					fprintf(OCfile, "sw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
			}
		}
		else if(callNum > 1) {
			if (callParaNum[callNum-2] > 0 && callParaNum[callNum - 2] < 3) {
				fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * callParaNum[callNum - 2]);
				for (int i = 0; i < callParaNum[callNum - 2]; i++) {
					fprintf(OCfile, "sw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
			}
			else if(callParaNum[callNum - 2] >= 3) {
				fprintf(OCfile, "subi $sp, $sp, %d\n", 4 * 3);
				for (int i = 0; i < 3; i++) {
					fprintf(OCfile, "sw %s, %d($sp)\n", paraRegName[i + 1], 4 * i);
				}
			}
		}
	}
}