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
}

pathCalibrationWiz::~pathCalibrationWiz()
{
	delete ui;
}

void pathCalibrationWiz::on_pb_start_clicked()
{
	ui->gb_settings->setEnabled(false);
	ui->gb_calibration->setEnabled(true);
}
