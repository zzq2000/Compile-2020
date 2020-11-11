#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"idTable.h"
#include"symTables.h"
#include"error.h"
IdTable idTables[1024];
int idNum = 0;

int searchId(char* name) {
	int i;
	for (i = idNum-1; i >= 0; i--) {
		if (strcmp(idTables[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

int insertId(char* name, Category category, Type type, int dimention, int level) {
	int searchResult = searchId(name);
	if (searchResult != -1 && idTables[searchResult].level == level) {
		errorPro(line, 'b');
		return -1;
	}
	IdTable newTable;
	newTable.name = (char*)malloc(strlen(name) + 1);
	strcpy(newTable.name, name);
	newTable.category = category;
	newTable.type = type;
	newTable.dimention = dimention;
	newTable.level = level;
	newTable.paraNum = 0;
	idTables[idNum++] = newTable;
	return idNum-1;
}

void insertPara(int idNum, Type type) {
	idTables[idNum].paralist[idTables[idNum].paraNum++] = type;
}

void clearLevel(int level) {
	while (idTables[idNum-1].level == level) {
		idNum--;
	}
}