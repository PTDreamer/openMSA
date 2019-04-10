/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      msa.cpp file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   msa
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
#include "msa.h"
#include "genericadc.h"
#include "ad9850.h"
#include "lmx2326.h"
#include "controllers/interface.h"
#include "hardwaredevice.h"
#include <QDebug>

bool msa::getIsInverted() const
{
	return isInverted;
}

int msa::getResolution_filter_bank() const
{
	return resolution_filter_bank;
}

void msa::setResolution_filter_bank(int value)
{
	resolution_filter_bank = value;
}

void msa::hardwareInit(QHash<MSAdevice, int> devices, interface *usedInterface)
{
	currentInterface = usedInterface;
	qDeleteAll(currentHardwareDevices);
	currentHardwareDevices.clear();
	foreach (msa::MSAdevice dev, devices.keys()) {
		switch (devices.value(dev)) {
		case hardwareDevice::LMX2326:
			currentHardwareDevices.insert(dev, new lmx2326(dev, usedInterface));
			break;
		case hardwareDevice::AD9850:
			currentHardwareDevices.insert(dev, new ad9850(dev, usedInterface));
			break;
		case hardwareDevice::AD7685:
		case hardwareDevice::LT1865:
			currentHardwareDevices.insert(dev, new genericADC(dev, hardwareDevice::HWdevice(devices.value(dev)), usedInterface));
			break;
		default:
			break;
		}
	}
	currentInterface->hardwareInit();
}

bool msa::initScan(bool inverted, double start, double end, quint32 steps, int band)
{
	msa::scanConfig cfg = msa::getInstance().getScanConfiguration();
	cfg.gui.start  = start;
	cfg.gui.stop = end;
	cfg.gui.steps_number = steps;
	cfg.gui.band = band;
	//TODO CalculateAllStepsForLO3Synth
	msa::getInstance().currentScan.steps->clear();
	double step = (end - start) / double(steps);
	if(step > msa::getInstance().currentScan.configuration.pathCalibration.bandwidth_MHZ)
		msa::getInstance().currentInterface->errorOcurred(msa::MSA, "Frequency step size exceeds final filter bandwidth signals may be missed.", false, true);
	cfg.gui.step_freq = step;
	msa::getInstance().setScanConfiguration(cfg);
	int thisBand = 0;
	int bandSelect = 0;
    for(quint32 x = 0; x < steps; ++x) {
		msa::scanStep s;
		s.realFrequency = start + (x * step);
		if(band < 0) {
			if(s.realFrequency < 1000)
				thisBand = 1;
			else if(s.realFrequency < 2000)
				thisBand = 2;
			else
				thisBand = 3;
		}
		s.translatedFrequency = s.realFrequency;
		double IF1;
		if(band < 0)
			bandSelect = thisBand;
		else
			bandSelect = band;
		switch (bandSelect) {
		case 2:
			s.translatedFrequency = s.translatedFrequency - msa::getInstance().currentScan.configuration.LO2;
			break;
		case 3:
			IF1 = msa::getInstance().currentScan.configuration.LO2 - msa::getInstance().currentScan.configuration.pathCalibration.centerFreq_MHZ;
			s.translatedFrequency = s.translatedFrequency - 2*IF1;
			break;
		default:
			break;
		}
		s.band = bandSelect;
		//qDebug() << "step:" << x << "real frequency:" << s.realFrequency << "translated frequency:" << s.translatedFrequency;
		msa::getInstance().currentScan.steps->insert(x, s);
	}
	isInverted = inverted;
	extrapolateFrequenctCalibrationForCurrentScan();
	return currentInterface->initScan();
}

bool msa::initScan(bool inverted, double start, double end, double step_freq, int band)
{
    quint32 steps = quint32((end - start) / step_freq);
	return initScan(inverted, start, end, steps, band);
}


msa::scanConfig msa::getScanConfiguration()
{
	return msa::getInstance().currentScan.configuration;
}

void msa::setScanConfiguration(msa::scanConfig configuration)
{
	msa::getInstance().currentScan.configuration = configuration;
}

void msa::extrapolateFrequenctCalibrationForCurrentScan() {
	QList<quint32> ksteps = msa::getInstance().currentScan.steps->keys();
	QList<double> fcsteps = msa::getInstance().currentScan.configuration.frequencyCalibration.freqToPower.keys();
	QHash<double, double> fTod = msa::getInstance().currentScan.configuration.frequencyCalibration.freqToPower;
	std::sort(ksteps.begin(), ksteps.end());
	std::sort(fcsteps.begin(), fcsteps.end());
	for (int x = 0; x < ksteps.length(); ++x) {
		double f = msa::getInstance().currentScan.steps->value(ksteps.at(x)).realFrequency;
		if(f < fcsteps.first()) {
			(*(msa::getInstance().currentScan.steps))[ksteps.at(x)].frequencyCal = msa::getInstance().currentScan.configuration.frequencyCalibration.freqToPower.value(fcsteps.first());
		}
		else if(f > fcsteps.last()) {
			(*(msa::getInstance().currentScan.steps))[ksteps.at(x)].frequencyCal = msa::getInstance().currentScan.configuration.frequencyCalibration.freqToPower.value(fcsteps.last());
		}
		else {
			for (int y = 0; y < fcsteps.length(); ++y) {
				if(fcsteps.at(y) == f) {
					(*msa::getInstance().currentScan.steps)[(ksteps.at(x))].frequencyCal = fTod.value(fcsteps.at(y));
					break;
				}
//ret.adcToMagCalFactors.value(values.at(int(y))).dbm_val - double(values.at(int(y)) - x)*(ret.adcToMagCalFactors.value(values.at(int(y))).dbm_val - ret.adcToMagCalFactors.value(values.at(int(y -1))).dbm_val) / double(values.at(int(y))-values.at(int(y - 1)));
				else if(fcsteps.at(y) > f) {
					if(y == 0)
						continue;
					double ff = fTod.value(fcsteps.at(int(y)))
							- double(fcsteps.at(int(y)) - f) *
							(fTod.value(fcsteps.at(int(y))) - fTod.value(fcsteps.at(int(y -1)))) /
							double(fcsteps.at(int(y))-fcsteps.at(int(y - 1)));
					(*msa::getInstance().currentScan.steps)[(ksteps.at(x))].frequencyCal = ff;
					break;
				}
			}
		}
	}
}
bool msa::setPathCalibrationAndExtrapolate(QString pathName)
{
	bool found = false;
	calParser::magPhaseCalData ret;
	foreach (calParser::magPhaseCalData data, msa::getInstance().currentScan.configuration.pathCalibrationList) {
		if(data.pathName.contains(pathName)) {
			ret = data;
			found = true;
			break;
		}
	}
	if(!found)
		return false;
	msa::getInstance().currentScan.configuration.pathCalibration = ret;
	msa::getInstance().currentScan.configuration.pathCalibration.adcToMagCalFactors.clear();
	QList<uint> values = ret.adcToMagCalFactors.keys();
	std::sort(values.begin(), values.end());
	for(uint x = 0; x < 0xFFFF; ++x) {
		if(x < values.first())
			msa::getInstance().currentScan.configuration.pathCalibration.adcToMagCalFactors.insert(x, ret.adcToMagCalFactors.value(values.first()));
		else if(x > values.last())
			msa::getInstance().currentScan.configuration.pathCalibration.adcToMagCalFactors.insert(x, ret.adcToMagCalFactors.value(values.last()));
		else {
			for(uint y = 0; y < uint(values.length()); ++y) {
				if(x == values.at(int(y))) {
					msa::getInstance().currentScan.configuration.pathCalibration.adcToMagCalFactors.insert(x, ret.adcToMagCalFactors.value(values.at(int(y))));
					break;
				}
				if(values.at(int(y)) > x) {
					calParser::magCalFactors fact;
					fact.dbm_val = ret.adcToMagCalFactors.value(values.at(int(y))).dbm_val - double(values.at(int(y)) - x)*(ret.adcToMagCalFactors.value(values.at(int(y))).dbm_val - ret.adcToMagCalFactors.value(values.at(int(y -1))).dbm_val) / double(values.at(int(y))-values.at(int(y - 1)));
					fact.phase_val = ret.adcToMagCalFactors.value(values.at(int(y))).phase_val - (values.at(int(y)) - x)*(ret.adcToMagCalFactors.value(values.at(int(y))).phase_val - ret.adcToMagCalFactors.value(values.at(int(y -1))).phase_val) / (values.at(int(y))-values.at(int(y - 1)));
					msa::getInstance().currentScan.configuration.pathCalibration.adcToMagCalFactors.insert(x, fact);
					break;
				}
			}
		}
	}
	return true;
}
