/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      calparser.h file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   calParser
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
#ifndef CALPARSER_H
#define CALPARSER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QDateTime>
#include <QRandomGenerator>
#include <QStandardPaths>

#define STANDARD_FREQ_CAL_FILENAME "FrequencyCalibration.json"
#define STANDARD_PATHS_CAL_FILENAME "PathsCalibration.json"

class calParser : public QObject
{
	Q_OBJECT
public:
	explicit calParser(QObject *parent = nullptr);

	typedef struct {
		QString calDate;
		double calPower;
		QHash<double, double> freqToPower;
	}freqCalData;
	typedef struct {
		double dbm_val;
		double phase_val;
	} magCalFactors;
	typedef struct {
		int controlPin;
		double centerFreq_MHZ;
		double bandwidth_MHZ;
		QString calDate;
		QString pathName;
		double calFrequency;
		QHash<uint, magCalFactors> adcToMagCalFactors;
	}magPhaseCalData;
	bool saveCalDataToFile(freqCalData data, QString file);
	bool saveCalDataToFile(QList<magPhaseCalData> data, QString file);
	freqCalData loadFreqCalDataFromFile(QString file, bool &success, QString &errorText);
	QList<magPhaseCalData> loadMagPhaseCalDataFromFile(QString file, bool &success, QString &errorText);
	freqCalData importFreqCalFromOriginalSW(QString file, bool &success);
	magPhaseCalData importMagPhaseCalFromOriginalSW(QString file, bool &success);
	QString getConfigLocation();
signals:

public slots:
};

#endif // CALPARSER_H
