#include <stdio.h>
#include <stdlib.h>
#include "symTables.h"
#include "parser.h"
void print2file(FILE*);
int main() {
	FILE* inFile, * outFile;
	if ((inFile = fopen("testfile.txt", "r")) == NULL || (outFile = fopen("output.txt", "w")) == NULL) {
		exit(1);
	}
	//analysis(inFile);
	//print2file(outFile);
	parsering(inFile);
	gt2file(outFile, grammerTree);
	fclose(inFile);
	fclose(outFile);
}

//void print2file(FILE* outFile) {
//	for (int i = 0; i < symNum; i++) {
//		fprintf(outFile, "%s %s\n", sym[symTables[i].symbol], symTables[i].value);
//	}
//}

