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
	connect(ui.cifafenxi,SIGNAL(triggered()),this,SLOT(analyseWord())); //�ʷ�����
	connect(ui.showSimbleList,SIGNAL(triggered()),this,SLOT(showSimbleList())); //��ʾ���ű�
	connect(ui.yufafenxi,SIGNAL(triggered()),this,SLOT(analyseSentence())); //�﷨����

}

Editor::~Editor()
{

}

//�ʷ�����
void Editor::analyseWord()
{

	QStringList content = ui.textEdit->toPlainText().split("\n");

	//ÿһ�з��� , ���ݿո�ָ�  
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

	//��ʾ����� 
	showAnalyseResult();
	dealWithError();

	showErrorList();
}
void Editor::findToken(string word,int lineNum,string lineStr)
{
	//��������Ƿ��ǺϷ���ʶ�� , 
	legalFlag = false ; 

	//������б�ע�� 
	if (memLine == lineNum)
	{
		return ; 
	}

	if(ifContinue)
	{
		if(identifierFlag)
		{
			string ident = word ; 
			//���»��ߺ���ĸ��ʼ w[a-zA-Z0-9_] 
			QRegExp reg("\\b[a-zA-Z_]\\w*");

			//�ȶԶ��Ž��д���
			if (word.length() > 1 && ( word[word.length() - 1] == ',' ||  word[word.length() - 1] == ';' ))
			{
				//���� int day, ��ȡ��day  
				ident =  word.substr(0,word.length() - 1);
			}
			bool match = reg.exactMatch(QString::fromStdString(ident));	

			if(match)
			{
				addResult(lineNum,ident,"100");
				addSimble(lineNum,ident); //���ӵ����ű���
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
				//����ǹؼ��� ��ô�ڵ�ǰ����ȫ��ƥ��  while 
				if(atoi( sql_row[1]) >= 0 && atoi( sql_row[1]) <= 31)
				{
					addResult(lineNum,sql_row[0],sql_row[1]);

					//����ǻ������� , ��identifierFlag = true ; 
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
				//��������� ������ŵ�
				else
				{
					//++   >=  -- 
					if( word.length() > 0)
					{
						string row = sql_row[0] ; 
						//����������ŵ�  ��ȫ��ƥ�� ����Ҫ����С������Щ
						if(row.length() > 1)
						{
							//����ǵ���ע�� 
							if(atoi(sql_row[1]) == 87)
							{
								//	addResult(lineNum,sql_row[0],sql_row[1]);

								memLine = lineNum ;  
								return  ; 
							}
							//����ע��
							if(atoi(sql_row[1]) == 83)
							{
								addResult(lineNum,sql_row[0],sql_row[1]);

								closeCheckMap[83]++;
								ifContinue = false ; 
								break;
							}

							//++ �� -- �� >= 
							addResult(lineNum,sql_row[0],sql_row[1]);

						}

						//С���� ������ "
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

	// ����������ʽ��  (9    ���� ++ �����Ѿ�ƥ����� �����ֻ����ƥ��  
	if(word.length() > 1 )
	{ 
		for (int i = 0; i < word.length(); i++)
		{
			//������ڶ���ע�ͷ�Χ
			if(ifContinue)
			{
				//����#��ʼ, �����Ǻ궨�� 
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
				//	����ǵ���ע��
				if( i == 0 &&  ( word[i] == '/') && word[1] == '/')
				{
					//	addResult(lineNum,"//","87");
					memLine = lineNum ;  
					break ; //ֱ���˳�ѭ��
				}	

				//����Ƕ���ע�� 
				if( i == 0 &&  ( word[i] == '/') && word[1] == '*')
				{
					addResult(lineNum,"/*","83");

					closeCheckMap[83] ++ ;

					ifContinue = false ;

				}	
			}

			//����ע�ͽ���
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

			//����ע��֮�����ƥ��  
			if(i < word.length() && ifContinue)
			{
				//���Ǻ궨��
				//Ҫ���� _++_�������� �е��Ѿ�ƥ�����,���������ƥ��
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
						//�� ++ --  
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
								//��Ȼֻ�ѳ�һ����¼
								while (	sql_row3 = mysql_fetch_row(res3))
								{
									addResult(lineNum,sql_row3[0],sql_row3[1]);

									singleFlag = false ; 
									i = i + 2 ;
								}
							}
						}

						//ֻ�� +  ���� ++ 
						if(singleFlag)
						{
							if(atoi(sql_row1[1]) == 74)  //�� ; ��ֹ��ʶ������  
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
		//δʶ�𵽵ı�ʶ�����еڶ���ɸѡ 
		//�����жϱ�ʶ���Ƿ��Ѿ���ʶ��  
		for(int i = 0 ; i < simbleList.size(); i ++)
		{
			if (simbleList.at(i).word.compare(word) ==  0)
			{
				//������ڷ��ű����Ѿ�����  ֱ�ӷ���
				addResult(lineNum,word,"100");
				return ;
			}
		}
		//��δʶ��ı�ʶ��,��Ҫ�ж��Ƿ���int ���͸�ֵ���� 
		if(isNum(word))
		{
			addResult(lineNum,word,"101");
		}
		//�����δʶ��  ���Ǵ�����Ϣ 
		else if(word.length() > 0)
		{
			//����ǺϷ��ַ� Ҳ����token��
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


//����ƥ��� 
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

//����һ�����
void Editor::addResult(int lineNum,string ch,string index)
{
	legalFlag = true  ;//word �Ѿ���ʶ��,�ǺϷ��� 
	string res = intToString(lineNum) + ":	" +ch+ "	" + index;
	analysisResult.append((QString::fromStdString(res)));
	tokens.push_back(ch);
	lineNumber.push_back(lineNum);
	chIndex.push_back(index);
}

//��Ӵ�����Ϣ
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


//���ű�������һ������
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



//��ʾ���ű�
void Editor::showSimbleList()
{
	ui.tokenInfo->clear();

	ui.tokenInfo->insertPlainText(QStringLiteral("���  ����  ����   ����   ����    ֵ   �ڴ��ַ\n"));

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

//��ʾ�������
void Editor::showAnalyseResult()
{

	ui.tokenInfo->clear();

	analysisResult.insert(0,QString("row") + ":	" + "token	 index");
	for(int i = 0 ; i != analysisResult.size() ; i++)
	{
		ui.tokenInfo->insertPlainText(QString(analysisResult.at(i))+"\n");
	}
}

//��ʾ������Ϣ
void Editor::showErrorList()
{
	ui.errorInfo->clear();

	for(int i  = 0 ; i < errorInfoList.size() ; i++)
	{
		ui.errorInfo->insertPlainText(QString(errorInfoList.at(i))+"\n");
	}

}


//���������Ϣ
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

//�﷨����
void Editor::analyseSentence()
{
	tokenIndex = 0 ; 
	int startline = -1;
	int beforeLineNum = 0; 
	bool flag = false ;
	int needSemNum = -1 ; //��Ҫ�ֺŵ�����

	//��Ҫ����
	while (tokenIndex < tokens.size())
	{
		string token = tokens[tokenIndex]; //����
		int lineNum = lineNumber[tokenIndex]; //�к�
		string index = chIndex[tokenIndex];//�±�

		//�����# ����ͷ�ļ� �����һ���ַ��ǲ���include 
		if(index == "77")
		{
			tokenIndex++ ; //ȡ��һ��token ��֤�ǲ���include 
			string index2 = chIndex[tokenIndex];//�±�
			startline = lineNum ; 

			if(index2 != "78" && index2 != "81" && index2!= "80") //�Ҳ��Ǻ궨��
			{
				QString error = QStringLiteral("����ͷ�ļ�����");
				//cout << error << endl;
				addSentenceError(lineNum,error);
			}
		}
		//����ͷ�ļ�֮�� ������ʼ��
		if(startline != -1  &&  startline != lineNum)
		{
			//���û�з������� 
			if (index != "5" && index != "10" && 
				index != "13" && index != "17" && index != "18" 
				&& index != "21" && index != "29")
			{
				QString error = QStringLiteral("�����������,ȱ�ٷ�������");
				//cout << error << endl;
				addSentenceError(lineNum,error);
			}
			startline = -1;
			beforeLineNum = lineNum + 1 ;  // 4 
		}
		//��ʼ�����е����ݷ��� 
		else if(startline == -1)
		{
			//��ʾ������
			if(beforeLineNum != lineNum)
			{
				//������к��ǵ�һ��token����int�ؼ���
				if(index != "17")
				{
					flag = false ;
				}
				//������к���һ��û�зֺ� 
				if(needSemNum > 0)
				{
					QString error = QStringLiteral( "���û����ֹ���� ; ");
					addSentenceError(needSemNum,error);
					needSemNum = -1;
				}


				beforeLineNum = lineNum;
			}
			//����˵����䴦��
			//int ���Ͷ��� 
			if(index == "17")
			{
				//�����int 
				flag = true ; 
				needSemNum = lineNum ; //4�� ��Ҫ �ֺ�
			}
			//���� = ,���ʽ ����Ҫ�ֺ�  
			if(index == "65")
			{
				needSemNum = lineNum ; 
			}
			//������ڶ������,�������˷ֺŽ������ 
			if(flag && ( needSemNum < 0 )) 
			{
				if(index == "100" )
				{
					QString error = QStringLiteral( "����������ִ���");
					addSentenceError(lineNum,error);
				}
			}
			//����������ʽ  �Լ� ��ֵ  ,��Ҫ ���з��� 
			if(index == "74")
			{
				if(needSemNum == lineNum)
				{
					needSemNum = -1;	
				}
			}
			//�����while �ж�֮���ǲ���(�������ʽ)
			if(index == "31")
			{
				bool boolFlag = false ; 
				int kuohao = 0 ; 
				while(!boolFlag)
				{
					tokenIndex++ ; 
					string index2 = chIndex[tokenIndex];//�±�
					lineNum = lineNumber[tokenIndex];
					string token2 = tokens[tokenIndex]; //����


					if(index2 == "32") //������ʶ��
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
				//û���ж�
				if(!boolFlag){
					QString error = QStringLiteral( "while �жϳ��ִ��� ");
					addSentenceError(lineNum,error);
				}



			}


		}

		tokenIndex++ ; 

	}

	showSentenceError();
}

//����﷨����������Ϣ
void Editor::addSentenceError(int lineNum,QString error)
{
	if(lineNum != -1)
	{
		sentenceError.append(QString::fromStdString(intToString(lineNum))+ " : " +error);
	}
}

//�﷨��������
void Editor::showSentenceError()
{
	ui.errorInfo->clear();

	ui.errorInfo->insertPlainText(QStringLiteral("�﷨��������:")+"\n\n");
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

//�ж��ǲ�������
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

