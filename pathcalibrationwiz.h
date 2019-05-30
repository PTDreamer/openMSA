/**
 ******************************************************************************
 *
 * @file       pathcalibration.h
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      pathcalibration.h file
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
#ifndef PATHCALIBRATION_H
#define PATHCALIBRATION_H

#include <QWidget>
#include "calparser.h"
#include "hardware/msa.h"
#include "hardware/controllers/interface.h"
#include <QMutex>
#include <QMutexLocker>
#include <QDialog>

namespace Ui {
class pathCalibration;
}

class pathCalibrationWiz : public QDialog
{
	Q_OBJECT

public:
	explicit pathCalibrationWiz(QString name, double centerFreq, double bandWidth, int controlPin, double calibFreq, calParser::magPhaseCalData data, QWidget *parent = nullptr);
	~pathCalibrationWiz();

	calParser::magPhaseCalData getReturnData() const;

private slots:
	void on_pb_start_clicked();
	void adcDataReady(quint32, quint32, quint32);

	void on_pb_read_adc_clicked();
	void onAveragesReady(double mag, double phase);
	void on_pb_delete_measurement_clicked();

	void on_pb_cancel_clicked();

	void on_pb_save_clicked();

	void on_saveConfigData_clicked();

signals:
	void averagesReady(double mag, double phase);
   void WidgetClosed(calParser::magPhaseCalData data);
protected:
private:
	Ui::pathCalibration *ui;
	msa::scanConfig scanConfigBackup;
	interface::status scanStatusBackup;
	unsigned int readDelayBackup;
	double lastMag;
	double lastPhase;
	QMutex *mutex;
	QMutex *valmutex;
	calParser::magPhaseCalData returnData;
};

#endif // PATHCALIBRATION_H
