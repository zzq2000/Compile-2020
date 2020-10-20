const int NE = 0;
const int LT = +1, GE = -2, GT = -20, LE = 888, EQ = +22;
const char constA = 'A';
const char constB = '+', constC = '2', constD = '*';

int x;
char array[10], double_axis_array[10][9];
char t = 'z';
int globalFlag = 1;
int Nums[4] = {0, 1, 2, 3};
int flags[3][4] = {{0, 0, 0, 0},{1, 1, 1, 1},{1, 0, 0, 1}};

int getNumSum() {
    const char unUsed = '-';
    int sum = 0;
    int i;
    for (i = 0; i < 3; i = i + 1) {
        sum = sum + Nums[i];
    }
    return (sum);
}

int whileGetNumSum() {
    int i = 0;
    int sum = 0;
    while (i < 3) {
        sum = sum + Nums[i];
        i = i + 1;
    }
    return (sum);
}

int getX() {
    return (x);
}

int getNum(int i) {
    return (Nums[i]);
}

int add_and_increase(int fff, int y) {
    return (fff + y + 1);
}

void none() {

}

void setNum(int i, int x) {
    Nums[i] = x;
    return;
}

char getAAA(){
    return ('A');
}

void main() {
    int i, j, k;
    printf("18373317");
    if (1==1) {
    }
    if (0==1) {
        x = 100;
    } else {
        j = 0;
        for (x = 6; x >= 0; x = x-1)
            for (i = 0; i <= 6; i = i+1)
                double_axis_array[x][i] = 'a';
    }
    switch (- +2 * 3 - x) {
        case -5:
            x = getNumSum();
        default:;
    }
    none();
    scanf(k);
    printf(add_and_increase(1, k));
    printf(whileGetNumSum());
    printf("abcdefghijklmnopqrstuvwxyz_");
    printf(add_and_increase(getNum(2) * getX() + EQ, globalFlag));
    printf('g');
    printf(('c'));
    printf(double_axis_array[0][3]);
    printf(flags[2][x * 0 + 2]);
    printf("a", 'a');
}
