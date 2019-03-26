#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "hardware/lmx2326.h"
#include "hardware/controllers/slimusb.h"
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutexLocker>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_pushButton_clicked();
	void dataReady(quint32, quint32, quint32);
	void on_Connect();
	void on_Disconnect();
	void newConnection();
private:
	Ui::MainWindow *ui;
	QHash<msa::MSAdevice, int> devices;
	slimusb *s;
	QTcpServer *server;
	QTcpSocket *socket;
	QMutex mutex;
	bool isConnected;
};

#endif // MAINWINDOW_H
