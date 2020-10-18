#ifndef PARSER
#define PARSER
#include "symTables.h"
typedef enum {
	PROGRAM = 0,
	CONSTSTATE,
	CONSTDEF,
	IDENTIFIER,
	INTEGER,
	UNSIGNEDINTEGER,
	LETTER,
	FIGURE,
	VARSTATE,
	VARDEF,
	VARDEFINIT,
	VARDEFNOINIT,
	TYPEIDENTIFIER,
	CONST,
	CHAR,
	ADDOPERATOR,
	MULTOPERATOR,
	FUCTIONDEF,
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
	RELATIONOPERATOR,
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
	FACTOR
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

void program(SymTable, gtNodePointer*);
void conststate(SymTable, gtNodePointer*);
void constdef(SymTable, gtNodePointer*);
void identifier(SymTable, gtNodePointer*);
void integer(SymTable, gtNodePointer*);
void unsignedinteger(SymTable, gtNodePointer*);
void letter(SymTable, gtNodePointer*);
void figure(SymTable, gtNodePointer*);
void varstate(SymTable, gtNodePointer*);
void vardef(SymTable, gtNodePointer*);
void vardefinit(SymTable, gtNodePointer*);
void vardefnoinit(SymTable, gtNodePointer*);
void typeidentifier(SymTable, gtNodePointer*);
void const_(SymTable, gtNodePointer*);
void char_(SymTable, gtNodePointer*);
void addoperator(SymTable, gtNodePointer*);
void multoperator(SymTable, gtNodePointer*);
void functiondef(SymTable, gtNodePointer*);
void voidfunctiondef(SymTable, gtNodePointer*);
void statehead(SymTable, gtNodePointer*);
void paralist(SymTable, gtNodePointer*);
void mainfunction(SymTable, gtNodePointer*);
void comstatement(SymTable, gtNodePointer*);
void statementlist(SymTable, gtNodePointer*);
void statement(SymTable, gtNodePointer*);
void loopstatement(SymTable, gtNodePointer*);
void stepsize(SymTable, gtNodePointer*);
void conditionstatement(SymTable, gtNodePointer*);
void condition(SymTable, gtNodePointer*);
void relationoperator(SymTable, gtNodePointer*);
void functioncall(SymTable, gtNodePointer*);
void voidfunctioncall(SymTable, gtNodePointer*);
void valueparalist(SymTable, gtNodePointer*);
void assignstatement(SymTable, gtNodePointer*);
void scan(SymTable, gtNodePointer*);
void print_(SymTable, gtNodePointer*);
void string_(SymTable, gtNodePointer*);
void switch_(SymTable, gtNodePointer*);
void caselist(SymTable, gtNodePointer*);
void case_(SymTable, gtNodePointer*);
void default_(SymTable, gtNodePointer*);
void return_(SymTable, gtNodePointer*);
void exp(SymTable, gtNodePointer*);
void item(SymTable, gtNodePointer*);
void factor(SymTable, gtNodePointer*);
#endif // 
