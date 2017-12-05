#pragma once
#include <string>
#include <winsock.h>
#include "mysql.h"
using namespace std;
class DataBase
{
public:
	DataBase(void);
	~DataBase(void);

	string user ;
	string password;
	string host ;
	string database ;
	int port ;

	MYSQL mySQL;
	MYSQL_RES *result;


	void initConnect() ; 


	MYSQL_RES* excuteQuery(string sql);

};

