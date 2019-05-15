/**
 ******************************************************************************
 *
 * @file       pathcalibration.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      pathcalibration.cpp file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   pathCalibration
 * @{
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>
 */
#include "pathcalibrationwiz.h"
#include "ui_pathcalibration.h"
#include <QMessageBox>
pathCalibrationWiz::pathCalibrationWiz(QString name, double centerFreq, double bandWidth, calParser::magPhaseCalData data, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::pathCalibration)
{
	ui->setupUi(this);
	ui->le_name->setText(name);
	ui->ds_center_frequency->setValue(centerFreq);
	ui->ds_bandwidth->setValue(bandWidth);
	QList<uint> keys = data.adcToMagCalFactors.keys();
	std::sort(keys.begin(), keys.end());
	int count = 0;
	foreach(uint key, keys) {
		ui->tw_pathCalibration->insertRow(count);
		ui->tw_pathCalibration->setItem(count, 0,  new QTableWidgetItem(QString::number(key)));
		ui->tw_pathCalibration->setItem(count, 1,  new QTableWidgetItem(QString::number(data.adcToMagCalFactors.value(key).dbm_val)));
		ui->tw_pathCalibration->setItem(count, 2,  new QTableWidgetItem(QString::number(data.adcToMagCalFactors.value(key).phase_val)));
		++count;
	}
	ui->gb_calibration->setEnabled(false);
	mutex = new QMutex;
	valmutex = new QMutex;
	connect(this, &pathCalibrationWiz::averagesReady, this, &pathCalibrationWiz::onAveragesReady);
}

pathCalibrationWiz::~pathCalibrationWiz()
{
	delete ui;
}

void pathCalibrationWiz::on_pb_start_clicked()
{
	ui->gb_settings->setEnabled(false);
	ui->gb_calibration->setEnabled(true);
	scanConfigBackup = msa::getInstance().getScanConfiguration();
	msa::scanConfig newConfig = scanConfigBackup;
	bool found = false;
	QString narrower;
	double biggest = 0;
	foreach(QString s, scanConfigBackup.videoFilters.keys()) {
		if(scanConfigBackup.videoFilters.value(s).address != -1) {
			if(scanConfigBackup.videoFilters.value(s).value > biggest) {
				biggest = scanConfigBackup.videoFilters.value(s).value;
				narrower = s;
				found = true;
			}
		}
	}
	newConfig.currentVideoFilterName = narrower;
	if(!found)
		QMessageBox::information(this, tr("Video Filter"), tr("Please manually select the narrower video filter"));
	calParser::magPhaseCalData cal;
	cal.bandwidth_MHZ = ui->ds_bandwidth->value();
	cal.centerFreq_MHZ = ui->ds_center_frequency->value();
	newConfig.pathCalibration = cal;
	scanStatusBackup = msa::getInstance().currentInterface->getCurrentStatus();
	newConfig.gui.scanType = ComProtocol::scanType_t(newConfig.scanType);
	//TODO set scantype
	msa::getInstance().setScanConfiguration(newConfig);
	msa::getInstance().initScan(false, ui->ds_cal_frequency->value(), ui->ds_cal_frequency->value(), quint32(1000), -1);
	msa::getInstance().currentInterface->setStatus(interface::status_scanning);
	connect(msa::getInstance().currentInterface, SIGNAL(dataReady(quint32,quint32,quint32)), this, SLOT(adcDataReady(quint32, quint32, quint32)), Qt::UniqueConnection);
	msa::getInstance().currentInterface->setWriteReadDelay_us(ulong(ulong(ui->sb_delay->value()) * 1000));
	//msa::getInstance().currentInterface
	//TODO choose proper video filter
	// command to frequency and set span to 0
	// handle zero span on initScan and GUI
}

void pathCalibrationWiz::adcDataReady(quint32 step, quint32 mag, quint32 phase)
{
	valmutex->lock();
	lastMag = mag;
	lastPhase = phase;
	valmutex->unlock();
	mutex->unlock();
}

void pathCalibrationWiz::on_pb_read_adc_clicked()
{
	QList<double> magVals;
	QList<double> phaseVals;
	for(int x = 0; x < ui->sb_average->value(); ++x) {
		while(!mutex->tryLock()) {
			QApplication::processEvents();
		}
		ui->progressBar->setValue(int(float(x + 1) / ui->sb_average->value()  * 100));
		valmutex->lock();
		magVals.append(lastMag);
		phaseVals.append(lastPhase);
		valmutex->unlock();
	}
	std::sort(magVals.begin(), magVals.end());
	std::sort(phaseVals.begin(), phaseVals.end());
	magVals.removeFirst();
	magVals.removeLast();
	phaseVals.removeFirst();
	phaseVals.removeLast();
	double acc = 0;
	foreach(double val, magVals) {
		acc += val;
	}
	double avgmag = acc / magVals.length();
	acc = 0;
	foreach(double val, phaseVals) {
		acc += val;
	}
	double avgphase = acc / phaseVals.length();
	emit averagesReady(avgmag, avgphase);
}

void pathCalibrationWiz::onAveragesReady(double mag, double phase)
{
	static double phaseRef;
	double phased = 360 * phase / 65535;
	if(ui->cb_read_phase) {
		if(ui->tw_pathCalibration->rowCount() == 0)
			phaseRef = phased;
		else {
			phased = phased - phaseRef;
		}
		while(phased <= -180)
			phased = phased + 360;
		while(phased > 180)
			phased = phased - 360;
	}
	ui->tw_pathCalibration->insertRow(ui->tw_pathCalibration->rowCount());
	ui->tw_pathCalibration->setItem(ui->tw_pathCalibration->rowCount()-1, 0,  new QTableWidgetItem(QString::number(mag)));
	if(ui->cb_read_phase)
		ui->tw_pathCalibration->setItem(ui->tw_pathCalibration->rowCount()-1, 2,  new QTableWidgetItem(QString::number(phased)));
	if(ui->cb_autoset_dbm && ui->tw_pathCalibration->rowCount() > 1) {
		bool ok;
		double last = ui->tw_pathCalibration->item(ui->tw_pathCalibration->rowCount() - 2, 1)->text().toDouble(&ok);
		if(ok) {
			last += ui->ds_dbm_auto_increment->value();
			ui->tw_pathCalibration->setItem(ui->tw_pathCalibration->rowCount() -1, 1, new QTableWidgetItem(QString::number(last)));
		}
	}
}
