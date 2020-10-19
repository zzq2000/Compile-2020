#ifndef SYMTABLES
#define SYMTABLES
#define MAXL 1024
typedef enum
{
	IDENFR = 0,
	INTCON,
	CHARCON,
	STRCON,
	CONSTTK,
	INTTK,
	CHARTK,
	VOIDTK,
	MAINTK,
	IFTK,
	ELSETK,
	SWITCHTK,
	CASETK,
	DEFAULTTK,
	WHILETK,
	FORTK,
	SCANFTK,
	PRINTFTK,
	RETURNTK,
	PLUS,
	MINU,
	MULT,
	DIV,
	LSS,
	LEQ,
	GRE,
	GEQ,
	EQL,
	NEQ,
	COLON,
	ASSIGN,
	SEMICN,
	COMMA,
	LPARENT,
	RPARENT,
	LBRACK,
	RBRACK,
	LBRACE,
	RBRACE,
	nullsym
} Symbol;

typedef struct {
	Symbol symbol;
	char* value;
} SymTable;
extern SymTable symTables[];
extern int symNum;
extern char sym[][10];
extern char keywords[][10];
extern int line;
void analysis(FILE* );
SymTable getsym_(FILE* inFile);
#endif // !SYMTABLES
