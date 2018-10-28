/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      ad9850.h file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   ad9850
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
#ifndef AD9850_H
#define AD9850_H

#include <QString>
#include "global_defs.h"
#include <QtEndian>
#include <QHash>
#include <QBitArray>
#include "hardware/hardwaredevice.h"
#include "deviceparser.h"

class ad9850 : public genericDDS
{
	Q_OBJECT
public:
	explicit ad9850(hardwareDevice::MSAdevice device, QObject *parent = 0);
	void processNewScan();
	bool init();
	void reinit();
	// gets the type of CLK this device needs, dedicated or system wide
	clockType getClk_type() const;
	typedef enum {PIN_DATA, PIN_FQUD, PIN_WCLK, PIN_VIRTUAL_CLOCK} pins;
private:
	typedef enum {FIELD_FREQUENCY, FIELD_CONTROL, FIELD_POWER, FIELD_PHASE} fields_type;
	bool checkSettings();
	quint64 deviceRegister;
	void registerToBuffer(quint64 *reg, int pin, quint32 step);
signals:

public slots:
};

#endif // AD9850_H
