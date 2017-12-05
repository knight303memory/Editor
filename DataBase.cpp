#include "DataBase.h"


DataBase::DataBase(void)
{
	user = "root";
	password = "847748";
	host = "localhost" ;
	database = "editor" ;
	port = 3306 ;

	initConnect();
}


DataBase::~DataBase(void)
{
	if (result != NULL) 
	{
	mysql_free_result(result);
	}
	mysql_close(&mySQL);
}
void DataBase::initConnect() 
{
		mysql_init(&mySQL);
		mysql_real_connect(&mySQL, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, NULL, 0);
}

MYSQL_RES*  DataBase::excuteQuery(string sql)
{
	int res = 0;
	res = mysql_query(&mySQL,sql.c_str());

	if(!res)
	{
		result = mysql_store_result(&mySQL);
		return result;
	}
	else
	{
		return NULL;
	} 
}
