#pragma once

#include <string>
#define _UNKNOWN 0
#define _INT 1
#define _LONG 2
#define _CHAR 3
#define _DOUBLE 4
#define _FLOAT 5
#define _SHORT 6


using namespace std ;
class SimbleTable
{


public:
	SimbleTable(void);
	~SimbleTable(void);

	int lineNum ;//定义时下标 
	string word ; 
	int length; 
	int type ; 
	string species ; //种属
	string value ; //值
	string mem ; //内存地址
};

