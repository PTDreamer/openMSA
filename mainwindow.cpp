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

//! [0]
MainWindow::MainWindow()
{
	createMessageGroupBox();
	iconLabel = new QLabel("Icon:");

	iconLabel->setMinimumWidth(durationLabel->sizeHint().width());

	createActions();
	createTrayIcon();

	connect(showMessageButton, &QAbstractButton::clicked, this, &MainWindow::showMessage);
	connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::messageClicked);
	connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);

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
	start();
}
//! [0]

//! [1]
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
		showMessage();
		break;
	default:
		;
	}
}
//! [4]

//! [5]
void MainWindow::showMessage()
{
	QSystemTrayIcon::MessageIcon msgIcon = QSystemTrayIcon::MessageIcon(
			typeComboBox->itemData(typeComboBox->currentIndex()).toInt());
		trayIcon->showMessage(titleEdit->text(), bodyEdit->toPlainText(), msgIcon,
						  durationSpinBox->value() * 1000);
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
	msa::getInstance().currentScan.steps = new QHash<quint32, msa::scanStep>();
//	hardwareDevice::scanStruct scan;
//	scan.configuration.LO2 = 1024;
//	scan.configuration.appxdds1 = 10.7;
//	scan.configuration.baseFrequency = 0;
//	scan.configuration.PLL1phasefreq = 0.974;
//	scan.configuration.finalFrequency = 10.7;
//	scan.configuration.masterOscilatorFrequency = 64;
//	for(int x = 0; x < 400; ++x) {
//		hardwareDevice::scanStep step;
//		step.frequency = -0.075 + (x*(0.15/399));
//		scan.steps.insert(x, step);
//		////qDebug() << -0.075 + (x*(0.15/400));
//	}
//	hardwareDevice::currentScan = scan;
//	lmx.processNewScan();
//	ad9850 ad(hardwareDevice::DDS1, this);
//	ad.init();
//	lmx.init();
//	ad.processNewScan();

//	s.init(3);
//	if(s.getDevices().size() > 0)
//	//qDebug() << "--"<< s.getDevices().at(0).serial<< s.getDevices().at(0).deviceNumber;
//	s.openDevice(3);

	settings = new QSettings("JBTech", "OpenMSA", this);
	settings->clear();
	setServerPort(static_cast<quint16>(settings->value("app/serverPort", 1234).toInt()));
	//DEBUG
	server = new ComProtocol(this, settings->value("app/debugLevel", 0).toInt());
	server->setServerPort(getServerPort());
	if(!server->startServer())
		QMessageBox::critical(this, "Socket server failed to start","Please fix the issue and restart the application");
	connect(server, &ComProtocol::serverConnected, this, &MainWindow::newConnection);
	connect(server, &ComProtocol::packetReceived, this, &MainWindow::onMessageReceivedServer);
	if(settings->value("app/connectionType", interface::USB).toInt() == interface::USB)
		hwInterface = new simulator(this);
	connect(hwInterface, SIGNAL(dataReady(quint32,quint32,quint32)), this, SLOT(dataReady(quint32, quint32, quint32)));
	hwInterface->setWriteReadDelay_us(settings->value("msa/hardwareConfig/writeReadDelay_us", 1000).toUInt());
	devices.insert(msa::PLL1, settings->value("msa/hardwareTypes/PLL1", static_cast <int>(hardwareDevice::LMX2326)).toInt());
	devices.insert(msa::PLL2, settings->value("msa/hardwareTypes/PLL2", static_cast <int>(hardwareDevice::LMX2326)).toInt());
	devices.insert(msa::DDS1, settings->value("msa/hardwareTypes/DDS1", static_cast <int>(hardwareDevice::AD9850)).toInt());
	devices.insert(msa::PLL3, settings->value("msa/hardwareTypes/PLL3", static_cast <int>(hardwareDevice::LMX2326)).toInt());
	devices.insert(msa::DDS3, settings->value("msa/hardwareTypes/DDS1", static_cast <int>(hardwareDevice::AD9850)).toInt());
	devices.insert(msa::ADC_MAG, settings->value("msa/hardwareTypes/ADC_MAG", static_cast <int>(hardwareDevice::AD7685)).toInt());
	devices.insert(msa::ADC_PH, settings->value("msa/hardwareTypes/ADC_PH", static_cast <int>(hardwareDevice::AD7685)).toInt());
	//DEBUG
	hwInterface->init(settings->value("app/debugLevel", 08).toInt());
	msa::scanConfig config;
	config.LO2 = static_cast <double>(settings->value("msa/hardwareConfig/LO2", 1024).toFloat());
	config.appxdds1 = static_cast <double>(settings->value("msa/hardwareConfig/appxdds1", 10.7).toFloat());
	config.appxdds3 = static_cast <double>(settings->value("msa/hardwareConfig/appxdds3", 10.7).toFloat());
	config.baseFrequency = static_cast <double>(settings->value("msa/hardwareConfig/baseFrequency", 0).toFloat());
	config.PLL1phasefreq = static_cast <double>(settings->value("msa/hardwareConfig/PLL1phasefreq", 0.974).toFloat());
	config.PLL2phasefreq = static_cast <double>(settings->value("msa/hardwareConfig/PLL2phasefreq", 4).toFloat());
	config.PLL3phasefreq = static_cast <double>(settings->value("msa/hardwareConfig/PLL3phasefreq", 0.974).toFloat());
	config.finalFilterFrequency = static_cast <double>(settings->value("msa/hardwareConfig/finalFilterFrequency", 10.7).toFloat());
	config.masterOscilatorFrequency = static_cast <double>(settings->value("msa/hardwareConfig/masterOscilatorFrequency", 64).toFloat());
	config.dds1Filterbandwidth = static_cast <double>(settings->value("msa/hardwareConfig/dds1Filterbandwidth", 0.015).toFloat());
	config.dds3Filterbandwidth = static_cast <double>(settings->value("msa/hardwareConfig/dds3Filterbandwidth", 0.015).toFloat());
	config.PLL1phasepolarity_inverted = settings->value("msa/hardwareConfig/PLL1phasepolarity_inverted", true).toBool();
	config.PLL2phasepolarity_inverted = settings->value("msa/hardwareConfig/PLL2phasepolarity_inverted", false).toBool();
	config.PLL3phasepolarity_inverted = settings->value("msa/hardwareConfig/PLL3phasepolarity_inverted", true).toBool();
	config.finalFilterBandwidth = static_cast <double>(settings->value("msa/hardwareConfig/finalFilterBandwidth", 0.015).toFloat());;

	config.scanType = ComProtocol::SA_TG;
	config.adcAveraging = 2;
	config.gui.TGoffset = 0;
	config.gui.TGreversed = false;
	config.gui.SGout = 10;
	config.gui.SGout_multi = 1000000;

	config.gui.stop_multi = 1000000;
	config.gui.start_multi = 1000000;
	config.gui.step_freq_multi = 1000000;
	config.gui.center_freq_multi = 1000000;
	config.gui.span_freq_multi = 1000000;
	config.gui.band = -1;
	config.gui.stop = 0.075;
	config.gui.start = -0.075;
	config.gui.scanType = ComProtocol::scanType_t(config.scanType);
	config.gui.steps_number = 400;
	config.gui.step_freq = (config.gui.stop - config.gui.start) / config.gui.steps_number;
	config.gui.center_freq = config.gui.start + ((config.gui.stop - config.gui.start) / 2);
	config.gui.stepModeAuto = false;
	config.gui.isStepInSteps = true;
	config.gui.span_freq = (config.gui.stop - config.gui.start);
	config.gui.TGoffset_multi = 1000000;
	msa::getInstance().setScanConfiguration(config);
	connect(hwInterface,SIGNAL(connected()), this,SLOT(on_Connect()));
	connect(hwInterface,SIGNAL(disconnected()), this,SLOT(on_Disconnect()));
	if(hwInterface->getIsConnected())
		on_Connect();

//	msa::getInstance().hardwareInit(devices, hwInterface);
//	msa::getInstance().initScan(false, -0.075, 0.075, 0.15/400);
//	hwInterface->autoScan();
}

MainWindow::~MainWindow()
{
#ifdef QT_NO_SYSTEMTRAYICON
	delete ui;
#endif
	delete msa::getInstance().currentScan.steps;
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
        dac.mag = mag;
        dac.phase = phase;
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
	//msa::getInstance().initScan(false, 95, 105, (quint32)2000, -1);
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
	ComProtocol::msg_scan_config cfg_msg;
	msa::scanConfig config = msa::getInstance().getScanConfiguration();
	cfg_msg = config.gui;
	cfg_msg.scanType = ComProtocol::scanType_t(config.scanType);
	QMutexLocker locker(&messageSend);
	server->sendMessage(ComProtocol::SCAN_CONFIG, ComProtocol::MESSAGE_SEND, &cfg_msg);
}


void MainWindow::onMessageReceivedServer(ComProtocol::messageType type, QByteArray data)
{
	static quint32 lastMessage = 0;
	ComProtocol::msg_scan_config m_config;
	ComProtocol::messageCommandType command;
	quint32 msgNumber = 0;
	qDebug() << "onMessageReceived";
	switch (type) {
		case ComProtocol::SCAN_CONFIG:
			server->unpackMessage(data, type, command, msgNumber, &m_config);
		break;
	}
	msa::scanConfig config = msa::getInstance().getScanConfiguration();
	config.scanType = m_config.scanType;
	config.gui = m_config;
	msa::getInstance().setScanConfiguration(config);
	if(m_config.isStepInSteps)// TODO HANDLE m_config.stepModeAuto
		msa::getInstance().initScan(false,  m_config.start, m_config.stop, quint32(m_config.step_freq), m_config.band);
	else {
		msa::getInstance().initScan(false,  m_config.start, m_config.stop, m_config.step_freq, m_config.band);
	}
	if(lastMessage != 0) {
		Q_ASSERT(msgNumber - lastMessage == 1);
	}
	lastMessage = msgNumber;
}

quint16 MainWindow::getServerPort() const
{
	return serverPort;
}

void MainWindow::setServerPort(const quint16 &value)
{
	serverPort = value;
}

