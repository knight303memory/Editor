#pragma once
#include <string>
#include <winsock.h>
#include "mysql.h"
using namespace std;
class DBMS
{
public:
	DBMS(void);

	~DBMS(void);

	string user ;
	string password;
	string host ;
	string database ;
	int port ;

	MYSQL mySQL;
	MYSQL_RES *result;

	//连接数据库
	void initConnect() ; 

	//执行查询语句
	MYSQL_RES* excuteQuery(string sql);


};

