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
		} scanStep;

		typedef struct {
			double baseFrequency;
			double LO2; // from configuration
			double finalFilterFrequency; //final filter frequency;
			double finalFilterBandwidth; //final filter bandwidth;
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
			double masterOscilatorFrequency;
			uint8_t adcAveraging;
			ComProtocol::scanType_t scanType;
			ComProtocol::msg_scan_config gui;
		} scanConfig;
		typedef struct {
			scanConfig configuration;
			QHash<quint32, scanStep> *steps;
		} scanStruct;
		scanStruct currentScan;
		void setScanConfiguration(msa::scanConfig configuration);
		void initScan(bool inverted, double start, double end, double step_freq, int band = -1);
        void initScan(bool inverted, double start, double end, quint32 steps, int band = -1);
		bool getIsInverted() const;
		int getResolution_filter_bank() const;
		void setResolution_filter_bank(int value);

		msa::scanConfig getScanConfiguration();
};

#endif // MSA_H
