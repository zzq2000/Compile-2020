#include <stdio.h>
#include <string.h>
#include "str.h"
vector<string> strlist;
int strlenSum = 0;


int addstr(char* s) {
	if (s != NULL) {
		string s_ = s;
		strlist.push_back(s_);
		strlenSum = strlenSum + strlen(s) + 1;
		return getStrNum() - 1;
	}
	return -1;
}

int getStrNum() {
	return strlist.size();
}
