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
#include "helperform.h"
#include <QSystemTrayIcon>
#include "calparser.h"
#include "calibrationviewer.h"
#include "hardwareconfigwidget.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QDialog>

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
QT_END_NAMESPACE

//! [0]
//!
#else // QT_NO_SYSTEMTRAYICON


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();
private:
	Ui::MainWindow *ui;
#endif
#ifndef QT_NO_SYSTEMTRAYICON
class MainWindow : public QDialog
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();
	void setVisible(bool visible) override;

protected:
	void closeEvent(QCloseEvent *event) override;
	typedef enum {INFO, WARNING, ERROR} message_type;

private slots:
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
	void showMessage(int type, QString title, QString text, int duration);
	void messageClicked();

private:
	void createMessageGroupBox();
	void createActions();
	void createTrayIcon();

	QLabel *iconLabel;

	QGroupBox *messageGroupBox;
	QLabel *typeLabel;
	QLabel *durationLabel;
	QLabel *durationWarningLabel;
	QLabel *titleLabel;
	QLabel *bodyLabel;
	QComboBox *typeComboBox;
	QSpinBox *durationSpinBox;
	QLineEdit *titleEdit;
	QTextEdit *bodyEdit;
	QPushButton *showMessageButton;

	QAction *minimizeAction;
	QAction *showLogAction;
	QAction *showCalibrationAction;
	QAction *maximizeAction;
	QAction *restoreAction;
	QAction *quitAction;

	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;

#endif

signals:
	void triggerMessage(int type, QString title, QString text, int duration);
private slots:
	void on_pushButton_clicked();
	void dataReady(quint32, quint32, quint32);
	void on_Connect();
	void on_Disconnect();
	void newConnection();
	void onMessageReceivedServer(ComProtocol::messageType, QByteArray);
	void interfaceError(QString, bool, bool);
	void showCalibration();
private:
	void loadCalibrationFiles(msa::scanConfig *config);
	void startServer(hardwareConfigWidget::appSettings_t &settings);
	void loadHardware(hardwareConfigWidget::appSettings_t &settings);
	calParser m_calParser;
	HelperForm *logForm;
	QHash<msa::MSAdevice, int> devices;
	interface *hwInterface;
	QMutex mutex;
	QMutex messageSend;
	bool isConnected;

	ComProtocol *server;

	hardwareConfigWidget *configurator;
	void start();
};
#endif // MAINWINDOW_H
