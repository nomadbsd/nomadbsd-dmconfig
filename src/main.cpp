#include <QApplication>
#include <QTranslator>
#include "mainwin.h"
#include "defs.h"

int
main(int argc, char *argv[])
{
	QApplication app(argc, argv);  
	QTranslator translator;
	if (translator.load(QLocale(), QLatin1String(PROGRAM),
	    QLatin1String("_"), QLatin1String(":/i18n")))
		qApp->installTranslator(&translator);
	MainWin *win = new MainWin;
	win->show();

	return (app.exec());
}

