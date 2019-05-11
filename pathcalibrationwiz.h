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

namespace Ui {
class pathCalibration;
}

class pathCalibrationWiz : public QWidget
{
	Q_OBJECT

public:
	explicit pathCalibrationWiz(QString name, double centerFreq, double bandWidth, calParser::magPhaseCalData data, QWidget *parent = nullptr);
	~pathCalibrationWiz();

private slots:
	void on_pb_start_clicked();

private:
	Ui::pathCalibration *ui;
};

#endif // PATHCALIBRATION_H