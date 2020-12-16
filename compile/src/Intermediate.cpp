#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"Intermediate.h"


IC IClists[4096];
int ICnum = 0;
int ICNumTemp = 0;
int tempNum = 0;
int solidNum = 0;
int labNum = 0;
ICFUNC funclists[256];
int funcnum = 0;
IC* lists = IClists;
char ICName[][15] = {"add", "sub", "mult", "divi", "read", "print", "move", "enter", "array_read", "array_write",
	"slt", "sle", "sgt", "sge", "sne", "seq", "setlab", "beq", "bne", "j", "ret", "retv", "call", "callv", "parameter", "alloc"};

Operand* newOperand(IdTable idtable) {
	Operand* Tp = (Operand*)malloc(sizeof(Operand));
	Tp->name = idtable.name;
	Tp->category = idtable.category;
	Tp->type = idtable.type;
	if (idtable.category == var || idtable.category == array) {
		Tp->addr = idtable.address;
		Tp->level = idtable.level;
	}
	else if (idtable.category == con) {
		Tp->value = idtable.value;
	}
	return Tp;
}

Operand* newOperand(IdTable idtable, int offset) {
	Operand* Tp = (Operand*)malloc(sizeof(Operand));
	Tp->name = idtable.name;
	Tp->category = idtable.category;
	Tp->type = idtable.type;
	Tp->level = idtable.level;
	Tp->addr = idtable.address + offset * (idtable.type == Integer ? 4 : 1);
	return Tp;
}

Operand* newOperand(int integer, Type type) {
	Operand* Tp = (Operand*)malloc(sizeof(Operand));
	Tp->name = NULL;
	Tp->category = con;
	Tp->type = type;
	Tp->value = integer;
	if (type == String) {
		Tp->name = (char*)malloc(10*sizeof(char));
		sprintf(Tp->name, "str%d", integer);
	}
	return Tp;
}

Operand* tempOperand() {
	Operand* Tp = (Operand*)malloc(sizeof(Operand));
	Tp->category = temp;
	Tp->type = Integer;
	Tp->name = (char*)malloc(10 * sizeof(char));
	sprintf(Tp->name, "TEMP_%d", tempNum++);
	return Tp;
}

Operand* getLab() {
	//TODO
	Operand* lab = (Operand*)malloc(sizeof(Operand));
	lab->name = (char*)malloc(10 * sizeof(char));
	sprintf(lab->name, "LABEL%d", labNum++);
	return lab;
}

void FunctionDefBegin(char* name) {
	funclists[funcnum].name = name;
	funclists[funcnum].num = 0;
	lists = funclists[funcnum].lists;
	ICNumTemp = ICnum;
	ICnum = 0;
}

void FunctionDefEnd() {
	lists = IClists;
	funclists[funcnum].num = ICnum;
	ICnum = ICNumTemp;
	funcnum++;
}

void clearTemp()
{
	tempNum = 0;
}

void ICAdd(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = add;
	if (rs.category == con && rt.category != con) {
		lists[ICnum].rd = rd;
		lists[ICnum].rs = rt;
		lists[ICnum].rt = rs;
	}
	else {
		lists[ICnum].rd = rd;
		lists[ICnum].rs = rs;
		lists[ICnum].rt = rt;
	}
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICSub(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = sub;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICMult(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = mult;
	if (rs.category == con && rt.category != con) {
		lists[ICnum].rd = rd;
		lists[ICnum].rs = rt;
		lists[ICnum].rt = rs;
	}
	else {
		lists[ICnum].rd = rd;
		lists[ICnum].rs = rs;
		lists[ICnum].rt = rt;
	}
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICDiv(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = OP::divi;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICRead(Operand rd) {
	lists[ICnum].op = read;
	lists[ICnum].rd = rd;
	lists[ICnum].opNum = 1;
	ICnum++;
}

void ICPrint(Operand rd) {
	lists[ICnum].op = print;
	lists[ICnum].rd = rd;
	lists[ICnum].opNum = 1;
	ICnum++;
}

void ICMove(Operand rd, Operand rs) {
	lists[ICnum].op = OP::move;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].opNum = 2;
	ICnum++;
}

void ICenter() {
	lists[ICnum].op = enter;
	lists[ICnum].opNum = 0;
	ICnum++;
}

void ICArrayRead(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = OP::array_read;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICArrayWrite(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = OP::array_write;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICSlt(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = OP::slt;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICSle(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = OP::sle;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICSgt(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = OP::sgt;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICSge(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = OP::sge;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICSne(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = OP::sne;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICSeq(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = OP::seq;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICSetLab(Operand rd) {
	lists[ICnum].op = setlab;
	lists[ICnum].rd = rd;
	lists[ICnum].opNum = 1;
	ICnum++;
}

void ICBeq(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = OP::beq;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICBne(Operand rd, Operand rs, Operand rt) {
	lists[ICnum].op = OP::bne;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].rt = rt;
	lists[ICnum].opNum = 3;
	ICnum++;
}

void ICJ(Operand rd) {
	lists[ICnum].op = OP::j;
	lists[ICnum].rd = rd;
	lists[ICnum].opNum = 1;
	ICnum++;
}

void ICRET() {
	lists[ICnum].op = OP::ret;
	ICnum++;
}

void ICRETV(Operand rd) {
	lists[ICnum].op = OP::retv;
	lists[ICnum].rd = rd;
	lists[ICnum].opNum = 1;
	ICnum++;
}

void ICCall(Operand rd) {
	lists[ICnum].op = OP::call;
	lists[ICnum].rd = rd;
	ICnum++;
}

void ICCallV(Operand rd, Operand rs) {
	lists[ICnum].op = OP::callv;
	lists[ICnum].rd = rd;
	lists[ICnum].rs = rs;
	lists[ICnum].opNum = 2;
	ICnum++;
}

void ICPara(Operand rd) {
	lists[ICnum].op = OP::parameter;
	lists[ICnum].rd = rd;
	lists[ICnum].opNum = 1;
	ICnum++;
}

void ICAlloc(Operand rd) {
	lists[ICnum].op = OP::alloc;
	lists[ICnum].rd = rd;
	ICnum++;
}

void ICEnd() {
	lists[ICnum].op = OP::end;
	ICnum++;
}

void ICParaBegin() {
	lists[ICnum].op = OP::parabegin;
	ICnum++;
}

void printIC() {
	FILE* ICFile = fopen("testfile3_18373727.txt", "w");
	for (int i = 0; i < ICnum; i++) {
		fprintf(ICFile, "%s ", ICName[IClists[i].op]);
		if (IClists[i].opNum == 3) {
			if (IClists[i].rd.name != NULL) {
				fprintf(ICFile, "%s ", IClists[i].rd.name);
			}
			else {
				fprintf(ICFile, "%d ", IClists[i].rd.value);
			}
			if (IClists[i].rs.name != NULL) {
				fprintf(ICFile, "%s ", IClists[i].rs.name);
			}
			else {
				fprintf(ICFile, "%d ", IClists[i].rs.value);
			}
			if (IClists[i].rt.name != NULL) {
				fprintf(ICFile, "%s ", IClists[i].rt.name);
			}
			else {
				fprintf(ICFile, "%d ", IClists[i].rt.value);
			}
		}
		else if (IClists[i].opNum == 2) {
			if (IClists[i].rd.name != NULL) {
				fprintf(ICFile, "%s ", IClists[i].rd.name);
			}
			else {
				fprintf(ICFile, "%d ", IClists[i].rd.value);
			}
			if (IClists[i].rs.name != NULL) {
				fprintf(ICFile, "%s ", IClists[i].rs.name);
			}
			else {
				fprintf(ICFile, "%d ", IClists[i].rs.value);
			}
		}
		else if (IClists[i].opNum == 1) {
			if (IClists[i].rd.name != NULL) {
				fprintf(ICFile, "%s ", IClists[i].rd.name);
			}
			else {
				fprintf(ICFile, "%d ", IClists[i].rd.value);
			}
		}
		fprintf(ICFile, "\n");
	}
	for (int i = 0; i < funcnum; i++) {
		fprintf(ICFile, "%s:\n", funclists[i].name);
		for (int j = 0; j < funclists[i].num; j++) {
			fprintf(ICFile, "%s ", ICName[funclists[i].lists[j].op]);
			if (funclists[i].lists[j].opNum == 3) {
				if (funclists[i].lists[j].rd.name != NULL) {
					fprintf(ICFile, "%s ", funclists[i].lists[j].rd.name);
				}
				else {
					fprintf(ICFile, "%d ", funclists[i].lists[j].rd.value);
				}
				if (funclists[i].lists[j].rs.name != NULL) {
					fprintf(ICFile, "%s ", funclists[i].lists[j].rs.name);
				}
				else {
					fprintf(ICFile, "%d ", funclists[i].lists[j].rs.value);
				}
				if (funclists[i].lists[j].rt.name != NULL) {
					fprintf(ICFile, "%s ", funclists[i].lists[j].rt.name);
				}
				else {
					fprintf(ICFile, "%d ", funclists[i].lists[j].rt.value);
				}
			}
			else if (funclists[i].lists[j].opNum == 2) {
				if (funclists[i].lists[j].rd.name != NULL) {
					fprintf(ICFile, "%s ", funclists[i].lists[j].rd.name);
				}
				else {
					fprintf(ICFile, "%d ", funclists[i].lists[j].rd.value);
				}
				if (funclists[i].lists[j].rs.name != NULL) {
					fprintf(ICFile, "%s ", funclists[i].lists[j].rs.name);
				}
				else {
					fprintf(ICFile, "%d ", funclists[i].lists[j].rs.value);
				}
			}
			else if (funclists[i].lists[j].opNum == 1) {
				if (funclists[i].lists[j].rd.name != NULL) {
					fprintf(ICFile, "%s ", funclists[i].lists[j].rd.name);
				}
				else {
					fprintf(ICFile, "%d ", funclists[i].lists[j].rd.value);
				}
			}
			fprintf(ICFile, "\n");
		}
	}
}