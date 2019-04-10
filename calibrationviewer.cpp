/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      calibrationviewer.cpp file
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
#include "calibrationviewer.h"
#include "ui_calibrationviewer.h"

calibrationViewer::calibrationViewer(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::calibrationViewer)
{
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	ui->setupUi(this);

	calParser::magPhaseCalData pathCalibration = msa::getInstance().currentScan.configuration.pathCalibration;
	calParser::freqCalData frequencyCalibration = msa::getInstance().currentScan.configuration.frequencyCalibration;

	QLineSeries *magCalSeries = new QLineSeries();
	QLineSeries *phaseCalSeries = new QLineSeries();

	QLineSeries *magCalSeriesLimited = new QLineSeries();
	QLineSeries *phaseCalSeriesLimited = new QLineSeries();

	QLineSeries *freqCalSeries = new QLineSeries();
	QLineSeries *freqCalSeriesLimited = new QLineSeries();

	QList<unsigned int> pkeys = pathCalibration.adcToMagCalFactors.keys();
	QList<double> fkeys = frequencyCalibration.freqToPower.keys();

	std::sort(pkeys.begin(), pkeys.end());
	std::sort(fkeys.begin(), fkeys.end());

	double firstDB = pathCalibration.adcToMagCalFactors.value(pkeys.first()).dbm_val;
	double LastDB = pathCalibration.adcToMagCalFactors.value(pkeys.last()).dbm_val;
	double firstPh = pathCalibration.adcToMagCalFactors.value(pkeys.first()).phase_val;
	double LastPh = pathCalibration.adcToMagCalFactors.value(pkeys.last()).phase_val;
	double firstFreq = frequencyCalibration.freqToPower.value(fkeys.first());
	double LastFreq = frequencyCalibration.freqToPower.value(fkeys.last());

	int firstDBindex = 0;
	int lastDBindex = 0;
	int firstPHindex = 0;
	int lastPHindex = 0;
	int firstFreqindex = 0;
	int lastFreqindex = 0;

	for (int x = 0; x < pkeys.length(); ++x) {
		if(firstDB < pathCalibration.adcToMagCalFactors.value(pkeys.value(x)).dbm_val) {
			firstDBindex = x;
			break;
		}
	}

	for (int x = 0; x < pkeys.length(); ++x) {
		if(std::abs(firstPh - pathCalibration.adcToMagCalFactors.value(pkeys.value(x)).phase_val) > 0.01) {
			firstPHindex = x;
			break;
		}
	}

	for (int x = 0; x < fkeys.length(); ++x) {
		if(std::abs(firstFreq - frequencyCalibration.freqToPower.value(pkeys.value(x))) > 0.01) {
			firstFreqindex = x;
			break;
		}
	}

	for (int x = pkeys.length() - 1; x > 0; --x) {
		if(LastDB > pathCalibration.adcToMagCalFactors.value(pkeys.value(x)).dbm_val) {
			lastDBindex = x;
			break;
		}
	}

	for (int x = pkeys.length() - 1; x > 0; --x) {
		if(std::abs(LastPh - pathCalibration.adcToMagCalFactors.value(pkeys.value(x)).phase_val) > 0.01) {
			lastPHindex = x;
			break;
		}
	}

	for (int x = fkeys.length() - 1; x > 0; --x) {
		if(std::abs(LastFreq - frequencyCalibration.freqToPower.value(pkeys.value(x))) > 0.01) {
			lastFreqindex = x;
			break;
		}
	}

	for (int x = firstDBindex;x < lastDBindex; ++x) {
		magCalSeriesLimited->append(pkeys.value(x), pathCalibration.adcToMagCalFactors.value(pkeys.value(x)).dbm_val);
	}

	for (int x = firstPHindex;x < lastPHindex; ++x) {
		phaseCalSeriesLimited->append(pkeys.value(x), pathCalibration.adcToMagCalFactors.value(pkeys.value(x)).phase_val);
	}

	for (int x = firstFreqindex;x < lastFreqindex; ++x) {
		freqCalSeriesLimited->append(fkeys.value(x), frequencyCalibration.freqToPower.value(fkeys.value(x)));
	}

	foreach (unsigned int val, pkeys) {
		magCalSeries->append(val, pathCalibration.adcToMagCalFactors.value(val).dbm_val);
		phaseCalSeries->append(val, pathCalibration.adcToMagCalFactors.value(val).phase_val);
	}

	foreach (double val, fkeys) {
		freqCalSeries->append(val, frequencyCalibration.freqToPower.value(val));
	}
	QVector<QLineSeries*> s;
	s.insert(0, freqCalSeries);
	s.insert(1, freqCalSeriesLimited);
	seriesStore.insert(0, s);
	s.clear();
	s.insert(0, magCalSeries);
	s.insert(1, magCalSeriesLimited);
	seriesStore.insert(1, s);
	s.clear();
	s.insert(0, phaseCalSeries);
	s.insert(1, phaseCalSeriesLimited);
	seriesStore.insert(2, s);

	titles.insert(0, "Frequency Calibration");
	titles.insert(1, "Magnitude Calibration");
	titles.insert(2, "Phase Calibration");
	xtitles.insert(0, "Frequency in MHz");
	xtitles.insert(1, "ADC value");
	xtitles.insert(2, "ADC value");
	ytitles.insert(0, "Magnitude in db");
	ytitles.insert(1, "Magnitude in dbm");
	ytitles.insert(2, "Phase in degrees");

	chart = new QChart();
	chart->legend()->hide();
	chart->addSeries(freqCalSeries);
	chart->createDefaultAxes();
	chart->setTitle(titles.at(0));
	chart->axes(Qt::Horizontal).at(0)->setTitleText(xtitles.at(0));
	chart->axes(Qt::Vertical).at(0)->setTitleText(ytitles.at(0));

	QChartView *chartView = new QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);
	ui->verticalLayout->addWidget(chartView);
	ui->verticalLayout->removeWidget(ui->chartView);
	delete ui->chartView;
}

calibrationViewer::~calibrationViewer()
{
	delete ui;
}

void calibrationViewer::on_comboBox_currentIndexChanged(int index)
{
	chart->removeSeries(chart->series().at(0));
	chart->addSeries(seriesStore.value(index).value(ui->checkBox->isChecked()));
	chart->createDefaultAxes();
	chart->axes(Qt::Horizontal).at(0)->setTitleText(xtitles.at(index));
	chart->axes(Qt::Vertical).at(0)->setTitleText(ytitles.at(index));
	chart->setTitle(titles.at(index));
}

void calibrationViewer::on_checkBox_toggled(bool checked)
{
	chart->removeSeries(chart->series().at(0));
	chart->addSeries(seriesStore.value(ui->comboBox->currentIndex()).value(checked));
	chart->createDefaultAxes();
	chart->axes(Qt::Horizontal).at(0)->setTitleText(xtitles.at(ui->comboBox->currentIndex()));
	chart->axes(Qt::Vertical).at(0)->setTitleText(ytitles.at(ui->comboBox->currentIndex()));
	chart->setTitle(titles.at(ui->comboBox->currentIndex()));
}
