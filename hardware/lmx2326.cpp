#include "lmx2326.h"
#include <QDebug>

lmx2326::lmx2326(msa::MSAdevice device, QObject *parent):genericPLL(parent)
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

	devicePin *pin = new devicePin;
	pin->name = "Data";
	pin->IOtype = hardwareDevice::MAIN_DATA;
	devicePins.insert(PIN_DATA, pin);
	pin = new devicePin;
	pin->name = "Load Enable";
	pin->IOtype = hardwareDevice::GEN_INPUT;
	devicePins.insert(PIN_LE, pin);
	pin = new devicePin;
	pin->name = "Clock";
	pin->IOtype = hardwareDevice::CLK;
	devicePins.insert(PIN_CLK, pin);
	pin = new devicePin;
	pin->name = "";
	pin->IOtype = hardwareDevice::VIRTUAL_CLK;
	devicePins.insert(PIN_VIRTUAL_CLOCK, pin);
	//qDebug() << "DURING INIT" << s.ncounter;
	Q_ASSERT(s.ncounter <= 0b111111111111111111111);

}

hardwareDevice::clockType lmx2326::getClk_type() const
{
	return hardwareDevice::CLOCK_RISING_EDGE;
}

void lmx2326::processNewScan()
{
	//qDebug() << "lmx2326 starting processNewScan";
	double ncounter = 0;
	double Bcounter = 0;
	double Acounter = 0;
	QList<quint32> index;
	index.append(msa::getInstance().currentScan.steps.keys());
	std::sort(index.begin(), index.end());

	foreach (int step, index) {
		if(initIndexes.contains(step))
				continue;
		ncounter = parser->parsePLLNCounter(msa::getInstance().currentScan.configuration, msa::getInstance().currentScan.steps[step],step);
		Bcounter = floor(ncounter/32);
		Acounter = round(ncounter-(Bcounter*32));
		setFieldRegister(N_CC, (int)control_field::NCOUNTER);
		setFieldRegister(N_CPGAIN_BIT, (int)cp_gain::HIGH);//Phase Det Current, 1= 1 ma, 0= 250 ua
		registerToBuffer(&s.ncounter, PIN_DATA, step);
		addLEandCLK(step);
		config[step] = s;
		//qDebug() << "step:"<<"Acounter" << Acounter << "Bcounter" << Bcounter << s.ncounter;
		//qDebug() << "step:"<<convertToStr(&s.ncounter);

	}
}

bool lmx2326::init()
{
	setFieldRegister(R_CC, (int)control_field::RCOUNTER);
	setFieldRegister(N_CC, (int)control_field::NCOUNTER);
	setFieldRegister(L_CC, (int)control_field::INIT);
	setFieldRegister(L_POWER_DOWN_MODE, 0);
	setFieldRegister(L_COUNTER_RESET, 0);
	setFieldRegister(L_POWER_DOWN, 0);
	setFieldRegister(L_FO_LD, (int)FoLD_field::TRI_STATE);
	if(parser->getPLLinverted((msa::getInstance().currentScan.configuration)))
		setFieldRegister(L_PH_DET_POLARITY, (int)phase_detector::INVERTED);
	else
		setFieldRegister(L_PH_DET_POLARITY, (int)phase_detector::NON_INVERTED);
	setFieldRegister(L_CP, (int)cp_tri_state::NORMAL);
	setFieldRegister(L_FASTLOCK, 0);
	setFieldRegister(L_TIMEOUT, 0);
	setFieldRegister(L_TESTMODES, 0);
	setFieldRegister(L_POWER_DOWN_MODE, 0);
	setFieldRegister(L_TESTMODE, 0);
	registerToBuffer(&s.latches, PIN_DATA, HW_INIT_STEP);
	addLEandCLK(HW_INIT_STEP);
	//qDebug() << "lmx2326 initData" << *devicePins.value(PIN_DATA)->data.value(HW_INIT_STEP).dataArray;
	//qDebug() << "lmx2326 initMask" << *devicePins.value(PIN_DATA)->data.value(HW_INIT_STEP).dataMask;
	//qDebug() << "lmx2326 initleda" << *devicePins.value(PIN_LE)->data.value(HW_INIT_STEP).dataArray;
	//qDebug() << "lmx2326 initlema" << *devicePins.value(PIN_LE)->data.value(HW_INIT_STEP).dataMask;
	////qDebug() << "lmx2326 initclkd" << *devicePins.value(PIN_CLK)->data.value(INIT_STEP).dataArray;
	////qDebug() << "lmx2326 initclkm" << *devicePins.value(PIN_CLK)->data.value(INIT_STEP).dataMask;
	//qDebug() << "lmx2326 initvcld" << *devicePins.value(PIN_VIRTUAL_CLOCK)->data.value(HW_INIT_STEP).dataArray;
	//qDebug() << "lmx2326 initvclm" << *devicePins.value(PIN_VIRTUAL_CLOCK)->data.value(HW_INIT_STEP).dataMask;

	initIndexes.clear();
	initIndexes.append(HW_INIT_STEP);
	config[HW_INIT_STEP] = s;
	double rcounter = parser->parsePLLRCounter(msa::getInstance().currentScan.configuration);//10.7/0.974 = 11
	//qDebug()<<"RCOUNTER"<<rcounter;
	setFieldRegister(R_DIVIDER, rcounter);
	setFieldRegister(R_LD, 0);
	setFieldRegister(R_TESTMODES, 0);
	registerToBuffer(&s.rcounter, PIN_DATA, HW_INIT_STEP -1);
	//qDebug() << "LMX2326 SCAN_INIT_STEP rcounter:" << convertToStr(&s.rcounter) << rcounter;
	//qDebug() << "LMX2326 SCAN_INIT_STEP PIN_DATA dataArray:" << *devicePins.value(PIN_DATA)->data.value(HW_INIT_STEP-1).dataArray;
	addLEandCLK(HW_INIT_STEP - 1);
	config[HW_INIT_STEP - 1] = s;
	initIndexes.append(HW_INIT_STEP - 1);
	msa::scanStep st;
	msa::getInstance().currentScan.steps.insert(HW_INIT_STEP, st);
	double ncounter = parser->parsePLLNCounter(msa::getInstance().currentScan.configuration, msa::getInstance().currentScan.steps[HW_INIT_STEP],HW_INIT_STEP);
	if(ncounter > 0) {
		double Bcounter = floor(ncounter/32);
		double Acounter = round(ncounter-(Bcounter*32));
		//qDebug() << "PLL2 Acounter" << Acounter << "Bcounter" << Bcounter;
		setFieldRegister(N_ACOUNTER_DIVIDER, Acounter);
		setFieldRegister(N_BCOUNTER_DIVIDER, Bcounter);
		setFieldRegister(N_CC, (int)control_field::NCOUNTER);
		setFieldRegister(N_CPGAIN_BIT, (int)cp_gain::HIGH);//Phase Det Current, 1= 1 ma, 0= 250 ua
		registerToBuffer(&s.ncounter, PIN_DATA, HW_INIT_STEP-2);
		addLEandCLK(HW_INIT_STEP - 2);
		initIndexes.append(HW_INIT_STEP - 2);
	}
	config[HW_INIT_STEP - 2] = s;
	return true;
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

QHash<quint32, lmx2326_struct> lmx2326::getConfig() const
{
	return config;
}

double lmx2326::getVcoFrequency(double external_clock_frequency)
{
	double ret;
	ret = ((double)32 * (double)getFieldRegister(N_BCOUNTER_DIVIDER)+ (double)getFieldRegister(N_ACOUNTER_DIVIDER)) * (external_clock_frequency / (double)getFieldRegister(R_DIVIDER));
	//qDebug() << ((32 * (double)getFieldRegister(N_BCOUNTER_DIVIDER))+ (double)getFieldRegister(N_ACOUNTER_DIVIDER));
	//qDebug() << (external_clock_frequency / (double)getFieldRegister(R_DIVIDER));

	return ret;
}

bool lmx2326::addLEandCLK(quint32 step)
{
	int totalSize = registerSize + 2;
	resizePinData(&devicePins.value(PIN_DATA)->data[step], totalSize);
	devicePin *vclk = devicePins.value(PIN_VIRTUAL_CLOCK);
	if(!vclk->data.contains(step)) {
		vclk->data.insert(step, createPinData(totalSize));
	}
	vclk->data.value(step).dataArray->fill(true, 0, registerSize);
	vclk->data.value(step).dataMask->fill(true, 0, registerSize);
	devicePin *le = devicePins.value(PIN_LE);
	if(!le->data.contains(step))
		le->data.insert(step, createPinData(totalSize));
	le->data.value(step).dataArray->setBit(registerSize);
	le->data.value(step).dataMask->setBit(registerSize);
	le->data.value(step).dataArray->clearBit(registerSize + 1);
	le->data.value(step).dataMask->setBit(registerSize + 1);
	return true;
}
