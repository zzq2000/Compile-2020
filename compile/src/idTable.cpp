#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include"idTable.h"
#include"symTables.h"
#include"error.h"
IdTable idTables[1024];
int idNum = 0;
int addr = 0;
int level = 0;
int tempVar = 0;	//专门给switch语句存变量使用

char* toLower(char* s) {
	int len = strlen(s);
	for (int i = 0; i < len; i++) {
		s[i] = tolower(s[i]);
	}
	return s;
}

int alloc(Type type) {
	if (level == 0) {
		int varaddr = addr;
		if (type == Char) {
			addr++;
		}
		else if (type == Integer) {
			if (addr % 4 != 0) {
				addr = addr + 4 - addr % 4;
				varaddr = addr;
				addr = addr + 4;
			}
			else {
				varaddr = addr;
				addr = addr + 4;
			}
		}
		return varaddr;
	}
	else {
		if (type == Char) {
			addr -= 1;
			return addr;
		}
		else {
			if (addr % 4 == 0) {
				addr -= 4;
				return addr;
			}
			else {
				addr -= 8 + addr % 4;
				return addr;
			}
		}
	}
}

int alloc(Type type, int dimention, int length[]) {
	if (level == 0) {
		int varaddr = addr;
		if (type == Char) {
			int temp = 1;
			for (int i = 0; i < dimention; i++) {
				temp *= length[i];
			}
			addr += temp;
		}
		else if (type == Integer) {
			if (addr % 4 != 0) {
				addr = addr + 4 - addr % 4;
				varaddr = addr;
			}
			int temp = 1;
			for (int i = 0; i < dimention; i++) {
				temp *= length[i];
			}
			addr += temp * 4;
		}
		return varaddr;
	}
	else {
		if (type == Char) {
			int temp = 1;
			for (int i = 0; i < dimention; i++) {
				temp *= length[i];
			}
			addr -= temp;
			return addr;
		}
		else {
			if (addr % 4 != 0) {
				addr -= 4 + addr % 4;
			}
			int temp = 1;
			for (int i = 0; i < dimention; i++) {
				temp *= length[i];
			}
			addr -= temp * 4;
			return addr;
		}
	}
}

int searchId(char* name) {
	int i;
	name = toLower(name);
	for (i = idNum-1; i >= 0; i--) {
		if (strcmp(idTables[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

int insertId(char* name, Category category, Type type, int dimention) {
	int searchResult = searchId(name);
	if (searchResult != -1 && idTables[searchResult].level == level) {
		errorPro(line, 'b');
		return -1;
	}
	IdTable newTable;
	newTable.name = toLower(name);
	newTable.category = category;
	newTable.type = type;
	newTable.dimention = dimention;
	newTable.level = level;
	newTable.paraNum = 0;
	if (category == var) {
		newTable.address = alloc(type);
	}
	idTables[idNum++] = newTable;
	return idNum-1;
}

int insertId(char* name, Category category, Type type, int dimention, int value) {
	int searchResult = searchId(name);
	if (searchResult != -1 && idTables[searchResult].level == level) {
		errorPro(line, 'b');
		return -1;
	}
	IdTable newTable;
	newTable.name = toLower(name);
	newTable.category = category;
	newTable.type = type;
	newTable.dimention = dimention;
	newTable.level = level;
	newTable.paraNum = 0;
	newTable.value = value;
	if (category == var) {
		newTable.address = alloc(type);
	}
	idTables[idNum++] = newTable;
	return idNum - 1;
}

int insertId(char* name, Category category, Type type, int dimention, int length[]) {
	int searchResult = searchId(name);
	if (searchResult != -1 && idTables[searchResult].level == level) {
		errorPro(line, 'b');
		return -1;
	}
	IdTable newTable;
	newTable.name = toLower(name);
	newTable.category = category;
	newTable.type = type;
	newTable.dimention = dimention;
	newTable.level = level;
	newTable.paraNum = 0;
	for (int i = 0; i < dimention; i++) {
		newTable.length[i] = length[i];
	}
	newTable.address = alloc(type, dimention, length);
	idTables[idNum++] = newTable;
	return idNum - 1;
}

IdTable createTempVar(Type type) {
	IdTable newTable;
	newTable.name = (char*)malloc(10 * sizeof(char));
	sprintf(newTable.name, "TEMPVAR_%d", tempVar++);
	newTable.category = var;
	newTable.type = type;
	newTable.address = alloc(type);
	newTable.level = level;
	return newTable;
}

void insertPara(int idNum, Type type, char* name) {
	idTables[idNum].paraName[idTables[idNum].paraNum] = toLower(name);
	idTables[idNum].paralist[idTables[idNum].paraNum++] = type;
}

void addLevel() {
	level++;
	addr = 0;
}

void clearLevel() {
	while (idTables[idNum-1].level == level) {
		idNum--;
	}
	level--;
}