#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hardware/lmx2326.h"
#include <QDebug>
#include "hardware/ad9850.h"
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow), lmx(deviceParser::PLL1, this)
{
	ui->setupUi(this);
	hardwareDevice::scanStruct scan;
	scan.configuration.LO2 = 1024;
	scan.configuration.appxdds1 = 10.7;
	scan.configuration.baseFrequency = 0;
	scan.configuration.PLL1phasefreq = 0.974;
	scan.configuration.finalFrequency = 10.7;
	scan.configuration.masterOscilatorFrequency = 64;
	for(int x = 0; x < 400; ++x) {
		hardwareDevice::scanStep step;
		step.frequency = -0.075 + (x*(0.15/399));
		scan.steps.insert(x, step);
		//qDebug() << -0.075 + (x*(0.15/400));
	}
	lmx.processNewScan(scan);
	ad9850 ad(deviceParser::DDS1, this);
	ad.init();
	lmx.init();
	ad.processNewScan(scan);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_pushButton_clicked()
{
	lmx.init();
}

