#include "editor.h"

Editor::Editor(QWidget *parent)
	: QMainWindow(parent)
{
	addFlag1 = false;
	addFlag2 = false;
	ifContinue = true ; 
	ui.setupUi(this);
	identifierFlag = false ;

	connect(ui.open,SIGNAL(triggered()),this,SLOT(openFile() ));
	connect(ui.save,SIGNAL(triggered()),this,SLOT(saveFile() ));
	connect(ui.cifafenxi,SIGNAL(triggered()),this,SLOT(analyseWord())); //词法分析
	connect(ui.showSimbleList,SIGNAL(triggered()),this,SLOT(showSimbleList())); //显示符号表
	connect(ui.yufafenxi,SIGNAL(triggered()),this,SLOT(analyseSentence())); //语法分析

}

Editor::~Editor()
{

}

//词法分析
void Editor::analyseWord()
{

	QStringList content = ui.textEdit->toPlainText().split("\n");

	//每一行分析 , 根据空格分隔  
	for (int i = 0; i < content.length(); i++)
	{
		identifierFlag = false ;
		simbleType = _UNKNOWN ;
		QString lineStr = content.at(i);
		lineStr = lineStr.replace(QRegExp("[\\s]+"), " ");
		QStringList words = lineStr.split(" ");
		for (int j = 0; j < words.length(); j++)
		{
			findToken(words.at(j).toStdString(),i+1,lineStr.toStdString());
		}
	}

	//显示出结果 
	showAnalyseResult();
	dealWithError();

	showErrorList();
}
void Editor::findToken(string word,int lineNum,string lineStr)
{
	//这个单词是否是合法标识符 , 
	legalFlag = false ; 

	//如果这行被注释 
	if (memLine == lineNum)
	{
		return ; 
	}

	if(ifContinue)
	{
		if(identifierFlag)
		{
			string ident = word ; 
			//以下划线和字母开始 w[a-zA-Z0-9_] 
			QRegExp reg("\\b[a-zA-Z_]\\w*");

			//先对逗号进行处理
			if (word.length() > 1 && ( word[word.length() - 1] == ',' ||  word[word.length() - 1] == ';' ))
			{
				//形如 int day, 截取到day  
				ident =  word.substr(0,word.length() - 1);
			}
			bool match = reg.exactMatch(QString::fromStdString(ident));	

			if(match)
			{
				addResult(lineNum,ident,"100");
				addSimble(lineNum,ident); //增加到符号表中
			}
		}

		string sql = "select * from word where ch='";
		string excuSql = sql + word + "'";
		result = dbms.excuteQuery(excuSql);
		MYSQL_ROW sql_row;

		if (result)
		{ 
			while ((sql_row = mysql_fetch_row(result)) )
			{
				//如果是关键字 那么在当前行中全字匹配  while 
				if(atoi( sql_row[1]) >= 0 && atoi( sql_row[1]) <= 31)
				{
					addResult(lineNum,sql_row[0],sql_row[1]);

					//如果是基本类型 , 设identifierFlag = true ; 
					switch (atoi( sql_row[1]))
					{

					case 5:simbleType = _CHAR ;identifierFlag = true ;break ;
					case 10:simbleType = _DOUBLE ;identifierFlag = true ;break ;
					case 13:simbleType = _FLOAT ;identifierFlag = true ;break ;
					case 17:simbleType = _INT ;identifierFlag = true ;break ;
					case 18:simbleType = _LONG ;identifierFlag = true ;break ;
					case 21:simbleType = _SHORT ;identifierFlag = true ;break ;
					default:
						break;
					}
				}
				//如果是括号 运算符号等
				else
				{
					//++   >=  -- 
					if( word.length() > 0)
					{
						string row = sql_row[0] ; 
						//如果不是括号等  就全字匹配 但是要考虑小括号这些
						if(row.length() > 1)
						{
							//如果是单行注释 
							if(atoi(sql_row[1]) == 87)
							{
								//	addResult(lineNum,sql_row[0],sql_row[1]);

								memLine = lineNum ;  
								return  ; 
							}
							//多行注释
							if(atoi(sql_row[1]) == 83)
							{
								addResult(lineNum,sql_row[0],sql_row[1]);

								closeCheckMap[83]++;
								ifContinue = false ; 
								break;
							}

							//++ ， -- ， >= 
							addResult(lineNum,sql_row[0],sql_row[1]);

						}

						//小括号 花括号 "
						else if(row.length() == 1 )
						{
							if(atoi(sql_row[1]) == 74)
							{
								identifierFlag = false ;
								simbleType = _UNKNOWN ;
							}

							addResult(lineNum,sql_row[0],sql_row[1]);

							int index = atoi( sql_row[1]);
							checkMatch(index);

						}

					}

				}

			}

		}
	}

	// 搜索这种形式的  (9    但是 ++ 上面已经匹配过了 现在又会进行匹配  
	if(word.length() > 1 )
	{ 
		for (int i = 0; i < word.length(); i++)
		{
			//如果不在多行注释范围
			if(ifContinue)
			{
				//当以#开始, 可能是宏定义 
				if( i == 0 &&  ( word[i] == '#')){

					string str = word.substr(1,word.length());
					string sql = "select * from word where ch = '"+str +"'";

					MYSQL_RES *res2 = dbms.excuteQuery(sql);
					MYSQL_ROW sql_row2;
					if(res2)
					{
						while (sql_row2 = mysql_fetch_row(res2))
						{
							addResult(lineNum,"#","77");		
							addResult(lineNum,sql_row2[0],sql_row2[1]);
						}
					}

					break ;
				}
				//	如果是单行注释
				if( i == 0 &&  ( word[i] == '/') && word[1] == '/')
				{
					//	addResult(lineNum,"//","87");
					memLine = lineNum ;  
					break ; //直接退出循环
				}	

				//如果是多行注释 
				if( i == 0 &&  ( word[i] == '/') && word[1] == '*')
				{
					addResult(lineNum,"/*","83");

					closeCheckMap[83] ++ ;

					ifContinue = false ;

				}	
			}

			//多行注释结束
			if (!ifContinue)
			{
				for(int j =  word.length() - 1 ; j >= 1 ; j--)
				{
					if(word[j] == '/' && word[j-1] == '*')
					{
						closeCheckMap[83]--;
						addResult(lineNum,"*/","84");

						ifContinue = true ;
						i = j + 1 ;
						break ; 
					}
				}

			}

			//多行注释之后继续匹配  
			if(i < word.length() && ifContinue)
			{
				//不是宏定义
				//要处理 _++_这种问题 有的已经匹配过了,但是逐个又匹配
				string eachChar = charToString(word[i]);
				string secSql = "select * from word where ch = '";
				secSql = secSql + eachChar + "'";

				MYSQL_RES *res = dbms.excuteQuery(secSql);
				MYSQL_ROW sql_row1;


				if(res)
				{
					while (sql_row1 = mysql_fetch_row(res))
					{
						bool singleFlag = true ; 
						//对 ++ --  
						if( i != 0 )
						{
							string thiSql = "select * from word where ch = '" + word[i - 1] + eachChar + "'";

							MYSQL_RES *res2 = dbms.excuteQuery(thiSql);
							MYSQL_ROW sql_row2;
							if (res2)
							{

								while (	sql_row2 = mysql_fetch_row(res2))
								{
									addResult(lineNum,sql_row2[0],sql_row2[1]);

									singleFlag = false ; 

								}

							}
						}
						if( singleFlag && i != word.length() - 1 )
						{
							string thiSql = "select * from word where ch = '"  + eachChar + word[i + 1] + "'";
							MYSQL_RES *res3 = dbms.excuteQuery(thiSql);
							MYSQL_ROW sql_row3;
							if (res3)
							{
								//必然只搜出一条记录
								while (	sql_row3 = mysql_fetch_row(res3))
								{
									addResult(lineNum,sql_row3[0],sql_row3[1]);

									singleFlag = false ; 
									i = i + 2 ;
								}
							}
						}

						//只是 +  不是 ++ 
						if(singleFlag)
						{
							if(atoi(sql_row1[1]) == 74)  //是 ; 终止标识符定义  
							{
								identifierFlag = false ;
								simbleType = _UNKNOWN ;

							}

							addResult(lineNum,sql_row1[0],sql_row1[1]);		
						}

						int index = atoi( sql_row1[1]);
						checkMatch(index);
					}
				}

			}

		}

	}

	if(ifContinue && !legalFlag)
	{
		//未识别到的标识符进行第二次筛选 
		//首先判断标识符是否已经被识别  
		for(int i = 0 ; i < simbleList.size(); i ++)
		{
			if (simbleList.at(i).word.compare(word) ==  0)
			{
				//如果是在符号表中已经存在  直接返回
				addResult(lineNum,word,"100");
				return ;
			}
		}
		//是未识别的标识符,需要判断是否是int 类型赋值常数 
		if(isNum(word))
		{
			addResult(lineNum,word,"101");
		}
		//如果都未识别到  则是错误信息 
		else if(word.length() > 0)
		{
			//如果是合法字符 也存入token表
			QRegExp reg("\\b[a-zA-Z_]\\w*");

			if(reg.exactMatch(QString::fromStdString(word)))
			{
				addResult(lineNum,word,"100");
			}
			else
			{
				addErrorInfo(lineNum," unknown identifier "+word);
			}
		}

	}

}


//括号匹配等 
void Editor::checkMatch(int index)
{
	switch (index)
	{
	case 32:
	case 75:
	case 68:
	case 83:
		closeCheckMap[index]++;
		break ;
	case 33:
	case 76:
	case 69:
	case 84:

		closeCheckMap[index - 1]--;
		break ;
	case 85:
		if(addFlag1){
			closeCheckMap[85]--;
		}else
		{
			closeCheckMap[85]++;
		}
		addFlag1 = !addFlag1;
		break;
	case 86:
		if(addFlag2){
			closeCheckMap[86]--;
		}else
		{
			closeCheckMap[86]++;
		}
		addFlag2 = !addFlag2;
		break;
	default:
		break;
	}
}

//增加一条结果
void Editor::addResult(int lineNum,string ch,string index)
{
	legalFlag = true  ;//word 已经被识别,是合法的 
	string res = intToString(lineNum) + ":	" +ch+ "	" + index;
	analysisResult.append((QString::fromStdString(res)));
	tokens.push_back(ch);
	lineNumber.push_back(lineNum);
	chIndex.push_back(index);
}

//添加错误信息
void Editor::addErrorInfo(int lineNum , string error)
{
	if(lineNum != -1)
	{

		errorInfoList.append(QString::fromStdString(intToString(lineNum))+ QString::fromStdString(" : " +error));

	}else
	{
		errorInfoList.append( QString::fromStdString(" " +error));

	}
}


//符号表中增加一个符号
void Editor::addSimble(int lineNum , string ident)
{
	SimbleTable simble ;
	simble.lineNum = lineNum ;
	simble.word = ident; 
	simble.length = ident.length();  
	simble.species = "var";
	simble.type = simbleType ;
	simbleList.push_back(simble);
}



//显示符号表
void Editor::showSimbleList()
{
	ui.tokenInfo->clear();

	ui.tokenInfo->insertPlainText(QStringLiteral("入口  单词  长度   类型   种属    值   内存地址\n"));

	for(int i = 0 ; i != simbleList.size() ; i++)
	{
		string index  = intToString(i);
		string word = simbleList.at(i).word;
		string length = intToString(simbleList.at(i).length);
		string type = intToString(simbleList.at(i).type);
		string value =  simbleList.at(i).value;
		string spe =  simbleList.at(i).species;
		string mem =  simbleList.at(i).mem;
		ui.tokenInfo->insertPlainText(" "+QString::fromStdString(index)+"     "+QString::fromStdString(word)+"     "
			+QString::fromStdString(length)+"      "+QString::fromStdString(type)+"    "+QString::fromStdString(spe)+"    "+
			QString::fromStdString(value)+"   "
			+QString::fromStdString(mem)+"\n");
	}
}

//显示分析结果
void Editor::showAnalyseResult()
{

	ui.tokenInfo->clear();

	analysisResult.insert(0,QString("row") + ":	" + "token	 index");
	for(int i = 0 ; i != analysisResult.size() ; i++)
	{
		ui.tokenInfo->insertPlainText(QString(analysisResult.at(i))+"\n");
	}
}

//显示错误信息
void Editor::showErrorList()
{
	ui.errorInfo->clear();

	for(int i  = 0 ; i < errorInfoList.size() ; i++)
	{
		ui.errorInfo->insertPlainText(QString(errorInfoList.at(i))+"\n");
	}

}


//处理错误信息
void Editor::dealWithError()
{
	map<int,int>::iterator it;
	for(it=closeCheckMap.begin();it!=closeCheckMap.end();++it)
	{
		if(it->second != 0 )
		{
			string kuohao ; 
			switch (it->first)
			{

			case 32:kuohao = "(";break;
			case 75:kuohao = "{";break;
			case 68:kuohao = "[";break;
			case 85:kuohao = "\"";break;
			case 86:kuohao = "\'";break;
			case 83:kuohao = "\\*";break;

			default:
				break;
			}
			addErrorInfo(-1,kuohao + " not match ");
		}
	}

}

//语法分析
void Editor::analyseSentence()
{
	tokenIndex = 0 ; 
	int startline = -1;
	int beforeLineNum = 0; 
	bool flag = false ;
	int needSemNum = -1 ; //需要分号的行数

	//主要分析
	while (tokenIndex < tokens.size())
	{
		string token = tokens[tokenIndex]; //符号
		int lineNum = lineNumber[tokenIndex]; //行号
		string index = chIndex[tokenIndex];//下标

		//如果是# 包含头文件 检查下一个字符是不是include 
		if(index == "77")
		{
			tokenIndex++ ; //取下一个token 验证是不是include 
			string index2 = chIndex[tokenIndex];//下标
			startline = lineNum ; 

			if(index2 != "78" && index2 != "81" && index2!= "80") //且不是宏定义
			{
				QString error = QStringLiteral("包含头文件错误");
				//cout << error << endl;
				addSentenceError(lineNum,error);
			}
		}
		//包含头文件之后 函数开始行
		if(startline != -1  &&  startline != lineNum)
		{
			//如果没有返回类型 
			if (index != "5" && index != "10" && 
				index != "13" && index != "17" && index != "18" 
				&& index != "21" && index != "29")
			{
				QString error = QStringLiteral("函数定义错误,缺少返回类型");
				//cout << error << endl;
				addSentenceError(lineNum,error);
			}
			startline = -1;
			beforeLineNum = lineNum + 1 ;  // 4 
		}
		//开始函数中的内容分析 
		else if(startline == -1)
		{
			//表示换行了
			if(beforeLineNum != lineNum)
			{
				//如果换行后不是第一个token不是int关键字
				if(index != "17")
				{
					flag = false ;
				}
				//如果换行后上一行没有分号 
				if(needSemNum > 0)
				{
					QString error = QStringLiteral( "语句没有终止符号 ; ");
					addSentenceError(needSemNum,error);
					needSemNum = -1;
				}


				beforeLineNum = lineNum;
			}
			//变量说明语句处理
			//int 类型定义 
			if(index == "17")
			{
				//如果是int 
				flag = true ; 
				needSemNum = lineNum ; //4行 需要 分号
			}
			//遇到 = ,表达式 则需要分号  
			if(index == "65")
			{
				needSemNum = lineNum ; 
			}
			//如果是在定义变量,并且有了分号结束语句 
			if(flag && ( needSemNum < 0 )) 
			{
				if(index == "100" )
				{
					QString error = QStringLiteral( "变量定义出现错误");
					addSentenceError(lineNum,error);
				}
			}
			//如果遇到表达式  以及 赋值  ,需要 换行符号 
			if(index == "74")
			{
				if(needSemNum == lineNum)
				{
					needSemNum = -1;	
				}
			}
			//如果是while 判断之后是不是(布尔表达式)
			if(index == "31")
			{
				bool boolFlag = false ; 
				int kuohao = 0 ; 
				while(!boolFlag)
				{
					tokenIndex++ ; 
					string index2 = chIndex[tokenIndex];//下标
					lineNum = lineNumber[tokenIndex];
					string token2 = tokens[tokenIndex]; //符号


					if(index2 == "32") //左括号识别
					{
						kuohao++;
					}
					else if(index2 == "33")
					{
						kuohao++;
						if(kuohao == 2)
							break ;
					}
					else if(index2 == "46")
					{
						boolFlag = true ;
					}
					else 
					{
						for(int i = 0; i < simbleList.size() ;i++)
						{
							if(simbleList[i].word == token2 && (simbleList[i].type == 1 ))
							{
								boolFlag = true ;
								break ;
							}

						}
					}

				}
				//没有判断
				if(!boolFlag){
					QString error = QStringLiteral( "while 判断出现错误 ");
					addSentenceError(lineNum,error);
				}



			}


		}

		tokenIndex++ ; 

	}

	showSentenceError();
}

//添加语法分析错误信息
void Editor::addSentenceError(int lineNum,QString error)
{
	if(lineNum != -1)
	{
		sentenceError.append(QString::fromStdString(intToString(lineNum))+ " : " +error);
	}
}

//语法分析错误
void Editor::showSentenceError()
{
	ui.errorInfo->clear();

	ui.errorInfo->insertPlainText(QStringLiteral("语法分析错误:")+"\n\n");
	for(int i  = 0 ; i < sentenceError.size() ; i++)
	{
		ui.errorInfo->insertPlainText(sentenceError.at(i)+"\n");
	}
}



string Editor::intToString(int inte)
{

	string str ;
	ostringstream oss;
	oss<<inte;
	str = oss.str();

	return str ;

}

string Editor::charToString(char c)
{
	string str;
	stringstream stream;
	stream << c;
	str = stream.str();
	return str; 
}

//判断是不是数字
bool Editor::isNum(string str)
{
	stringstream sin(str);  
	double d;  
	char c;  
	if(!(sin >> d))  
		return false;  
	if (sin >> c)  
		return false;  
	return true;  
}

