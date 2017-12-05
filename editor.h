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
	QString openPath ; //���ļ���·��
	QString savePath ;// �����ļ�·�� 
	QStringList lineList;  
	QStringList analysisResult; 
	DataBase dbms; 
	MYSQL_RES *result ; 
	map<int,int> closeCheckMap ;
	bool addFlag1 ; // �ж� "" ǰ��Ҫ�õ�
	bool addFlag2 ; 
	int memLine ; //����ע��
	bool ifContinue ; //����ע��
	vector<SimbleTable> simbleList ; //���ű� 
	bool identifierFlag ; //���ڶ������ 
	QStringList errorInfoList ;//������Ϣ�б� 
	bool legalFlag ; //��ʶ���Ƿ�Ϸ�
	int simbleType ; 
	vector<string> tokens;
	vector<int> lineNumber ;
	vector<string> chIndex ;
	QStringList sentenceError ;//�﷨���������б�

	private slots:
		//���ļ�
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
				//���ô��������м��
				QTextBlockFormat format;
				format.setLineHeight(17,2);
				while (!in.atEnd()) {
					QString line = in.readLine();
					//�к�
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

		//�����к� 
		void on_textEdit_cursorPositionChanged()  
		{  
			//��ǰ���  
			QTextCursor tc = ui.textEdit->textCursor();   
			QTextLayout *pLayout = tc.block().layout();  
			//��ǰ����ڱ�BLOCK�ڵ����λ��  
			int nCurpos = tc.position()  - tc.block().position() ;  
			int nTextline = pLayout->lineForTextPosition(nCurpos).lineNumber() + tc.block().firstLineNumber() + 1 ;  
			cout<<nTextline<<endl;           //���Կ����к����Ź��ĸı���ı�  
		}  

		//�����ļ�
		void saveFile()
		{
			savePath = QFileDialog::getSaveFileName(this, tr("Save File"), QStringLiteral("δ����"), tr(".txt"));
			QFile file(savePath);  

			if (file.open(QFile::WriteOnly | QIODevice::Truncate)) {
				QTextStream out(&file);

				//д���ļ�
				out << ui.textEdit->toPlainText() ;
			}
		}

		//�ʷ�����
		void analyseWord(); 
		void showSimbleList();
		//�﷨����
		void analyseSentence();

private :
		//token����Ϣ
		void findToken(string word,int lineNum,string lineStr);

		//int ת string
		string intToString(int inte);

		void showAnalyseResult();

		string charToString(char c);

		//��ʼ������ ����map
		void initCloseCheckMap();

		//�ʷ�����������
		void dealWithError();

		//�������ƥ�� 
		void checkMatch(int index);

		//����һ�����
		void addResult(int lineNum,string ch,string index);
		void addErrorInfo(int lineNum,string error); 

		//����ű������ӷ���
		void addSimble(int lineNum , string ident);
	
		int tokenIndex;
		bool isNum(string str) ;
		void showErrorList();

		void addSentenceError(int lineNum,QString error);
		void showSentenceError();
};

#endif // EDITOR_H
