/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      calparser.cpp file
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
#include "calparser.h"
#include <QFile>
#include <QDebug>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QDir>

calParser::calParser(QObject *parent) : QObject(parent)
{
}

bool calParser::saveCalDataToFile(freqCalData data, QString file)
{
	if(file.isEmpty())
		file = getConfigLocation() + QDir::separator() + STANDARD_FREQ_CAL_FILENAME;
	QFileInfo inf(file);
	QDir dir(inf.absolutePath());
	if (!dir.exists())
		dir.mkpath(inf.absolutePath());

	QFile f(file);
	if(!f.open(QIODevice::ReadWrite))
		return false;
	QJsonObject obj;
	obj["CalDate"] = data.calDate;
	obj["CalPower"] = data.calPower;
	obj["Type"] = "Frequency Calibration";
	QJsonArray talks;
	QList<double> keys = data.freqToPower.keys();
	std::sort(keys.begin(), keys.end());
	foreach(double key, keys)
	{
		QJsonObject obj;
		obj["Frequency(MHz)"] = key;
		obj["Power(db)"] = data.freqToPower.value(key);
		talks.append(obj);
	}
	obj["FreqTable"] = talks;
	f.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
	f.close();
	return true;
}

bool calParser::saveCalDataToFile(QList<magPhaseCalData> data, QString file)
{
	if(file.isEmpty())
		file = getConfigLocation() + QDir::separator() + STANDARD_PATHS_CAL_FILENAME;
	QFileInfo inf(file);
	QDir dir(inf.absolutePath());
	if (!dir.exists())
		dir.mkpath(inf.absolutePath());

	QFile f(file);
	if(!f.open(QIODevice::ReadWrite))
		return false;
	QJsonObject root;
	root["Type"] = "Path Calibration";
	QJsonArray paths;
	foreach (magPhaseCalData d, data) {
		QJsonObject obj;
		obj["CalDate"] = d.calDate;
		obj["CalFrequency"] = d.calFrequency;
		obj["PathName"] = d.pathName;
		obj["FinalFilterFrequency"] = d.centerFreq_MHZ;
		obj["FinalFilterBandwidth"] = d.bandwidth_MHZ;
		obj["ControlPin"] = d.controlPin;
		QJsonArray talks;
		QList<uint> keys = d.adcToMagCalFactors.keys();
		std::sort(keys.begin(), keys.end());
		foreach(uint key, keys)
		{
			QJsonObject o;
			o["ADC"] = int(key);
			o["dBm"] = d.adcToMagCalFactors.value(key).dbm_val;
			o["Phase"] = d.adcToMagCalFactors.value(key).phase_val;
			talks.append(o);
		}
		obj["MagTable"] = talks;
		paths.append(obj);
	}
	root["Paths"] = paths;
	f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
	f.close();
	return true;
}

bool calParser::createDefaultFreqCalData(QString file)
{
	if(file.isEmpty())
		getConfigLocation() + QDir::separator() + STANDARD_FREQ_CAL_FILENAME;
	freqCalData d;
	d.calDate = "NEVER";
	d.calPower = 0.0;
	d.freqToPower.insert(0, 0);
	d.freqToPower.insert(1000, 0);
	return saveCalDataToFile(d, file);
}

bool calParser::createDefaultMagPhaseCalData(QString file)
{
	if(file.isEmpty())
		getConfigLocation() + QDir::separator() + STANDARD_PATHS_CAL_FILENAME;
	magPhaseCalData d;
	d.calDate = "NEVER";
	d.pathName = "DEFAULT";
	d.controlPin = -1;
	d.calFrequency = 0;
	d.bandwidth_MHZ = 0.015;
	d.centerFreq_MHZ = 10.7;
	magCalFactors f;
	f.dbm_val = -120.0;
	f.phase_val = 0.0;
	d.adcToMagCalFactors.insert(0, f);
	f.dbm_val = 0.0;
	f.phase_val = 0.0;
	d.adcToMagCalFactors.insert(32767, f);
	QList<magPhaseCalData> list;
	list.append(d);
	return saveCalDataToFile(list, file);
}



calParser::freqCalData calParser::loadFreqCalDataFromFile(QString file, bool &success, QString &errorText)
{
	if(file.isEmpty())
		file = getConfigLocation() + QDir::separator() + STANDARD_FREQ_CAL_FILENAME;
	success = true;
	calParser::freqCalData ret;
	QFile f(file);
	if(!f.open(QIODevice::ReadWrite)) {
		errorText = "Could not open frequency calibration file";
		success = false;
		return ret;
	}
	QJsonParseError jerror;
	QJsonDocument jdoc= QJsonDocument::fromJson(f.readAll(),&jerror);
	if(jerror.error != QJsonParseError::NoError) {
		errorText = "Frequency calibration file parsing error";
		success = false;
		return ret;
	}
	QJsonObject obj = jdoc.object();
	ret.calPower = obj["CalPower"].toDouble();
	ret.calDate = obj["CalDate"].toString();
	QJsonArray values = obj["FreqTable"].toArray();
	QString id,title,desc,comment;
	foreach(QJsonValue val, values)
	{
		const QJsonObject& obj = val.toObject();
		ret.freqToPower.insert(obj["Frequency(MHz)"].toDouble(), obj["Power(db)"].toDouble());
	}
	return  ret;
}

QList<calParser::magPhaseCalData> calParser::loadMagPhaseCalDataFromFile(QString file, bool &success, QString &errorText)
{
	if(file.isEmpty())
		file = getConfigLocation() + QDir::separator() + STANDARD_PATHS_CAL_FILENAME;
	success = true;
	QList<calParser::magPhaseCalData> ret;
	QFile f(file);
	if(!f.open(QIODevice::ReadWrite)) {
		errorText = "Could not open path calibration file";
		success = false;
		return ret;
	}
	QJsonParseError jerror;
	QJsonDocument jdoc= QJsonDocument::fromJson(f.readAll(),&jerror);
	if(jerror.error != QJsonParseError::NoError) {
		errorText = "Path calibration file parsing error";
		success = false;
		return ret;
	}
	QJsonObject obj = jdoc.object();
	if(obj["Type"] != "Path Calibration")
		success = false;
	QJsonArray paths = obj["Paths"].toArray();
	foreach(QJsonValue path, paths) {
		calParser::magPhaseCalData data;
		data.calFrequency = path["CalFrequency"].toDouble();
		data.calDate = path["CalDate"].toString();
		data.pathName = path["PathName"].toString();
		data.centerFreq_MHZ = path["FinalFilterFrequency"].toDouble();
		data.bandwidth_MHZ = path["FinalFilterBandwidth"].toDouble();
		data.controlPin = path["ControlPin"].toInt();
		QJsonArray values = path["MagTable"].toArray();
		QString id,title,desc,comment;
		foreach(QJsonValue val, values)
		{
			const QJsonObject& obj = val.toObject();
			magCalFactors m;
			m.dbm_val = obj["dBm"].toDouble();
			m.phase_val = obj["Phase"].toDouble();
			data.adcToMagCalFactors.insert(uint(obj["ADC"].toInt()), m);
		}
		ret.append(data);
	}
	return  ret;
}

calParser::freqCalData calParser::importFreqCalFromOriginalSW(QString file, bool &success)
{
	success = true;
	calParser::freqCalData ret;
	QFile f(file);
	if(!f.open(QIODevice::ReadOnly)) {
		success = false;
		return ret;
	}
	bool ok;
	while (!f.atEnd()) {
		QByteArray line = f.readLine();
		QRegularExpression re("(\\d+.\\d+)\\s+(-?\\d+.\\d+)");
		QRegularExpression ver("\\s*CalVersion=\\s+(\\d+.\\d+)");
		QRegularExpression datePow("\\.*Calibrated\\s+(\\d{2}/\\d{2}/\\d{2})\\s+at\\s+(-?\\d+.\\d+)");
		QRegularExpressionMatch match = re.match(line);
		QRegularExpressionMatch datePowmatch = datePow.match(line);
		QRegularExpressionMatch vermatch = ver.match(line);
		if (match.hasMatch()) {
			double freq = match.captured(1).toDouble(&ok);
			if(!ok)
				success = false;
			double power = match.captured(2).toDouble(&ok);
			if(!ok)
				success = false;
			if(success)
				ret.freqToPower.insert(freq, power);
		}
		else if(vermatch.hasMatch()) {
			//vermatch.captured(1);
		}
		else if(datePowmatch.hasMatch()) {
			ret.calDate = QDateTime::fromString(datePowmatch.captured(1), "MM/dd/yy").addYears(100).toString();
			ret.calPower = datePowmatch.captured(2).toDouble(&ok);
			if(!ok)
				success = false;
		}
	}
	return ret;
}

calParser::magPhaseCalData calParser::importMagPhaseCalFromOriginalSW(QString file, bool &success)
{
	success = true;
	calParser::magPhaseCalData ret;
	ret.controlPin = -1;
	ret.pathName = "Imported";
	QFile f(file);
	if(!f.open(QIODevice::ReadOnly)) {
		success = false;
		return ret;
	}
	bool ok;
	while (!f.atEnd()) {
		QByteArray line = f.readLine();
		//*Filter Path 1: CenterFreq=10.700000 MHz; Bandwidth=7.500000 kHz
		QRegularExpression re("\\.*CenterFreq=(\\d+.\\d+)\\s(...).*Bandwidth=(\\d+.\\d+)\\s(.{2,3})\\s");
		QRegularExpression ver("\\s*CalVersion=\\s+(\\d+.\\d+)");
		QRegularExpression datePow("\\.*Calibrated\\s+(\\d{2}/\\d{2}/\\d{2})\\s+at\\s+(-?\\d+.\\d+)");
		QRegularExpression vals("(\\d+.\\d+)\\s+(-?\\d+.\\d+)\\s+(-?\\d+.\\d+)");
		QRegularExpressionMatch match = re.match(line);
		QRegularExpressionMatch datePowmatch = datePow.match(line);
		QRegularExpressionMatch vermatch = ver.match(line);
		QRegularExpressionMatch valsmatch = vals.match(line);
		if (match.hasMatch()) {
			double f = match.captured(1).toDouble(&ok);
			if(!ok)
				success = false;
			double b = match.captured(3).toDouble(&ok);
			if(!ok)
				success = false;
			QString funit = match.captured(2);
			funit = funit.toUpper();
			double mult;
			if(funit.contains("MHZ"))
				mult = 1.0;
			else if(funit.contains("KHZ"))
				mult = 0.001;
			else {
				mult = 1.0;
			}
			f = f * mult;
			QString bunit = match.captured(4);
			bunit = bunit.toUpper();
			if(bunit.contains("MHZ"))
				mult = 1.0;
			else if(bunit.contains("KHZ"))
				mult = 0.001;
			else if(bunit.contains("HZ"))
				mult = 0.000001;
			else {
				mult = 1.0;
			}
			b = b * mult;
			ret.centerFreq_MHZ = f;
			ret.bandwidth_MHZ = b;
		}
		else if(vermatch.hasMatch()) {
			//vermatch.captured(1);
		}
		else if(datePowmatch.hasMatch()) {
			ret.calDate = QDateTime::fromString(datePowmatch.captured(1), "MM/dd/yy").addYears(100).toString();
			ret.calFrequency = datePowmatch.captured(2).toDouble(&ok);
			if(!ok)
				success = false;
		}
		else if(valsmatch.hasMatch()) {
			uint adc = valsmatch.captured(1).toUInt(&ok);
			if(!ok)
				success = false;
			double mag = valsmatch.captured(2).toDouble(&ok);
			if(!ok)
				success = false;
			double phase = valsmatch.captured(3).toDouble(&ok);
			if(!ok)
				success = false;
			magCalFactors m;
			m.dbm_val = mag;
			m.phase_val = phase;
			ret.adcToMagCalFactors.insert(adc, m);
		}
	}
	return ret;
}

QString calParser::getConfigLocation()
{
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}
