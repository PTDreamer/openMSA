#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hardware/lmx2326.h"
#include <QDebug>
#include "hardware/hardwaredevice.h"
#include "hardware/ad9850.h"
#include "hardware/controllers/slimusb.h"

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
//		//qDebug() << -0.075 + (x*(0.15/400));
//	}
//	hardwareDevice::currentScan = scan;
//	lmx.processNewScan();
//	ad9850 ad(hardwareDevice::DDS1, this);
//	ad.init();
//	lmx.init();
//	ad.processNewScan();

//	s.init(3);
//	if(s.getDevices().size() > 0)
//	qDebug() << "--"<< s.getDevices().at(0).serial<< s.getDevices().at(0).deviceNumber;
//	s.openDevice(3);
	slimusb *s = new slimusb(this);
	QHash<hardwareDevice::MSAdevice, hardwareDevice::HWdevice> devices;
	devices.insert(hardwareDevice::PLL1, hardwareDevice::LMX2326);
	devices.insert(hardwareDevice::DDS1, hardwareDevice::AD9850);
	devices.insert(hardwareDevice::ADC_MAG, hardwareDevice::AD7685);
	devices.insert(hardwareDevice::ADC_PH, hardwareDevice::AD7685);
	s->init(3);
	hardwareDevice::scanConfig config;
	config.LO2 = 1024;
	config.appxdds1 = 10.7;
	config.baseFrequency = 0;
	config.PLL1phasefreq = 0.974;
	config.finalFrequency = 10.7;
	config.masterOscilatorFrequency = 64;
	config.scanType = hardwareDevice::SA;
	config.adcAveraging = 2;
	s->setScanConfiguration(config);
	s->hardwareInit(devices);
	s->initScan(false, -0.075, 0.075, 0.15/400);
	//s->autoScan();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_pushButton_clicked()
{
}

