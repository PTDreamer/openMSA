#include "mainwindow.h"
#include <QApplication>

#ifndef QT_NO_SYSTEMTRAYICON
#include <QMessageBox>
int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(systray);

	QApplication app(argc, argv);

	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(nullptr, QObject::tr("Systray"),
							  QObject::tr("I couldn't detect any system tray "
										  "on this system."));
		return 1;
	}
	QApplication::setQuitOnLastWindowClosed(false);

	MainWindow window;
	return app.exec();
}

#else

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}

#endif
