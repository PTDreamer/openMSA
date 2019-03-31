#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hardware/lmx2326.h"
#include <QDebug>
#include "hardware/hardwaredevice.h"
#include "hardware/ad9850.h"
#include "hardware/controllers/slimusb.h"
#include "hardware/msa.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
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
	setServerPort(static_cast<quint16>(settings->value("app/serverPort", 1234).toInt()));
	server = new ComProtocol;
	server->setServerPort(getServerPort());
	if(!server->startServer())
		QMessageBox::critical(this, "Socket server failed to start","Please fix the issue and restart the application");
	connect(server, &ComProtocol::serverConnected, this, &MainWindow::newConnection);
	connect(server, &ComProtocol::packetReceived, this, &MainWindow::onMessageReceivedServer);

	if(settings->value("app/connectionType", interface::USB).toInt() == interface::USB)
		hwInterface = new slimusb(this);
	connect(hwInterface, SIGNAL(dataReady(quint32,quint32,quint32)), this, SLOT(dataReady(quint32, quint32, quint32)));
	hwInterface->setWriteReadDelay_us(settings->value("msa/hardwareConfig/writeReadDelay_us", 1000).toUInt());
	devices.insert(msa::PLL1, settings->value("msa/hardwareTypes/PLL1", static_cast <int>(hardwareDevice::LMX2326)).toInt());
	devices.insert(msa::PLL2, settings->value("msa/hardwareTypes/PLL2", static_cast <int>(hardwareDevice::LMX2326)).toInt());
	devices.insert(msa::DDS1, settings->value("msa/hardwareTypes/DDS1", static_cast <int>(hardwareDevice::AD9850)).toInt());
	devices.insert(msa::PLL3, settings->value("msa/hardwareTypes/PLL3", static_cast <int>(hardwareDevice::LMX2326)).toInt());
	devices.insert(msa::DDS3, settings->value("msa/hardwareTypes/DDS1", static_cast <int>(hardwareDevice::AD9850)).toInt());
	devices.insert(msa::ADC_MAG, settings->value("msa/hardwareTypes/ADC_MAG", static_cast <int>(hardwareDevice::AD7685)).toInt());
	devices.insert(msa::ADC_PH, settings->value("msa/hardwareTypes/ADC_PH", static_cast <int>(hardwareDevice::AD7685)).toInt());
	hwInterface->init(settings->value("app/debugLevel", 3).toInt());
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

	config.scanType = msa::SA_SG;
	config.adcAveraging = 2;
	config.TGoffset = 0;
	config.TGreversed = false;
	config.SGout = 0;
	msa::getInstance().setScanConfiguration(config);

	isConnected = false;
	connect(hwInterface,SIGNAL(connected()), this,SLOT(on_Connect()));
	if(hwInterface->getIsConnected())
		on_Connect();
	connect(hwInterface,SIGNAL(disconnected()), this,SLOT(on_Disconnect()));

//	msa::getInstance().hardwareInit(devices, hwInterface);
//	msa::getInstance().initScan(false, -0.0375, 0.0375, 0.075/400);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_pushButton_clicked()
{
}

void MainWindow::dataReady(quint32 step, quint32 mag, quint32 phase)
{
	qDebug() << "received step:" << step << "MAG=" << mag << "PHASE=" << phase;
	QString str;
	str=QString("S%1V%2E").arg(step).arg(mag);
    if(server->isConnected()) {
        ComProtocol::msg_dual_dac dac;
        dac.mag = mag;
        dac.phase = phase;
        dac.step = step;
        server->sendMessage(ComProtocol::DUAL_DAC, ComProtocol::MESSAGE_SEND, &dac);
    }
}

void MainWindow::on_Connect()
{
	QMutexLocker locker(&mutex);
	if(isConnected)
		return;
	msa::getInstance().hardwareInit(devices, hwInterface);
    msa::getInstance().initScan(false, -0.075, 0.075, 0.15/400);
	hwInterface->autoScan();
	isConnected = true;
}

void MainWindow::on_Disconnect()
{
	isConnected = false;
}

void MainWindow::newConnection()
{	
	ComProtocol::msg_dual_dac dac;
	for(int i = 0; i < 100; ++i) {
	dac.mag = 123+i;
	dac.phase = 456+i;
	dac.step = 69;

	server->sendMessage(ComProtocol::DUAL_DAC, ComProtocol::MESSAGE_SEND, &dac);
	}
//	QTimer *t = new QTimer(this);
//	connect(t, &QTimer::timeout, this, &MainWindow::sendTest);
//	t->start(100);
}


void MainWindow::onMessageReceivedServer(ComProtocol::messageType type, QByteArray data)
{
	static int lastMessage = -1;
	ComProtocol::msg_dual_dac m;
	ComProtocol::messageCommandType command;
	quint32 msgNumber;
	qDebug() << "onMessageReceived";
	switch (type) {
		case ComProtocol::DUAL_DAC:
			server->unpackMessage(data, type, command, msgNumber, &m);
		break;
	}
	qDebug() << "Server received:" << m.mag << m.phase << m.step << msgNumber;
	Q_ASSERT(msgNumber - lastMessage == 1);
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

