#ifndef PARSER
#define PARSER
#include "symTables.h"
typedef enum {
	PROGRAM = 0,
	CONSTSTATE,
	CONSTDEF,
	INTEGER,
	UNSIGNEDINTEGER,
	VARSTATE,
	VARDEF,
	VARDEFINIT,
	VARDEFNOINIT,
	CONST,
	FUNCTIONDEF,
	VOIDFUNCTIONDEF,
	STATEHEAD,
	PARALIST,
	MAINFUNCTION,
	COMSTATEMENT,
	STATEMENTLIST,
	STATEMENT,
	LOOPSTATEMENT,
	STEPSIZE,
	CONDITIONSTATEMENT,
	CONDITION,
	FUNCTIONCALL,
	VOIDFUNCTIONCALL,
	VALUEPARALIST,
	ASSIGNSTATEMENT,
	SCAN,
	PRINT,
	STRING,
	SWITCH,
	CASELIST,
	CASE,
	DEFAULT,
	RETURN,
	EXP,
	ITEM,
	FACTOR,
	nullvn
}Vn;
struct grammerTreeNode {
	Vn vn;
	SymTable* symTable;
	struct grammerTreeNode* sonNode[100];
};
typedef struct grammerTreeNode gtNode;
typedef struct grammerTreeNode* gtNodePointer;
extern gtNodePointer grammerTree;
extern char vnList[][40];

void createTreeNode(gtNodePointer, Vn, SymTable);
void parsering(FILE*);
void gt2file(FILE*, gtNodePointer);

void program(gtNodePointer*);
void conststate(gtNodePointer*);
void intconstdef(gtNodePointer*);
void charconstdef(gtNodePointer*);
void integer(gtNodePointer*);
void unsignedinteger(gtNodePointer*);
void varstate(gtNodePointer*);
void vardef(gtNodePointer*);
void vardefinit(gtNodePointer*);
void vardefnoinit(gtNodePointer*);
void const_(gtNodePointer*);
void addoperator(gtNodePointer*);
void multoperator(gtNodePointer*);
void functiondef(gtNodePointer*);
void voidfunctiondef(gtNodePointer*);
void statehead(gtNodePointer*);
void paralist(gtNodePointer*);
void mainfunction(gtNodePointer*);
void comstatement(gtNodePointer*);
void statementlist(gtNodePointer*);
void statement(gtNodePointer*);
void loopstatement(gtNodePointer*);
void stepsize(gtNodePointer*);
void conditionstatement(gtNodePointer*);
void condition(gtNodePointer*);
void relationoperator(gtNodePointer*);
void functioncall(gtNodePointer*);
void voidfunctioncall(gtNodePointer*);
void valueparalist(gtNodePointer*);
void assignstatement(gtNodePointer*);
void scan(gtNodePointer*);
void print_(gtNodePointer*);
void string_(gtNodePointer*);
void switch_(gtNodePointer*);
void caselist(gtNodePointer*);
void case_(gtNodePointer*);
void default_(gtNodePointer*);
void return_(gtNodePointer*);
void exp(gtNodePointer*);
void item(gtNodePointer*);
void factor(gtNodePointer*);
#endif // 
