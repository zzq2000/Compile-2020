#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>
#include"symTables.h"
#include"error.h"

SymTable symTables[100000];
int symNum = 0;
char sym[][10] = { "IDENFR", "INTCON","CHARCON","STRCON","CONSTTK","INTTK","CHARTK","VOIDTK","MAINTK",
	"IFTK","ELSETK","SWITCHTK","CASETK","DEFAULTTK","WHILETK","FORTK","SCANFTK","PRINTFTK","RETURNTK","PLUS",
	"MINU","MULT","DIV","LSS","LEQ","GRE","GEQ","EQL","NEQ","COLON", "ASSIGN", "SEMICN", "COMMA", "LPARENT",
	"RPARENT", "LBRACK", "RBRACK", "LBRACE", "RBRACE",
};
char keywords[][10] = { "const", "int", "char", "void", "main", "if", "else", "switch", "case", "default", "while",
	"for", "scanf", "printf", "return"
};

int line = 1;
int column = 0;
int lineend = 0;
char token[MAXL];
int tokenlen = 0;
Symbol symbol_;
void clearToken() {
	tokenlen = 0;
	token[tokenlen] = '\0';
}

void catToken(int c) {
	token[tokenlen++] = c;
	token[tokenlen] = '\0';
}

int selectKey() {
	char token_[128];
	for (int i = 0; i < tokenlen; i++) {
		token_[i] = tolower(token[i]);
	}
	token_[tokenlen] = '\0';
	for (int j = 0; j < 15; j++) {
		if (strcmp(token_, keywords[j]) == 0) {
			symbol_ = (Symbol)(j + CONSTTK);
			return 1;
		}
	}
	return 0;
}

int getchar_(FILE* inFile) {
	column++;
	return fgetc(inFile);
}

void getsym(FILE* inFile, int c) {
	clearToken();
	lineend = 0;
	while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
		if (c == '\n') {
			line++;
			lineend = 1;
			column = 0;
		}
		c = getchar_(inFile);
		if (c == EOF) {
			return;
		}
	}
	if (isalpha(c) || c == '_') {
		do {
			catToken(c);
			c = getchar_(inFile);
		} while (isalnum(c) || c == '_');
		ungetc(c, inFile);
		column--;
		int resultValue = selectKey();
		if (resultValue != 1) {
			symbol_ = IDENFR;
		}
	}
	else if (isdigit(c)) {
		do {
			catToken(c);
			c = getchar_(inFile);
		} while (isdigit(c));
		ungetc(c, inFile);
		symbol_ = INTCON;
	}
	else if (c == '\'') {
		c = getchar_(inFile);
		if (c != '\'') {
			catToken(c);
			c = getchar_(inFile);
		}
		symbol_ = CHARCON;
	}
	else if (c == '\"') {
		c = getchar_(inFile);
		while (c != '\"') {
			catToken(c);
			c = getchar_(inFile);
		}
		symbol_ = STRCON;
	}
	else if (c == '+') {
		catToken(c);
		symbol_ = PLUS;
	}
	else if (c == '-') {
		catToken(c);
		symbol_ = MINU;
	}
	else if (c == '*') {
		catToken(c);
		symbol_ = MULT;
	}
	else if (c == '/') {
		catToken(c);
		symbol_ = DIV;
	}
	else if (c == '<') {
		catToken(c);
		c = getchar_(inFile);
		if (c == '=') {
			catToken(c);
			symbol_ = LEQ;
		}
		else {
			ungetc(c, inFile);
			column--;
			symbol_ = LSS;
		}
	}
	else if (c == '>') {
		catToken(c);
		c = getchar_(inFile);
		if (c == '=') {
			catToken(c);
			symbol_ = GEQ;
		}
		else {
			ungetc(c, inFile);
			column--;
			symbol_ = GRE;
		}
	}
	else if (c == '=') {
		catToken(c);
		c = getchar_(inFile);
		if (c == '=') {
			catToken(c);
			symbol_ = EQL;
		}
		else {
			ungetc(c, inFile);
			column--;
			symbol_ = ASSIGN;
		}
	}
	else if (c == '!') {
		catToken(c);
		c = getchar_(inFile);
		if (c == '=') {
			catToken(c);
			symbol_ = NEQ;
		}
	}
	else if (c == ':') {
		catToken(c);
		symbol_ = COLON;
	}
	else if (c == ';') {
		catToken(c);
		symbol_ = SEMICN;
	}
	else if (c == ',') {
		catToken(c);
		symbol_ = COMMA;
	}
	else if (c == '(') {
		catToken(c);
		symbol_ = LPARENT;
	}
	else if (c == ')') {
		catToken(c);
		symbol_ = RPARENT;
	}
	else if (c == '[') {
		catToken(c);
		symbol_ = LBRACK;
	}
	else if (c == ']') {
		catToken(c);
		symbol_ = RBRACK;
	}
	else if (c == '{') {
		catToken(c);
		symbol_ = LBRACE;
	}
	else if (c == '}') {
		catToken(c);
		symbol_ = RBRACE;
	}
	//else error(line, column);
}

SymTable getsym_(FILE* inFile) {
	SymTable symTable;
	int c = fgetc(inFile);
	if (c != EOF) {
		getsym(inFile, c);
		if (tokenlen != 0) {
			symTable.symbol = symbol_;
			symTable.value = (char *)malloc(sizeof(char) * tokenlen + 1);
			if (symTable.value != NULL)
				strcpy(symTable.value, token);
			return symTable;
		}
		else if (tokenlen == 0 && (symbol_ == CHARCON || symbol_ == STRCON)) {
			symTable.symbol = symbol_;
			symTable.value = NULL;
			return symTable;
		}
	}
	symTable.symbol = nullsym;
	return symTable;
}

void analysis(FILE* inFile) {
	int c = 0;
	while ((c = getchar_(inFile)) != EOF) {
		getsym(inFile, c);
		if (tokenlen != 0) {
			symTables[symNum].symbol = symbol_;
			symTables[symNum].value = (char*)malloc(sizeof(char) * tokenlen + 1);
			if(symTables[symNum].value != NULL)
				strcpy(symTables[symNum++].value, token);
		}
	}
}