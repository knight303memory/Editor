#ifndef EDITOR_H
#define EDITOR_H

#include <QtWidgets/QMainWindow>
#include "ui_editor.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <iostream>
#include <QTextStream>
#include <QFile>
#include <QPainter>
#include <QTextLine>
#include <QStandardItemModel>
#include "DataBase.h"
#include <sstream>
#include <map>
#include <vector>
#include "SimbleTable.h"
using namespace std;
class Editor : public QMainWindow
{
	Q_OBJECT

public:
	Editor(QWidget *parent = 0);
	~Editor();

private:
	Ui::EditorClass ui;
	QString openPath ; //打开文件的路径
	QString savePath ;// 保存文件路径 
	QStringList lineList;  
	QStringList analysisResult; 
	DataBase dbms; 
	MYSQL_RES *result ; 
	map<int,int> closeCheckMap ;
	bool addFlag1 ; // 判断 "" 前后要用到
	bool addFlag2 ; 
	int memLine ; //多行注释
	bool ifContinue ; //多行注释
	vector<SimbleTable> simbleList ; //符号表 
	bool identifierFlag ; //是在定义变量 
	QStringList errorInfoList ;//错误信息列表 
	bool legalFlag ; //标识符是否合法
	int simbleType ; 
	vector<string> tokens;
	vector<int> lineNumber ;
	vector<string> chIndex ;
	QStringList sentenceError ;//语法分析错误列表

	private slots:
		//打开文件
		void openFile()
		{
			openPath = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("*")); 
			if(openPath.length() == 0) { 
				QMessageBox::information(NULL, tr("Path"), tr("You didn't select any files.")); 
			} else { 
				QFile file(openPath);  
				if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  
					return;  
				QTextStream in(&file);
				ui.textEdit->setText("");
				int lineNum = 1 ; 
				//设置代码区域行间隔
				QTextBlockFormat format;
				format.setLineHeight(17,2);
				while (!in.atEnd()) {
					QString line = in.readLine();
					//行号
					lineList.append(QString::number(lineNum));
					lineNum++;

					string str = line.toStdString();
					ui.textEdit->insertPlainText(line+"\n");
					ui.textEdit->textCursor().setBlockFormat(format);

				}

			} 

			showLineNum();
		}

		void showLineNum()
		{

			QStandardItemModel  *standardItemModel = new QStandardItemModel(this);  
			for(int i = 0 ; i != lineList.size() ; i++)
			{
				QStandardItem *item = new QStandardItem(lineList.at(i));
				standardItemModel->appendRow(item);  
			}
			ui.lineNum ->setModel(standardItemModel);  
			connect(ui.textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(on_textEdit_cursorPositionChanged()));  

		}

		//计算行号 
		void on_textEdit_cursorPositionChanged()  
		{  
			//当前光标  
			QTextCursor tc = ui.textEdit->textCursor();   
			QTextLayout *pLayout = tc.block().layout();  
			//当前光标在本BLOCK内的相对位置  
			int nCurpos = tc.position()  - tc.block().position() ;  
			int nTextline = pLayout->lineForTextPosition(nCurpos).lineNumber() + tc.block().firstLineNumber() + 1 ;  
			cout<<nTextline<<endl;           //可以看到行号随着光标的改变而改变  
		}  

		//保存文件
		void saveFile()
		{
			savePath = QFileDialog::getSaveFileName(this, tr("Save File"), QStringLiteral("未命名"), tr(".txt"));
			QFile file(savePath);  

			if (file.open(QFile::WriteOnly | QIODevice::Truncate)) {
				QTextStream out(&file);

				//写入文件
				out << ui.textEdit->toPlainText() ;
			}
		}

		//词法分析
		void analyseWord(); 
		void showSimbleList();
		//语法分析
		void analyseSentence();

private :
		//token表信息
		void findToken(string word,int lineNum,string lineStr);

		//int 转 string
		string intToString(int inte);

		void showAnalyseResult();

		string charToString(char c);

		//初始化括号 引号map
		void initCloseCheckMap();

		//词法分析错误处理
		void dealWithError();

		//检查括号匹配 
		void checkMatch(int index);

		//插入一条结果
		void addResult(int lineNum,string ch,string index);
		void addErrorInfo(int lineNum,string error); 

		//向符号表中增加符号
		void addSimble(int lineNum , string ident);
	
		int tokenIndex;
		bool isNum(string str) ;
		void showErrorList();

		void addSentenceError(int lineNum,QString error);
		void showSentenceError();
};

#endif // EDITOR_H
