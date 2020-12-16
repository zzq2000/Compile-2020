#include<stdio.h>
#include<string.h>
#include"register.h"
 Reg tempReg[10];
 int tempRegNum = 0;
 char tempRegName[10][4] = {"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9"};
 Reg globalReg[8];
 int globalRegNum = 0;
 char globalRegName[8][4] = {"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7"};
 Reg paraReg[4];
 int paraRegNum = 0;
 char paraRegName[4][4] = { "$a0", "$a1", "$a2", "$a3" };

 int searchReg(Operand op) {
	 if (op.category == temp) {
		 for (int i = 0; i < tempRegNum; i++) {
			 if (tempReg[i].name != NULL && strcmp(tempReg[i].name, op.name) == 0) {
				 tempReg[i].count++;
				 return tempReg[i].id;
			 }
		 }
	 }
	 else if (op.category == var){
		 for (int i = 0; i < globalRegNum; i++) {
			 if (strcmp(globalReg[i].name, op.name) == 0) {
				 globalReg[i].count++;
				 return globalReg[i].id;
			 }
		 }
	 }
	 return -1;
 }

 int setReg(Operand op) {
	 if (op.category == temp) {
		 if (tempRegNum < 10) {
			 tempReg[tempRegNum].id = tempRegNum;
			 tempReg[tempRegNum].name = op.name;
			 tempReg[tempRegNum].count = 1;
			 tempReg[tempRegNum].dirty = 0;
			 tempReg[tempRegNum].valid = 1;
			 tempReg[tempRegNum].op = op;
			 tempRegNum++;
			 return tempRegNum - 1;
		 }
		 else {
			 int id = 0;
			 for (int i = 0; i < 10; i++) {
				 if (tempReg[i].valid == 0) {
					 id = i;
				 }
			 }
			 tempReg[id].id = id;
			 tempReg[id].name = op.name;
			 tempReg[id].count = 1;
			 tempReg[id].dirty = 0;
			 tempReg[id].valid = 1;
			 tempReg[id].op = op;
			 return id;
		 }
	 }
	 else if (op.category == var) {
		 if (globalRegNum < 8) {
			 globalReg[globalRegNum].id = globalRegNum;
			 globalReg[globalRegNum].name = op.name;
			 globalReg[globalRegNum].count = 1;
			 globalReg[globalRegNum].dirty = 0;
			 globalReg[globalRegNum].valid = 1;
			 globalReg[globalRegNum].op = op;
			 globalRegNum++;
			 return globalRegNum-1;
		 }
		 else {
			 int id= 0;
			 int mincount = globalReg[0].count;
			 for (int i = 1; i < globalRegNum; i++) {
				 if (globalReg[i].count < mincount) {
					 id = i;
					 mincount = globalReg[i].count;
				 }
			 }
			 globalReg[id].id = id;
			 globalReg[id].name = op.name;
			 globalReg[id].count = 1;
			 globalReg[id].dirty = 0;
			 globalReg[id].valid = 1;
			 globalReg[id].op = op;
			 return id;
		 }
	 }
	 return -1;
 }

 int setReg() {
	 if (tempRegNum < 10) {
		 tempReg[tempRegNum].id = tempRegNum;
		 tempReg[tempRegNum].name = NULL;
		 tempReg[tempRegNum].count = 1;
		 tempReg[tempRegNum].dirty = 0;
		 tempReg[tempRegNum].valid = 1;
		 tempRegNum++;
		 return tempRegNum - 1;
	 }
	 else {
		 int id = 0;
		 for (int i = 0; i < 10; i++) {
			 if (tempReg[i].valid == 0) {
				 id = i;
			 }
		 }
		 tempReg[id].id = id;
		 tempReg[id].name = NULL;
		 tempReg[id].count = 1;
		 tempReg[id].dirty = 0;
		 tempReg[id].valid = 1;
		 return id;
	 }
 }

 int setParaReg(Operand op) {
	 if (paraRegNum < 4) {
		 paraReg[paraRegNum].id = paraRegNum;
		 paraReg[paraRegNum].name = op.name;
		 paraReg[paraRegNum].count = 0;
		 paraReg[paraRegNum].valid = 1;
		 paraRegNum++;
		 return paraRegNum - 1;
	 }
	 return -1;
 }

 /*int searchParaReg(Operand op) {
	 for (int i = 0; i < paraRegNum; i++) {
		 if (strcmp(op.name, paraReg[i].name) == 0) {
			 return i;
		 }
	 }
	 return -1;
 }*/

 void clearTempReg() {
	 tempRegNum = 0;
 }

 void clearTempReg(int id) {
	 tempReg[id].id = -1;
	 tempReg[id].name = NULL;
	 tempReg[id].valid = 0;
	 tempReg[id].count = 0;
	 tempReg[id].dirty = 0;
	 tempRegNum--;
 }

 void clearGlobalReg() {
	 globalRegNum = 0;
 }

 void clearGlobalReg(int id) {
	 globalReg[id].id = -1;
	 globalReg[id].name = NULL;
	 globalReg[id].valid = 0;
	 globalReg[id].count = 0;
	 globalReg[id].dirty = 0;
	 globalRegNum--;
 }

 void clearParaReg() {
	 paraRegNum = 0;
 }

 void setNotValid(int id) {
	 tempReg[id].valid = 0;
 }

 void changed(Operand op) {
	 if (op.category == var) {
		 for (int i = 0; i < globalRegNum; i++) {
			 if (strcmp(globalReg[i].name, op.name) == 0) {
				 globalReg[i].dirty = 1;
			 }
		 }
	 }
 }