#ifndef LMX2326_H
#define LMX2326_H
#include <QString>
#include "global_defs.h"
#include <QtEndian>
#include <QHash>
#include <QBitArray>
#include "hardware/hardwaredevice.h"
#include "deviceparser.h"
typedef struct {
	quint64 rcounter;
	quint64 ncounter;
	quint64 latches;
} lmx2326_struct;

class lmx2326: public genericPLL
{
	Q_OBJECT
public:
	lmx2326(msa::MSAdevice device, QObject *parent);

	clockType getClk_type() const;
	void processNewScan();
	bool init();
	void reinit();
	~lmx2326();
	int getRCounter();
	typedef enum {PIN_CLK, PIN_DATA, PIN_LE, PIN_VIRTUAL_CLOCK} pins;
protected:
private:
	void loadRcounterCC(int value);
	lmx2326_struct s;
	// the first letter(s) denotes the register to which the field belongs (ex:r means rcounter)
	typedef enum {
	R_CC, R_LD, R_DIVIDER, R_TESTMODES,
	N_CC, N_ACOUNTER_DIVIDER, N_BCOUNTER_DIVIDER, N_CPGAIN_BIT,
	L_CC, L_COUNTER_RESET, L_POWER_DOWN, L_FO_LD, L_PH_DET_POLARITY, L_CP, L_FASTLOCK, L_TIMEOUT, L_TESTMODES, L_POWER_DOWN_MODE, L_TESTMODE}fields_type;

	enum class control_field {RCOUNTER=0, NCOUNTER=1, FUNCTION_LATCH=2, INIT=3};
	enum class FoLD_field {TRI_STATE=0, R_DIVIDER_OUT=4, N_DIVIDER_OUT=2, SERIAL_DATA_OUT=6, DIGITAL_LOCK_DETECT=1, nCHANNEL_OPEN_DRAIN_LOCK_DETECT=5, ACTIVE_HIGH=3, ACTIVE_LOW=7};
	enum class phase_detector {NON_INVERTED=0, INVERTED=1};
	enum class cp_gain {LOW = 0, HIGH};//250uA, 1ma
	enum class cp_tri_state {NORMAL, TRI_STATE};
	// returns VCO frequency based on the current register values
	double getVcoFrequency(double external_clock_frequency);
	bool addLEandCLK(quint32 step);
	bool checkSettings();
};
#endif // LMX2326_H
