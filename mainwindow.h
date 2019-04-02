#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "hardware/lmx2326.h"
#include "hardware/controllers/slimusb.h"
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutexLocker>
#include "shared/comprotocol.h"
#include "QSettings"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	quint16 getServerPort() const;
	void setServerPort(const quint16 &value);

private slots:
	void on_pushButton_clicked();
	void dataReady(quint32, quint32, quint32);
	void on_Connect();
	void on_Disconnect();
	void newConnection();
	void onMessageReceivedServer(ComProtocol::messageType, QByteArray);
private:
	Ui::MainWindow *ui;
	QHash<msa::MSAdevice, int> devices;
	interface *hwInterface;
	QMutex mutex;
	QMutex messageSend;
	bool isConnected;

	quint16 serverPort;
	ComProtocol *server;

	QSettings *settings;
};

#endif // MAINWINDOW_H
