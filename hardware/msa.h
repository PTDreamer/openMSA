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

class hardwareDevice;
class interface;
class msa
{
	public:
	typedef enum {PLL1, PLL2, PLL3, DDS1, DDS3, ADC_MAG, ADC_PH} MSAdevice;
		static msa& getInstance()
		{
			static msa    instance;
			return instance;
		}
		QHash<msa::MSAdevice, hardwareDevice *> currentHardwareDevices;
	private:
		msa() {}
		interface *currentInterface;
		bool isInverted;
	public:
		msa(msa const&)               = delete;
		void operator=(msa const&)  = delete;
		void hardwareInit(QHash<MSAdevice, int> devices, interface *usedInterface);
		typedef enum {SA, SA_TG, SA_SG, VNA} scanType_t;
		typedef struct {
			double translatedFrequency;
			double realFrequency;
			double LO1;
			double LO3;
			int band;
		} scanStep;

		typedef struct {
			double baseFrequency;
			double LO2; // from configuration
			double finalFilterFrequency; //final filter frequency;
			double finalFilterBandwidth; //final filter bandwidth;
			double TGoffset; //tracking generator offset
			bool   TGreversed;
			double SGout; //signal generator output frequency
			double appxdds1; // center freq. of DDS1 xtal filter; exact value determined in calibration
			double appxdds3; // center freq. of DDS3 xtal filter; exact value determined in calibration
			double dds3Filterbandwidth;
			double dds1Filterbandwidth;
			double PLL1phasefreq; // from configuration default=0.974
			double PLL2phasefreq; // 4
			double PLL3phasefreq; // from configuration default=0.974
			double masterOscilatorFrequency;
			uint8_t adcAveraging;
			scanType_t scanType;
		} scanConfig;
		typedef struct {
			scanConfig configuration;
			QHash<quint32, scanStep> steps;
		} scanStruct;
		scanStruct currentScan;
		void setScanConfiguration(msa::scanConfig configuration);
		void initScan(bool inverted, double start, double end, double step, int band = -1);
		bool getIsInverted() const;
};

#endif // MSA_H
