#ifndef INTER
#define INTER
typedef enum {
	add,
	sub,
	mult,
	div,
	read,
	print,
	move,
} OP;
typedef struct {
	OP op;
	int rd;
	int rs;
	int rt;
} IC;
extern IC IClists[1024];
extern int ICnum;
void ICread();
#endif // !INTER

