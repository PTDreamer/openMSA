#include "lmx2326.h"
#include <QDebug>

lmx2326::lmx2326(deviceParser::MSAdevice device, QObject *parent):genericPLL(parent)
{
	registerSize = 21;
	parser = new deviceParser(device, this);
	s.latches = 0;
	s.ncounter = 0;
	s.rcounter = 0;
	QList<int> bits;
	//bit numbering follows ADF4118 datasheet, LMX2326 is inverted
	bits << 20;
	fillField(R_LD, bits, &s.rcounter);
	bits.clear();
	bits << 19 << 18<< 17 << 16;
	fillField(R_TESTMODES, bits, &s.rcounter);
	bits.clear();
	bits << 15 << 14 << 13 << 12 << 11 << 10 << 9 << 8 << 7 << 6
		 << 5 << 4 << 3<< 2;
	fillField(R_DIVIDER, bits, &s.rcounter);
	bits.clear();
	bits << 1 << 0;
	fillField(R_CC, bits, &s.rcounter);
	fillField(L_CC, bits, &s.latches);
	fillField(N_CC, bits, &s.ncounter);
	bits.clear();
	bits << 2 << 3 << 4 << 5 << 6;
	fillField(N_ACOUNTER_DIVIDER, bits, &s.ncounter);
	bits.clear();
	bits << 7 << 8 << 9 << 10 << 11 << 12 << 13 << 14 << 15 << 16 << 17 << 18 << 19;
	fillField(N_BCOUNTER_DIVIDER, bits, &s.ncounter);
	bits.clear();
	bits << 20;
	fillField(N_CPGAIN_BIT, bits, &s.ncounter);
	bits.clear();
	bits << 2;
	fillField(L_COUNTER_RESET, bits, &s.latches);
	bits.clear();
	bits << 3;
	fillField(L_POWER_DOWN, bits, &s.latches);
	bits.clear();
	bits << 4<<5<<6;
	fillField(L_FO_LD, bits, &s.latches);
	bits.clear();
	bits << 7;
	fillField(L_PH_DET_POLARITY, bits, &s.latches);
	bits.clear();
	bits << 8;
	fillField(L_CP, bits, &s.latches);
	bits.clear();
	bits << 9<<10<<11;
	fillField(L_FASTLOCK, bits, &s.latches);
	bits.clear();
	bits << 12<<13<<14<<15;
	fillField(L_TIMEOUT, bits, &s.latches);
	bits.clear();
	bits << 16<<17<<18;
	fillField(L_TESTMODES, bits, &s.latches);
	bits.clear();
	bits << 19;
	fillField(L_POWER_DOWN_MODE, bits, &s.latches);
	bits.clear();
	bits << 20;
	fillField(L_TESTMODE, bits, &s.latches);
	setFieldRegister(L_CC, (int)control_field::FUNCTION_LATCH);
	setFieldRegister(L_FO_LD, (int)FoLD_field::R_DIVIDER_OUT);
	qDebug() << convertToStr(&s.latches);

	devicePin *pin = new devicePin;
	pin->dataArray = QHash<quint32, QBitArray *>();
	pin->name = "Data";
	pin->type = hardwareDevice::INPUT;
	devicePins.insert(PIN_DATA, pin);
	pin = new devicePin;
	pin->dataArray = QHash<quint32, QBitArray *>();
	pin->name = "Load Enable";
	pin->type = hardwareDevice::INPUT;
	devicePins.insert(PIN_LE, pin);
	pin = new devicePin;
	pin->dataArray = QHash<quint32, QBitArray *>();
	pin->name = "Clock";
	pin->type = hardwareDevice::CLK;
	devicePins.insert(PIN_CLK, pin);
}

hardwareDevice::clockType lmx2326::getClk_type() const
{
	return hardwareDevice::CLOCK_RISING_EDGE;
}

void lmx2326::processNewScan(scanStruct scan)
{
	//rcounter = appxdds1/PLL1phasefreq
	//appxdds1 = rcounter * PLL1phasefreq
	//
	//fvco e [(32 x B) + A] x fosc/R(11)
	double rcounter = parser->parsePLLRCounter(scan.configuration);//10.7/0.974 = 11
	setFieldRegister(R_DIVIDER, rcounter);
	setFieldRegister(R_LD, 0);
	setFieldRegister(R_TESTMODES, 0);
	registerToBuffer(&s.rcounter, PIN_DATA, INIT_STEP);
	qDebug() << convertToStr(&s.rcounter) << rcounter;
	qDebug() << *devicePins.value(PIN_DATA)->dataArray.value(INIT_STEP);
	addLEandCLK(INIT_STEP);
	qDebug() << *devicePins.value(PIN_DATA)->dataArray.value(INIT_STEP);
	qDebug() << *devicePins.value(PIN_LE)->dataArray.value(INIT_STEP);
	foreach (int step, scan.steps.keys()) {
		double ncounter = parser->parsePLLNCounter(scan.configuration, scan.steps[step],step);
		double Bcounter = floor(ncounter/32);
		double Acounter = round(ncounter-(Bcounter*32));
		setFieldRegister(N_ACOUNTER_DIVIDER, Acounter);
		setFieldRegister(N_BCOUNTER_DIVIDER, Bcounter);
		setFieldRegister(N_CC, (int)control_field::NCOUNTER);
		setFieldRegister(N_CPGAIN_BIT, (int)cp_gain::HIGH);//Phase Det Current, 1= 1 ma, 0= 250 ua
		registerToBuffer(&s.ncounter, PIN_DATA, step);
		addLEandCLK(step);
	}
}

void lmx2326::init()
{
	setFieldRegister(R_CC, (int)control_field::RCOUNTER);
	setFieldRegister(N_CC, (int)control_field::NCOUNTER);
	setFieldRegister(L_CC, (int)control_field::INIT);
	setFieldRegister(L_POWER_DOWN_MODE, 0);
	setFieldRegister(L_COUNTER_RESET, 0);
	setFieldRegister(L_POWER_DOWN, 0);
	setFieldRegister(L_FO_LD, (int)FoLD_field::TRI_STATE);
	setFieldRegister(L_PH_DET_POLARITY, (int)phase_detector::NON_INVERTED);
	setFieldRegister(L_CP, (int)cp_tri_state::NORMAL);
	setFieldRegister(L_FASTLOCK, 0);
	setFieldRegister(L_TIMEOUT, 0);
	setFieldRegister(L_TESTMODES, 0);
	setFieldRegister(L_POWER_DOWN_MODE, 0);
	setFieldRegister(L_TESTMODE, 0);
	registerToBuffer(&s.latches, PIN_DATA, INIT_STEP);
	addLEandCLK(INIT_STEP);
	qDebug() << *devicePins.value(PIN_DATA)->dataArray.value(0);
	qDebug() << *devicePins.value(PIN_LE)->dataArray.value(0);
	qDebug() << *devicePins.value(PIN_CLK)->dataArray.value(0);
}

void lmx2326::reinit()
{

}

lmx2326::~lmx2326()
{

}

int lmx2326::getRCounter()
{
	return getFieldRegister(R_DIVIDER);
}

bool lmx2326::checkSettings()
{
	if(getFieldRegister(N_ACOUNTER_DIVIDER) > getFieldRegister(N_BCOUNTER_DIVIDER))
		return false;
	else if(getFieldRegister(N_BCOUNTER_DIVIDER) < 3)
		return false;
	else if(getFieldRegister(R_DIVIDER) < 3)
		return false;
	return true;
}

double lmx2326::getVcoFrequency(double external_clock_frequency)
{
	double ret;
	ret = ((double)32 * (double)getFieldRegister(N_BCOUNTER_DIVIDER)+ (double)getFieldRegister(N_ACOUNTER_DIVIDER)) * (external_clock_frequency / (double)getFieldRegister(R_DIVIDER));
	qDebug() << ((32 * (double)getFieldRegister(N_BCOUNTER_DIVIDER))+ (double)getFieldRegister(N_ACOUNTER_DIVIDER));
	qDebug() << (external_clock_frequency / (double)getFieldRegister(R_DIVIDER));

	return ret;
}

bool lmx2326::addLEandCLK(quint32 step)
{
	devicePin *cl = devicePins.value(PIN_CLK);
	if(!cl->dataArray.contains(step))
		cl->dataArray.insert(step, new QBitArray(registerSize));
	cl->dataArray.value(step)->fill(true);
	devicePin *le = devicePins.value(PIN_LE);
	if(!le->dataArray.contains(step))
		le->dataArray.insert(step, new QBitArray(registerSize + 1));
	else if(le->dataArray.value(step)->count() != (registerSize + 1))
		le->dataArray.value(step)->resize(registerSize + 1);
	le->dataArray.value(step)->setBit(registerSize);
	return true;
}
