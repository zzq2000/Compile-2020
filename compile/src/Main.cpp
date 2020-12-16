#include <stdio.h>
#include <stdlib.h>
#include "symTables.h"
#include "parser.h"
#include "error.h"
#include "objectCode.h"
void print2file(FILE*);
int main() {
	FILE* inFile, * outFile;
	inFile = fopen("testfile.txt", "r");
	outFile = fopen("output.txt", "w");
	if (inFile == NULL || outFile == NULL) {
		exit(1);
	}
	//analysis(inFile);
	//print2file(outFile);
	parsering(inFile);
	outputOC();
	printIC();
	//gt2file(outFile, grammerTree);
	fclose(inFile);
	fclose(outFile);
}

//void print2file(FILE* outFile) {
//	for (int i = 0; i < symNum; i++) {
//		fprintf(outFile, "%s %s\n", sym[symTables[i].symbol], symTables[i].value);
//	}
//}

