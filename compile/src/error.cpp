#include <stdio.h> 
#include <stdlib.h>
void error(int line, int column) {
	printf("Something Wrong with Your Program at line %d, column %d!", line, column);
	exit(1);
}
void err(int line) {
	printf("Something Wrong with Your Program at line %d!", line);
	exit(1);
}