/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      msa.h file
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
#ifndef MSA_H
#define MSA_H

#include <QHash>
#include "../shared/comprotocol.h"
#include "calparser.h"

typedef enum {INFO, WARNING, ERROR} message_type;

class MainWindow;
class hardwareDevice;
class interface;
class msa
{
public:
	typedef enum {PLL1, PLL2, PLL3, DDS1, DDS3, ADC_MAG, ADC_PH, MSA} MSAdevice;
	static msa& getInstance()
	{
		static msa    instance;
		return instance;
	}
	QHash<msa::MSAdevice, hardwareDevice *> currentHardwareDevices;
	interface *currentInterface;
private:
	msa() {}
	bool isInverted;
	int resolution_filter_bank;
	MainWindow *mw;
public:
	msa(msa const&)               = delete;
	void operator=(msa const&)  = delete;
	void hardwareInit(QHash<MSAdevice, int> devices, interface *usedInterface);
	typedef struct {
		double translatedFrequency;
		double realFrequency;
		double LO1;
		double LO3;
		double DDS1;
		double DDS2;
		double DDS3;
		int band;
		double frequencyCal;
	} scanStep;
        typedef  struct {
            int address;
            double value;
        } videoFilter_t;
	typedef  struct {
		bool isForced;
		double outputFreq;
		double oscFreq;
	} forceDDS;
//        typedef  struct {
//            int address;
//            double centerFrequency;
//            double bandwidth;
//        } resolutionFilter_t;

	typedef struct {
		QHash<QString, videoFilter_t> videoFilters;
		calParser::freqCalData frequencyCalibration;
		QList<calParser::magPhaseCalData> pathCalibrationList;
		calParser::magPhaseCalData pathCalibration;
		QString currentFinalFilterName;
		QString currentVideoFilterName;
		double baseFrequency;
		double LO2; // from configuration
		double appxdds1; // center freq. of DDS1 xtal filter; exact value determined in calibration
		double appxdds3; // center freq. of DDS3 xtal filter; exact value determined in calibration
		double dds3Filterbandwidth;
		double dds1Filterbandwidth;
		double PLL1phasefreq; // from configuration default=0.974
		double PLL2phasefreq; // 4
		double PLL3phasefreq; // from configuration default=0.974
		bool   PLL1phasepolarity_inverted;
		bool   PLL2phasepolarity_inverted;
		bool   PLL3phasepolarity_inverted;
		unsigned int PLL1pin14Output;
		unsigned int PLL3pin14Output;
		double masterOscilatorFrequency;
		double PDMInversion_degrees;
		quint32 PDMMaxOut;
		uint8_t adcAveraging;
		ComProtocol::scanType_t scanType;
		ComProtocol::msg_scan_config gui;
		forceDDS forcedDDS1;
		forceDDS forcedDDS3;
                bool cavityTestRunning;
	} scanConfig;
	typedef struct {
		scanConfig configuration;
		QHash<quint32, scanStep> *steps;
	} scanStruct;
	scanStruct currentScan;
	void setScanConfiguration(msa::scanConfig configuration);
	bool initScan(bool inverted, double start, double end, double step_freq, int band = -1);
	bool initScan(bool inverted, double start, double end, quint32 steps, int band = -1);
	bool getIsInverted() const;
	int getResolution_filter_bank() const;
	void setResolution_filter_bank(int value);
	void setMainWindow(MainWindow *window);
	msa::scanConfig getScanConfiguration();
	bool setPathCalibrationAndExtrapolate(QString pathName);
	void extrapolateFrequenctCalibrationForCurrentScan();
	QList<std::function<void(scanConfig)>> scanConfigChangedCallbacks;
	void addScanConfigChangedCallback(std::function<void(scanConfig)> callback);
	bool initScan(ComProtocol::msg_scan_config msg);
};

#endif // MSA_H
