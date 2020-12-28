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
	char* name;			//标识符名称
	Category category;	//标识符种类
	Type type;			//标识符类型
	int level;			//标识符层级
	Type paralist[32];	//函数参数类型列表
	char* paraName[32];	//函数参数名称列表
	int paraNum;		//函数参数个数
	int address;		//地址
	int value;			//初始值
	int dimention;		//标识符维度
	int length[2];		//各个维度上的长度
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