#ifndef IDTABLE
#define IDTABLE
typedef enum {
	con,
	var,
	array,
	func,
	voidfunc
} Category;
typedef enum {
	Integer,
	Char,
	Void
} Type;
typedef struct {
	char* name;
	Category category;
	Type type;
	int dimention;
	int level;
	Type paralist[20];
	int paraNum;
	int address;
} IdTable;
extern IdTable idTables[1024];
extern int idNum;
int searchId(char*);
int insertId(char* name, Category category, Type type, int dimention, int level);
void insertPara(int idNum, Type type);
void clearLevel(int level);
#endif