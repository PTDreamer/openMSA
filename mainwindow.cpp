#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hardware/lmx2326.h"
#include <QDebug>
#include "hardware/hardwaredevice.h"
#include "hardware/ad9850.h"
#include "hardware/controllers/slimusb.h"
#include "hardware/msa.h"


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

	socket = NULL;
	s = new slimusb(this);
	connect(s, SIGNAL(dataReady(quint32,quint32,quint32)), this, SLOT(dataReady(quint32, quint32, quint32)));
	s->setWriteReadDelay_us(100);
	devices.insert(msa::PLL1, hardwareDevice::LMX2326);
	devices.insert(msa::PLL2, hardwareDevice::LMX2326);
	devices.insert(msa::DDS1, hardwareDevice::AD9850);
	devices.insert(msa::PLL3, hardwareDevice::LMX2326);
	devices.insert(msa::DDS3, hardwareDevice::AD9850);
	devices.insert(msa::ADC_MAG, hardwareDevice::AD7685);
	devices.insert(msa::ADC_PH, hardwareDevice::AD7685);
	s->init(3);
	msa::scanConfig config;
	config.LO2 = 1024;
	config.appxdds1 = 10.7;
	config.appxdds3 = 10.7;
	config.baseFrequency = 0;
	config.PLL1phasefreq = 0.974;
	config.PLL2phasefreq = 4;
	config.PLL3phasefreq = 0.974;
	config.finalFilterFrequency = 10.7;
	config.masterOscilatorFrequency = 64;
	config.scanType = msa::SA_SG;
	config.adcAveraging = 2;
	config.dds1Filterbandwidth = 0.015;
	config.dds3Filterbandwidth = 0.015;
	config.TGoffset = 0;
	config.TGreversed = false;
	config.finalFilterBandwidth = 0.015;
	config.SGout = 0;
	config.PLL1phasepolarity_inverted = true;
	config.PLL2phasepolarity_inverted = false;
	config.PLL3phasepolarity_inverted = true;
	msa::getInstance().setScanConfiguration(config);

//	//CommandFilterSlimCBUSB
//	qDebug() << "send A102000080" << s->sendArrayForDebug(s->convertStringToByteArray("A102000080"));
//	qDebug() << "send A1010000" << s->sendArrayForDebug(s->convertStringToByteArray("A1010000"));
//	//SelectLatchedSwitches desiredFreqBand
//	qDebug() << "send A2010084" << s->sendArrayForDebug(s->convertStringToByteArray("A2010084"));
//	qDebug() << "send A2010004" << s->sendArrayForDebug(s->convertStringToByteArray("A2010004"));
//	qDebug() << "send A2010084" << s->sendArrayForDebug(s->convertStringToByteArray("A2010084"));
//	//init SelectFilter filtbank
//	qDebug() << "send A102000080" << s->sendArrayForDebug(s->convertStringToByteArray("A102000080"));
//	qDebug() << "send A1010000" << s->sendArrayForDebug(s->convertStringToByteArray("A1010000"));
//	//CommandPDMonly
//	qDebug() << "send A30300002000" << s->sendArrayForDebug(s->convertStringToByteArray("A30300002000"));
//	//SetADCmux
//	qDebug() << "send A0010000" << s->sendArrayForDebug(s->convertStringToByteArray("A0010000"));
//	//SelectLatchedSwitches desiredFreqBand
//	qDebug() << "send A2010084" << s->sendArrayForDebug(s->convertStringToByteArray("A2010084"));
//	qDebug() << "send A2010004" << s->sendArrayForDebug(s->convertStringToByteArray("A2010004"));
//	qDebug() << "send A2010084" << s->sendArrayForDebug(s->convertStringToByteArray("A2010084"));
	isConnected = false;
	connect(s,SIGNAL(connected()), this,SLOT(on_Connect()));
	if(s->getIsConnected())
		on_Connect();
	connect(s,SIGNAL(disconnected()), this,SLOT(on_Disconnect()));
	server = new QTcpServer(this);

	connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));

	if(!server->listen(QHostAddress::Any, 1234))
	{
		qDebug() << "Server could not start!";
	}
	else
	{
		qDebug() << "Server started!";
	}

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
	if(socket)
		socket->write(str.toLatin1());
}

void MainWindow::on_Connect()
{
	QMutexLocker locker(&mutex);
	if(isConnected)
		return;
	msa::getInstance().hardwareInit(devices, s);
	msa::getInstance().initScan(false, -0.0375, 0.0375, 0.075/400);
	s->autoScan();
	isConnected = true;
}

void MainWindow::on_Disconnect()
{
	isConnected = false;
}

void MainWindow::newConnection()
{
	socket = server->nextPendingConnection();
}

