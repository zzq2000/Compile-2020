#ifndef IDTABLE
#define IDTABLE
typedef enum {
	con,
	var,
	array,
	func,
	voidfunc,
	temp,
	label,
} Category;
typedef enum {
	Integer,
	Char,
	String,
	Void
} Type;
typedef struct {
	char* name;
	Category category;
	Type type;
	int dimention;
	int level;
	Type paralist[32];
	char* paraName[32];
	int paraNum;
	int address;
	int value;
	int length[2];
} IdTable;
extern IdTable idTables[1024];
extern int idNum;
extern int addr;
extern int level;
int searchId(char*);
int insertId(char* name, Category category, Type type, int dimention);
int insertId(char* name, Category category, Type type, int dimention, int value);
int insertId(char* name, Category category, Type type, int dimention, int length[]);
IdTable createTempVar(Type);
void insertPara(int idNum, Type type, char* name);
void addLevel();
void clearLevel();
#endif