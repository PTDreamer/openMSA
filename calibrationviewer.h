/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      calibrationviewer.h file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   calibrationViewer
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
#ifndef CALIBRATIONVIEWER_H
#define CALIBRATIONVIEWER_H

#include <QWidget>
#include <QtCharts>
#include "hardware/msa.h"

using namespace QtCharts;

namespace Ui {
class calibrationViewer;
}

class calibrationViewer : public QWidget
{
	Q_OBJECT

public:
	explicit calibrationViewer(QWidget *parent = nullptr);
	~calibrationViewer();

private slots:
	void on_comboBox_currentIndexChanged(int index);

	void on_checkBox_toggled(bool checked);

private:
	Ui::calibrationViewer *ui;
	QVector<QVector<QLineSeries *>> seriesStore;
	QChart *chart;
	QVector<QString> titles;
	QVector<QString> ytitles;
	QVector<QString> xtitles;
//	QLineSeries *magCalSeries;
//	QLineSeries *phaseCalSeries;

//	QLineSeries *magCalSeriesLimited;
//	QLineSeries *phaseCalSeriesLimited;

//	QLineSeries *freqCalSeries;
//	QLineSeries *freqCalSeriesLimited;
};

#endif // CALIBRATIONVIEWER_H
