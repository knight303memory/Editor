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

	//�������ݿ�
	void initConnect() ; 

	//ִ�в�ѯ���
	MYSQL_RES* excuteQuery(string sql);


};

