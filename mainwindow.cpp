#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hardware/lmx2326.h"
#include <QDebug>
#include "hardware/hardwaredevice.h"
#include "hardware/ad9850.h"
#include "hardware/controllers/slimusb.h"
#include "hardware/controllers/simulator.h"
#include "hardware/msa.h"
#include <QMessageBox>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QCloseEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMessageBox>

#include <QDir>

//! [0]
MainWindow::MainWindow():hwInterface(nullptr), server(nullptr)
{
	logForm = new HelperForm();

	connect(this, &MainWindow::triggerMessage, this, &MainWindow::showMessage);
	connect(this, &MainWindow::triggerMessage, logForm, &HelperForm::showMessage);
	createMessageGroupBox();
	iconLabel = new QLabel("Icon:");

	iconLabel->setMinimumWidth(durationLabel->sizeHint().width());

	configurator = new hardwareConfigWidget();
	createActions();
	createTrayIcon();
	connect(configurator, &hardwareConfigWidget::triggerMessage, this, &MainWindow::triggerMessage);
	connect(configurator, &hardwareConfigWidget::requiresReinit, this, &MainWindow::scanReinit);
	connect(configurator, &hardwareConfigWidget::requiresHwReinit, this, &MainWindow::hwReinit);
	//connect(showMessageButton, &QAbstractButton::clicked, this, &MainWindow::showMessage);
	connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::messageClicked);
	connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);
	trayIconTimer = new QTimer;
	trayIconTimer->setSingleShot(true);
	connect(trayIconTimer, &QTimer::timeout, this, &MainWindow::trayIconTimerCallback);
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(messageGroupBox);
	setLayout(mainLayout);

	QIcon icon = QIcon(":/images/smith_chart_red.png");
	trayIcon->setIcon(icon);
	setWindowIcon(icon);

	trayIcon->show();

	setWindowTitle(tr("Systray"));
	resize(400, 300);

	setVisible(false);

	configurator->loadSavedSettings();
	configurator->loadSettingsToGui();

	start();
}


void MainWindow::setVisible(bool visible)
{
	minimizeAction->setEnabled(visible);
	maximizeAction->setEnabled(!isMaximized());
	restoreAction->setEnabled(isMaximized() || !visible);
	QDialog::setVisible(visible);
}
//! [1]

//! [2]
void MainWindow::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_OSX
	if (!event->spontaneous() || !isVisible()) {
		return;
	}
#endif
	if (trayIcon->isVisible()) {
		QMessageBox::information(this, tr("Systray"),
								 tr("The program will keep running in the "
									"system tray. To terminate the program, "
									"choose <b>Quit</b> in the context menu "
									"of the system tray entry."));
		hide();
		event->ignore();
	}
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason) {
	case QSystemTrayIcon::Trigger:
	case QSystemTrayIcon::DoubleClick:
		break;
	case QSystemTrayIcon::MiddleClick:
//		showMessage();
		break;
	default:
		;
	}
}
//! [4]

//! [5]
void MainWindow::showMessage(int type, QString title, QString text, int duration)
{
	qDebug() << title << text << duration << trayMessagesList.length();
	if(trayIconTimer->isActive()) {
		trayMessages t;
		t.type =type;
		t.title = title;
		t.text = text;
		t.duration = duration;
		trayMessagesList.append(t);
		trayIconTimer->start(duration * 1000);
		return;
	}
	trayIconTimer->start(duration * 1000);
	QSystemTrayIcon::MessageIcon msgIcon = QSystemTrayIcon::Information;
	switch (type) {
	case INFO:
		msgIcon = QSystemTrayIcon::Information;
		break;
	case WARNING:
		msgIcon = QSystemTrayIcon::Warning;
		break;
	case ERROR:
		msgIcon = QSystemTrayIcon::Critical;
		break;
	}
	trayIcon->showMessage(title, text , msgIcon, duration * 1000);
}
//! [5]

//! [6]
void MainWindow::messageClicked()
{
	QMessageBox::information(nullptr, tr("Systray"),
							 tr("Sorry, I already gave what help I could.\n"
								"Maybe you should try asking a human?"));
}
//! [6]

void MainWindow::createMessageGroupBox()
{
	messageGroupBox = new QGroupBox(tr("Balloon Message"));

	typeLabel = new QLabel(tr("Type:"));

	typeComboBox = new QComboBox;
	typeComboBox->addItem(tr("None"), QSystemTrayIcon::NoIcon);
	typeComboBox->addItem(style()->standardIcon(
			QStyle::SP_MessageBoxInformation), tr("Information"),
			QSystemTrayIcon::Information);
	typeComboBox->addItem(style()->standardIcon(
			QStyle::SP_MessageBoxWarning), tr("Warning"),
			QSystemTrayIcon::Warning);
	typeComboBox->addItem(style()->standardIcon(
			QStyle::SP_MessageBoxCritical), tr("Critical"),
			QSystemTrayIcon::Critical);
	typeComboBox->addItem(QIcon(), tr("Custom icon"),
			QSystemTrayIcon::NoIcon);
	typeComboBox->setCurrentIndex(1);

	durationLabel = new QLabel(tr("Duration:"));

	durationSpinBox = new QSpinBox;
	durationSpinBox->setRange(5, 60);
	durationSpinBox->setSuffix(" s");
	durationSpinBox->setValue(15);

	durationWarningLabel = new QLabel(tr("(some systems might ignore this "
										 "hint)"));
	durationWarningLabel->setIndent(10);

	titleLabel = new QLabel(tr("Title:"));

	titleEdit = new QLineEdit(tr("Cannot connect to network"));

	bodyLabel = new QLabel(tr("Body:"));

	bodyEdit = new QTextEdit;
	bodyEdit->setPlainText(tr("Don't believe me. Honestly, I don't have a "
							  "clue.\nClick this balloon for details."));

	showMessageButton = new QPushButton(tr("Show Message"));
	showMessageButton->setDefault(true);

	QGridLayout *messageLayout = new QGridLayout;
	messageLayout->addWidget(typeLabel, 0, 0);
	messageLayout->addWidget(typeComboBox, 0, 1, 1, 2);
	messageLayout->addWidget(durationLabel, 1, 0);
	messageLayout->addWidget(durationSpinBox, 1, 1);
	messageLayout->addWidget(durationWarningLabel, 1, 2, 1, 3);
	messageLayout->addWidget(titleLabel, 2, 0);
	messageLayout->addWidget(titleEdit, 2, 1, 1, 4);
	messageLayout->addWidget(bodyLabel, 3, 0);
	messageLayout->addWidget(bodyEdit, 3, 1, 2, 4);
	messageLayout->addWidget(showMessageButton, 5, 4);
	messageLayout->setColumnStretch(3, 1);
	messageLayout->setRowStretch(4, 1);
	messageGroupBox->setLayout(messageLayout);
}

void MainWindow::createActions()
{
	showConfigAction = new QAction(tr("Show &Log"), this);
	connect(showConfigAction, &QAction::triggered, logForm, &QWidget::show);

	showLogAction = new QAction(tr("Show &Configuration"), this);
	connect(showLogAction, &QAction::triggered, configurator, &QWidget::show);

	showCalibrationAction = new QAction(tr("Show Ca&libration graphs"), this);
	connect(showCalibrationAction, &QAction::triggered, this, &MainWindow::showCalibration);

	minimizeAction = new QAction(tr("Mi&nimize"), this);
	connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

	maximizeAction = new QAction(tr("Ma&ximize"), this);
	connect(maximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

	restoreAction = new QAction(tr("&Restore"), this);
	connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

	quitAction = new QAction(tr("&Quit"), this);
	connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void MainWindow::createTrayIcon()
{
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(showConfigAction);
	trayIconMenu->addAction(showLogAction);
	trayIconMenu->addAction(showCalibrationAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(minimizeAction);
	trayIconMenu->addAction(maximizeAction);
	trayIconMenu->addAction(restoreAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);

	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setContextMenu(trayIconMenu);
}

#else

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	start();
}
#endif
void MainWindow::start() {
	isConnected = false;
	using namespace std::placeholders; // for `_1`

	msa::getInstance().addScanConfigChangedCallback(std::bind(&MainWindow::msaScanConfigChanged, this, _1));

	//msa::getInstance().addScanConfigChangedCallback(fnc_ptr);
	msa::getInstance().setMainWindow(this);
	msa::getInstance().currentScan.steps = new QHash<quint32, msa::scanStep>();
	hardwareConfigWidget::appSettings_t appSettings = configurator->getAppSettings();
	if(!server)
		startServer(appSettings);
	loadHardware(appSettings);
	msa::scanConfig config = configurator->getConfig();
	msa::getInstance().setScanConfiguration(config);
	connect(hwInterface,SIGNAL(connected()), this,SLOT(on_Connect()), Qt::UniqueConnection);
	connect(hwInterface,SIGNAL(disconnected()), this,SLOT(on_Disconnect()), Qt::UniqueConnection);
	if(hwInterface->getIsConnected())
		on_Connect();
}

void MainWindow::msaScanConfigChanged(msa::scanConfig config)
{
	qDebug() << "scan config changed";
	ComProtocol::msg_scan_config m_config;
	m_config = config.gui;
	if(server)
		server->sendMessage(ComProtocol::SCAN_CONFIG, ComProtocol::MESSAGE_SEND, &m_config);
}

MainWindow::~MainWindow()
{
#ifdef QT_NO_SYSTEMTRAYICON
	delete ui;
#endif
	delete configurator;
	delete msa::getInstance().currentScan.steps;
	logForm->deleteLater();
}

void MainWindow::on_pushButton_clicked()
{
}

void MainWindow::dataReady(quint32 step, quint32 mag, quint32 phase)
{
	if(msa::getInstance().currentInterface->getDebugLevel() > 2)
		qDebug() << "received step:" << step << "MAG=" << mag << "PHASE=" << phase;
	if(server->isConnected()) {
        ComProtocol::msg_dual_dac dac;
		dac.mag = msa::getInstance().currentScan.configuration.pathCalibration.adcToMagCalFactors.value(mag).dbm_val;
		dac.phase = msa::getInstance().currentScan.configuration.pathCalibration.adcToMagCalFactors.value(mag).phase_val;
		dac.mag += msa::getInstance().currentScan.configuration.frequencyCalibration.freqToPower.value(msa::getInstance().currentScan.steps->value(step).realFrequency);
		dac.step = step;
		QMutexLocker locker(&messageSend);
        server->sendMessage(ComProtocol::DUAL_DAC, ComProtocol::MESSAGE_SEND, &dac);
    }
}

void MainWindow::on_Connect()
{
	QMutexLocker locker(&mutex);
	if(isConnected)
		return;
	msa::scanConfig cfg;
	cfg = msa::getInstance().getScanConfiguration();
	msa::getInstance().hardwareInit(devices, hwInterface);
	msa::getInstance().initScan(cfg.gui.isInvertedScan, cfg.gui.start, cfg.gui.stop, cfg.gui.steps_number, cfg.gui.band);

	hwInterface->autoScan();
	isConnected = true;
}

void MainWindow::on_Disconnect()
{
	isConnected = false;
}

void MainWindow::newConnection()
{	
	emit triggerMessage(INFO, "Server", "New client connected", 3);
	ComProtocol::msg_scan_config cfg_msg;
	msa::scanConfig config = msa::getInstance().getScanConfiguration();
	cfg_msg = config.gui;
	cfg_msg.scanType = ComProtocol::scanType_t(config.scanType);
	QMutexLocker locker(&messageSend);
	server->sendMessage(ComProtocol::SCAN_CONFIG, ComProtocol::MESSAGE_SEND_REQUEST_ACK, &cfg_msg);
	ComProtocol::msg_final_filter fil_msg;
	foreach (calParser::magPhaseCalData d, config.pathCalibrationList) {
		strcpy(fil_msg.name, d.pathName.toLatin1().data());
		server->sendMessage(ComProtocol::FINAL_FILTER, ComProtocol::MESSAGE_SEND_REQUEST_ACK, &fil_msg);
	}
}

void MainWindow::scanReinit()//used for special tests
{
	ComProtocol::msg_scan_config m_config;
	msa::scanConfig config = configurator->getConfig();
	msa::getInstance().setScanConfiguration(config);
	config = msa::getInstance().getScanConfiguration();
	m_config = config.gui;
	bool ok;
	if(m_config.isStepInSteps)
		ok = msa::getInstance().initScan(m_config.isInvertedScan,  m_config.start, m_config.stop, m_config.steps_number, m_config.band);
	else {
		ok = msa::getInstance().initScan(m_config.isInvertedScan,  m_config.start, m_config.stop, m_config.step_freq, m_config.band);
	}
}

void MainWindow::hwReinit()
{
	start();
}

void MainWindow::trayIconTimerCallback()
{
	if(trayMessagesList.length() == 0)
		return;
	trayMessages t = trayMessagesList.last();
	trayMessagesList.removeLast();
	showMessage(t.type, t.title, t.text, t.duration);
}

void MainWindow::onMessageReceivedServer(ComProtocol::messageType type, QByteArray data)
{
	static quint32 lastMessage = 0;
	ComProtocol::msg_scan_config m_config;
	ComProtocol::messageCommandType command;
	quint32 msgNumber = 0;
	switch (type) {
		case ComProtocol::SCAN_CONFIG:
			server->unpackMessage(data, type, command, msgNumber, &m_config);
		break;
	}
	msa::scanConfig config = msa::getInstance().getScanConfiguration();
	config.scanType = m_config.scanType;
	config.gui = m_config;
	msa::getInstance().setScanConfiguration(config);
	bool ok;
	if(m_config.isStepInSteps)// TODO HANDLE m_config.stepModeAuto
		ok = msa::getInstance().initScan(m_config.isInvertedScan,  m_config.start, m_config.stop, m_config.steps_number, m_config.band);
	else {
		ok = msa::getInstance().initScan(m_config.isInvertedScan,  m_config.start, m_config.stop, m_config.step_freq, m_config.band);
	}
	//msa::getInstance().currentInterface->autoScan();
	//if(ok)
		//server->sendMessage(ComProtocol::SCAN_CONFIG, ComProtocol::MESSAGE_SEND, &m_config);

//		emit scanConfigChanged(config);
	if(lastMessage != 0) {
		//Q_ASSERT(msgNumber - lastMessage == 1);
	}
	lastMessage = msgNumber;
}

void MainWindow::interfaceError(QString text, bool critical, bool sendToGui)
{
	emit triggerMessage(ERROR, "Error", text, 3);
	if(sendToGui) {
		ComProtocol::msg_error_info msg;
		msg.isCritical = critical;
		strcpy(msg.text, text.toLatin1().constData());
		server->sendMessage(ComProtocol::ERROR_INFO, ComProtocol::MESSAGE_SEND, &msg);
	}
}

void MainWindow::showCalibration()
{
#ifndef NO_CHARTS
	calibrationViewer *v = new calibrationViewer();
	v->show();
#endif
}

void MainWindow::startServer(hardwareConfigWidget::appSettings_t &appSettings)
{
	//DEBUG
	server = new ComProtocol(this, appSettings.debugLevel);
	server->setServerPort(appSettings.serverPort);
	if(!server->startServer())
		emit triggerMessage(WARNING, "", QString("Socket server failed to start on port %1, Please fix the issue and restart the application").arg(appSettings.serverPort), 5);
	connect(server, &ComProtocol::serverConnected, this, &MainWindow::newConnection, Qt::UniqueConnection);
	connect(server, &ComProtocol::packetReceived, this, &MainWindow::onMessageReceivedServer, Qt::UniqueConnection);

}

void MainWindow::loadHardware(hardwareConfigWidget::appSettings_t &settings)
{
	if(hwInterface)
		delete hwInterface;
	if(settings.currentInterfaceType == interface::SIMULATOR)
		hwInterface = new simulator(this);
	else if (settings.currentInterfaceType == interface::USB) {
		hwInterface = new slimusb(this);
	}
	else {
		qDebug() << settings.currentInterfaceType;
		Q_ASSERT(false);
	}
	connect(hwInterface, SIGNAL(dataReady(quint32,quint32,quint32)), this, SLOT(dataReady(quint32, quint32, quint32)), Qt::UniqueConnection);
	connect(hwInterface, &interface::errorTriggered, this, &MainWindow::interfaceError, Qt::UniqueConnection);
	hwInterface->setWriteReadDelay_us(settings.readWriteDelay);
	devices.clear();
	foreach (msa::MSAdevice dev, settings.devices.keys()) {
		devices.insert(dev, settings.devices.value(dev));
	}
	hwInterface->init(settings.debugLevel);
}
