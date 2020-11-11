#include <stdio.h> 
#include <stdlib.h>
#include "error.h"
FILE* errorFile = fopen("error.txt", "w");
void error(int line, int column) {
	printf("Something Wrong with Your Program at line %d, column %d!", line, column);
	//exit(1);
}
void err(int line, char* s) {
	//printf("%d %s\n", line, s);
}
void errorPro(int line, char c) {
	if (errorFile!=NULL) fprintf(errorFile, "%d %c\n", line, c);
}