#include "editor.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	Editor w;
	w.show();
	return a.exec();
}
