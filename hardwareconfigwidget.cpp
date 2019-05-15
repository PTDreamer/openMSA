/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      hardwareconfigwidget.cpp file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hardwareConfigWidget
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
#include "hardwareconfigwidget.h"
#include "ui_hardwareconfigwidget.h"
#include "hardware/controllers/interface.h"
#include "hardware/hardwaredevice.h"
#include <QDir>

hardwareConfigWidget::hardwareConfigWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::hardwareConfigWidget), pathwiz(nullptr)
{
	ui->setupUi(this);
}

hardwareConfigWidget::~hardwareConfigWidget()
{
	if(getSaveSettingsOnExit()) {
		settings->setValue("app/lastValues/scanType", config.scanType);
		settings->setValue("app/lastValues/adcAveraging", config.adcAveraging);
		settings->setValue("app/lastValues/TGoffset", config.gui.TGoffset);
		settings->setValue("app/lastValues/TGreversed", config.gui.TGreversed);
		settings->setValue("app/lastValues/SGout", config.gui.SGout);
		settings->setValue("app/lastValues/SGout_multi", config.gui.SGout_multi);

		settings->setValue("app/lastValues/stop_multi", config.gui.stop_multi);
		settings->setValue("app/lastValues/start_multi", config.gui.start_multi);
		settings->setValue("app/lastValues/step_freq_multi", config.gui.step_freq_multi);
		settings->setValue("app/lastValues/center_freq_multi", config.gui.center_freq_multi);
		settings->setValue("app/lastValues/span_freq_multi", config.gui.span_freq_multi);
		settings->setValue("app/lastValues/band", config.gui.band);
		settings->setValue("app/lastValues/stop", config.gui.stop);
		settings->setValue("app/lastValues/start", config.gui.start);

		settings->setValue("app/lastValues/isInvertedScan", config.gui.isInvertedScan);

		settings->setValue("app/lastValues/steps_number", config.gui.steps_number);

		settings->setValue("app/lastValues/stepModeAuto", 	config.gui.stepModeAuto);
		settings->setValue("app/lastValues/isStepInSteps", config.gui.isStepInSteps);

		settings->setValue("app/lastValues/TGoffset_multi", config.gui.TGoffset_multi);
		settings->sync();
	}
	delete ui;
}

msa::scanConfig hardwareConfigWidget::getConfig() const
{
	return config;
}

void hardwareConfigWidget::setConfig(const msa::scanConfig &value)
{
	config = value;
}

hardwareConfigWidget::appSettings_t hardwareConfigWidget::getAppSettings() const
{
	return appSettings;
}

void hardwareConfigWidget::setAppSettings(const appSettings_t &value)
{
	appSettings = value;
}

bool hardwareConfigWidget::getSaveSettingsOnExit() const
{
	return saveSettingsOnExit;
}

void hardwareConfigWidget::setSaveSettingsOnExit(bool value)
{
	saveSettingsOnExit = value;
}

void hardwareConfigWidget::loadSavedSettings(bool loadDefaults)
{
	settings = new QSettings("JBTech", "OpenMSA", this);
	if(loadDefaults)
		settings->clear();
	setSaveSettingsOnExit(settings->value("app/saveSettingsOnExit", true).toBool());

	appSettings.serverPort = quint16(settings->value("app/serverPort", 1234).toUInt());
	appSettings.debugLevel = settings->value("app/debugLevel", 0).toInt();
	appSettings.currentInterfaceType = interface::interface_types(settings->value("app/connectionType", interface::SIMULATOR).toUInt());
	appSettings.devices.insert(msa::PLL1, hardwareDevice::HWdevice(settings->value("msa/hardwareTypes/PLL1", static_cast <int>(hardwareDevice::LMX2326)).toInt()));
	appSettings.devices.insert(msa::PLL2, hardwareDevice::HWdevice(settings->value("msa/hardwareTypes/PLL2", static_cast <int>(hardwareDevice::LMX2326)).toInt()));
	appSettings.devices.insert(msa::PLL3, hardwareDevice::HWdevice(settings->value("msa/hardwareTypes/PLL3", static_cast <int>(hardwareDevice::LMX2326)).toInt()));
	appSettings.devices.insert(msa::DDS1, hardwareDevice::HWdevice(settings->value("msa/hardwareTypes/DDS1", static_cast <int>(hardwareDevice::AD9850)).toInt()));
	appSettings.devices.insert(msa::DDS3, hardwareDevice::HWdevice(settings->value("msa/hardwareTypes/DDS3", static_cast <int>(hardwareDevice::AD9850)).toInt()));
	appSettings.devices.insert(msa::ADC_MAG, hardwareDevice::HWdevice(settings->value("msa/hardwareTypes/ADC_MAG", static_cast <int>(hardwareDevice::AD7685)).toInt()));
	appSettings.devices.insert(msa::ADC_PH, hardwareDevice::HWdevice(settings->value("msa/hardwareTypes/ADC_PH", static_cast <int>(hardwareDevice::AD7685)).toInt()));
	appSettings.readWriteDelay = settings->value("msa/hardwareConfig/writeReadDelay_us", 1000).toUInt();

	config.PDMInversion_degrees = (settings->value("msa/hardwareConfig/PDMInversion_degrees", 180).toDouble());
	config.PDMMaxOut = (settings->value("msa/hardwareConfig/PDMMaxOut", 65535).toUInt());

	config.LO2 = (settings->value("msa/hardwareConfig/LO2", 1024).toDouble());
	config.appxdds1 = (settings->value("msa/hardwareConfig/appxdds1", 10.7).toDouble());
	config.appxdds3 = (settings->value("msa/hardwareConfig/appxdds3", 10.7).toDouble());
	config.baseFrequency = (settings->value("msa/hardwareConfig/baseFrequency", 0).toDouble());
	config.PLL1phasefreq = (settings->value("msa/hardwareConfig/PLL1phasefreq", 0.974).toDouble());
	config.PLL2phasefreq = (settings->value("msa/hardwareConfig/PLL2phasefreq", 4).toDouble());
	config.PLL3phasefreq = (settings->value("msa/hardwareConfig/PLL3phasefreq", 0.974).toDouble());
	config.masterOscilatorFrequency = (settings->value("msa/hardwareConfig/masterOscilatorFrequency", 64).toDouble());
	config.dds1Filterbandwidth = (settings->value("msa/hardwareConfig/dds1Filterbandwidth", 0.015).toDouble());
	config.dds3Filterbandwidth = (settings->value("msa/hardwareConfig/dds3Filterbandwidth", 0.015).toDouble());
	config.PLL1phasepolarity_inverted = settings->value("msa/hardwareConfig/PLL1phasepolarity_inverted", true).toBool();
	config.PLL2phasepolarity_inverted = settings->value("msa/hardwareConfig/PLL2phasepolarity_inverted", false).toBool();
	config.PLL3phasepolarity_inverted = settings->value("msa/hardwareConfig/PLL3phasepolarity_inverted", true).toBool();
	config.PLL1pin14Output = settings->value("msa/hardwareConfig/PLL1pin14Output", 0).toUInt();
	config.PLL3pin14Output = settings->value("msa/hardwareConfig/PLL3pin14Output", 0).toUInt();
	config.currentFinalFilterName = (settings->value("msa/hardwareConfig/finalFilterName", "DUMMY").toString());
	config.currentVideoFilterName = (settings->value("msa/hardwareConfig/currentVideoFilterName", "").toString());

	config.scanType = ComProtocol::scanType_t(settings->value("app/lastValues/scanType", ComProtocol::SA_SG).toInt());
	config.adcAveraging = uint8_t (settings->value("app/lastValues/adcAveraging", 2).toUInt());
	config.gui.TGoffset = settings->value("app/lastValues/TGoffset", 0).toDouble();
	config.gui.TGreversed = settings->value("app/lastValues/TGreversed", false).toBool();
	config.gui.SGout = settings->value("app/lastValues/SGout", 10).toDouble();
	config.gui.SGout_multi = settings->value("app/lastValues/SGout_multi", 1000000).toUInt();

	config.gui.stop_multi = settings->value("app/lastValues/stop_multi", 1000000).toUInt();
	config.gui.start_multi = settings->value("app/lastValues/start_multi", 1000000).toUInt();
	config.gui.step_freq_multi = settings->value("app/lastValues/step_freq_multi", 1000000).toUInt();
	config.gui.center_freq_multi = settings->value("app/lastValues/center_freq_multi", 1000000).toUInt();
	config.gui.span_freq_multi = settings->value("app/lastValues/span_freq_multi", 1000000).toUInt();
	config.gui.band = settings->value("app/lastValues/band", -1).toInt();
	config.gui.stop = settings->value("app/lastValues/stop", 0.075).toDouble();
	config.gui.start = settings->value("app/lastValues/start", -0.075).toDouble();

	config.gui.isInvertedScan = settings->value("app/lastValues/isInvertedScan", false).toBool();

	config.gui.scanType = ComProtocol::scanType_t(config.scanType);
	config.gui.steps_number = settings->value("app/lastValues/steps_number", 400).toUInt();


	config.gui.step_freq = (config.gui.stop - config.gui.start) / config.gui.steps_number;
	config.gui.center_freq = config.gui.start + ((config.gui.stop - config.gui.start) / 2);

	config.gui.stepModeAuto = settings->value("app/lastValues/stepModeAuto", false).toBool();
	config.gui.isStepInSteps = settings->value("app/lastValues/isStepInSteps", true).toBool();

	config.gui.span_freq = (config.gui.stop - config.gui.start);
	config.gui.TGoffset_multi = settings->value("app/lastValues/TGoffset_multi", 1000000).toUInt();

	config.forcedDDS1.isForced = false;
	config.forcedDDS3.isForced = false;

	config.cavityTestRunning = false;
//	int size = settings->beginReadArray("msa/hardwareConfig/resolutionFilters");
//	for (int i = 0; i < size; ++i) {
//		  settings->setArrayIndex(i);
//		  msa::resolutionFilter_t f;
//		  f.centerFrequency = settings->value("centerFrequency").toDouble();
//		  f.bandwidth = settings->value("bandwidth").toDouble();
//		  f.address = settings->value("address").toInt();

//		  config.resolutionFilters.insert(settings->value("name").toString(), f);
//	  }
//	settings->endArray();

	int size = settings->beginReadArray("msa/hardwareConfig/videoFilters");
	for (int i = 0; i < size; ++i) {
		  settings->setArrayIndex(i);
		  msa::videoFilter_t v;
		  v.value = settings->value("value").toDouble();
		  v.address = settings->value("address").toInt();

		  config.videoFilters.insert(settings->value("name").toString(), v);
	  }
	settings->endArray();
	if(config.currentVideoFilterName.isEmpty() && config.videoFilters.keys().length() > 0)
		config.currentVideoFilterName = config.videoFilters.keys().first();

	loadCalibrationFiles();
}

void hardwareConfigWidget::saveSettings()
{
	settings->setValue("app/saveSettingsOnExit", getSaveSettingsOnExit());

	settings->setValue("app/serverPort", appSettings.serverPort);
	settings->setValue("app/debugLevel", appSettings.debugLevel);
	settings->setValue("app/connectionType", appSettings.currentInterfaceType);
	settings->setValue("msa/hardwareTypes/PLL1", appSettings.devices.value(msa::PLL1));
	settings->setValue("msa/hardwareTypes/PLL2", appSettings.devices.value(msa::PLL2));
	settings->setValue("msa/hardwareTypes/PLL3", appSettings.devices.value(msa::PLL3));
	settings->setValue("msa/hardwareTypes/DDS1", appSettings.devices.value(msa::DDS1));
	settings->setValue("msa/hardwareTypes/DDS3", appSettings.devices.value(msa::DDS3));
	settings->setValue("msa/hardwareTypes/ADC_MAG", appSettings.devices.value(msa::ADC_MAG));
	settings->setValue("msa/hardwareTypes/ADC_PH", appSettings.devices.value(msa::ADC_PH));

	settings->setValue("msa/hardwareConfig/writeReadDelay_us", appSettings.readWriteDelay);

	settings->setValue("msa/hardwareConfig/PDMInversion_degrees", config.PDMInversion_degrees);
	settings->setValue("msa/hardwareConfig/PDMMaxOut", config.PDMMaxOut);

	settings->setValue("msa/hardwareConfig/LO2", config.LO2);
	settings->setValue("msa/hardwareConfig/appxdds1", config.appxdds1);
	settings->setValue("msa/hardwareConfig/appxdds3", config.appxdds3);
	settings->setValue("msa/hardwareConfig/baseFrequency", config.baseFrequency);
	settings->setValue("msa/hardwareConfig/PLL1phasefreq", config.PLL1phasefreq);
	settings->setValue("msa/hardwareConfig/PLL2phasefreq", config.PLL2phasefreq);
	settings->setValue("msa/hardwareConfig/PLL3phasefreq", config.PLL3phasefreq);
	settings->setValue("msa/hardwareConfig/masterOscilatorFrequency", config.masterOscilatorFrequency);
	settings->setValue("msa/hardwareConfig/dds1Filterbandwidth", config.dds1Filterbandwidth);
	settings->setValue("msa/hardwareConfig/dds3Filterbandwidth", config.dds3Filterbandwidth);
	settings->setValue("msa/hardwareConfig/PLL1phasepolarity_inverted", config.PLL1phasepolarity_inverted);
	settings->setValue("msa/hardwareConfig/PLL2phasepolarity_inverted", config.PLL2phasepolarity_inverted);
	settings->setValue("msa/hardwareConfig/PLL3phasepolarity_inverted", config.PLL3phasepolarity_inverted);
	settings->setValue("msa/hardwareConfig/PLL1pin14Output", config.PLL1pin14Output);
	settings->setValue("msa/hardwareConfig/PLL3pin14Output", config.PLL3pin14Output);
	settings->setValue("msa/hardwareConfig/finalFilterName", config.currentFinalFilterName);
	settings->setValue("msa/hardwareConfig/currentVideoFilterName", config.currentVideoFilterName);



//	settings->beginWriteArray("msa/hardwareConfig/resolutionFilters");
//	int x = 0;
//	foreach(QString name, config.resolutionFilters.keys()) {
//		  settings->setArrayIndex(x);
//		  ++x;
//		  settings->setValue("centerFrequency", config.resolutionFilters.value(name).centerFrequency);
//		  settings->setValue("bandwidth", config.resolutionFilters.value(name).bandwidth);
//		  settings->setValue("address", config.resolutionFilters.value(name).address);
//		  settings->setValue("name", name);
//	}
//	  settings->endArray();
	  settings->beginWriteArray("msa/hardwareConfig/videoFilters");
	  int x = 0;
	  foreach(QString name, config.videoFilters.keys()) {
			settings->setArrayIndex(x);
			++x;
			settings->setValue("value", config.videoFilters.value(name).value);
			settings->setValue("address", config.videoFilters.value(name).address);
			settings->setValue("name", name);
	  }
		settings->endArray();

		settings->sync();
}

void hardwareConfigWidget::setSettingsFromGui()
{
	setSaveSettingsOnExit(ui->save_settings_on_exit->isChecked());
	appSettings.serverPort = quint16(ui->server_port->value());
	appSettings.debugLevel = ui->debug_level->currentIndex();
	appSettings.currentInterfaceType = interface::interface_types(ui->hw_interface->currentIndex());
	switch (ui->pll1_type->currentIndex()) {
	case 0:
		appSettings.devices.insert(msa::PLL1, hardwareDevice::LMX2326);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (ui->pll2_type->currentIndex()) {
	case 0:
		appSettings.devices.insert(msa::PLL2, hardwareDevice::LMX2326);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (ui->pll3_type->currentIndex()) {
	case 0:
		appSettings.devices.insert(msa::PLL3, hardwareDevice::LMX2326);
		break;
	case 1:
		appSettings.devices.insert(msa::PLL3, hardwareDevice::LMX2326);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (ui->dds3_type->currentIndex()) {
	case 0:
		appSettings.devices.insert(msa::DDS3, hardwareDevice::AD9850);
		break;
	case 1:
		appSettings.devices.insert(msa::DDS3, hardwareDevice::AD9850);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (ui->dds1_type->currentIndex()) {
	case 0:
		appSettings.devices.insert(msa::DDS1, hardwareDevice::AD9850);
		break;
	case 1:
		appSettings.devices.insert(msa::DDS1, hardwareDevice::AD9850);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (ui->adc_mag_type->currentIndex()) {
	case 0:
		appSettings.devices.insert(msa::ADC_MAG, hardwareDevice::AD7685);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (ui->adc_phase_type->currentIndex()) {
	case 0:
		appSettings.devices.insert(msa::ADC_PH, hardwareDevice::AD7685);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	appSettings.readWriteDelay = uint(ui->read_write_delay->value());
	config.LO2 = ui->pll2_plo2_frequency->value();

	config.appxdds1 = ui->dds1_frequency->value();
	config.appxdds3 = ui->dds3_frequency->value();

	config.baseFrequency = ui->base_frequency->value();
	config.PLL1phasefreq = ui->pll1_frequency->value();
	config.PLL2phasefreq = ui->pll2_frequency->value();
	config.PLL3phasefreq = ui->pll3_frequency->value();

	config.masterOscilatorFrequency = ui->master_osc_frequency->value();
	config.dds1Filterbandwidth = ui->dds1_bandwidth->value();
	config.dds3Filterbandwidth = ui->dds3_bandwidth->value();

	config.PLL1phasepolarity_inverted = ui->pll1_inverted->isChecked();
	config.PLL2phasepolarity_inverted = ui->pll2_inverted->isChecked();
	config.PLL3phasepolarity_inverted = ui->pll3_inverted->isChecked();

	switch (uint(ui->cbPLL1pin14->currentIndex())) {
	case 0:
		config.PLL1pin14Output = 0;
		break;
	case 1:
		config.PLL1pin14Output = 4;
		break;
	case 2:
		config.PLL1pin14Output = 2;
		break;
	case 3:
		config.PLL1pin14Output = 6;
		break;
	case 4:
		config.PLL1pin14Output = 1;
		break;
	case 5:
		config.PLL1pin14Output = 5;
		break;
	case 6:
		config.PLL1pin14Output = 3;
		break;
	case 7:
		config.PLL1pin14Output = 7;
		break;
	}
	switch (uint(ui->cbPLL3pin14->currentIndex())) {
	case 0:
		config.PLL3pin14Output = 0;
		break;
	case 1:
		config.PLL3pin14Output = 4;
		break;
	case 2:
		config.PLL3pin14Output = 2;
		break;
	case 3:
		config.PLL3pin14Output = 6;
		break;
	case 4:
		config.PLL3pin14Output = 1;
		break;
	case 5:
		config.PLL3pin14Output = 5;
		break;
	case 6:
		config.PLL3pin14Output = 3;
		break;
	case 7:
		config.PLL3pin14Output = 7;
		break;
	}
	config.PDMInversion_degrees = ui->pdm_inversion_degrees->value();
	config.PDMMaxOut = quint32(ui->pdm_max_out->value());

	//config.resolutionFilters.clear();
	config.videoFilters.clear();
//TODO
//	for(int x = 0; x < ui->resolution_filters_table->rowCount(); ++x) {
//		msa::resolutionFilter_t r;

//		QString name = ui->resolution_filters_table->item(x, 0)->text();
//		r.centerFrequency = ui->resolution_filters_table->item(x, 1)->text().toDouble();
//		r.bandwidth = ui->resolution_filters_table->item(x, 2)->text().toDouble();
//		r.address = ui->resolution_filters_table->item(x, 3)->text().toInt();
//		config.resolutionFilters.insert(name, r);
//	}

	for(int x = 0; x < ui->video_filters_table->rowCount(); ++x) {
		msa::videoFilter_t v;

		QString name = ui->video_filters_table->item(x, 0)->text();
		v.value = ui->video_filters_table->item(x, 1)->text().toDouble();
		v.address = ui->video_filters_table->item(x, 2)->text().toInt();
		config.videoFilters.insert(name, v);
	}
	emit requiresHwReinit();
}

void hardwareConfigWidget::loadSettingsToGui()
{
	ui->save_settings_on_exit->setChecked(getSaveSettingsOnExit());
	ui->server_port->setValue(appSettings.serverPort);
	ui->debug_level->setCurrentIndex(appSettings.debugLevel);
	ui->hw_interface->setCurrentIndex(appSettings.currentInterfaceType);
	switch (appSettings.devices.value(msa::PLL1)) {
	case hardwareDevice::LMX2326:
		ui->pll1_type->setCurrentIndex(0);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (appSettings.devices.value(msa::PLL2)) {
	case hardwareDevice::LMX2326:
		ui->pll2_type->setCurrentIndex(0);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (appSettings.devices.value(msa::PLL3)) {
	case hardwareDevice::LMX2326:
		ui->pll2_type->setCurrentIndex(0);
		break;
	case hardwareDevice::NONE:
		ui->pll1_type->setCurrentIndex(1);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (appSettings.devices.value(msa::DDS3)) {
	case hardwareDevice::AD9850:
		ui->dds3_type->setCurrentIndex(0);
		break;
	case hardwareDevice::NONE:
		ui->dds3_type->setCurrentIndex(1);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (appSettings.devices.value(msa::DDS1)) {
	case hardwareDevice::AD9850:
		ui->dds1_type->setCurrentIndex(0);
		break;
	case hardwareDevice::NONE:
		ui->dds1_type->setCurrentIndex(1);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (appSettings.devices.value(msa::ADC_MAG)) {
	case hardwareDevice::AD7685:
		ui->adc_mag_type->setCurrentIndex(0);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (appSettings.devices.value(msa::ADC_PH)) {
	case hardwareDevice::AD7685:
		ui->adc_phase_type->setCurrentIndex(0);
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	ui->read_write_delay->setValue(int(appSettings.readWriteDelay));
	ui->pll2_plo2_frequency->setValue(config.LO2);

	ui->dds1_frequency->setValue(config.appxdds1);
	ui->dds3_frequency->setValue(config.appxdds3);

	ui->base_frequency->setValue(config.baseFrequency);
	ui->pll1_frequency->setValue(config.PLL1phasefreq);
	ui->pll2_frequency->setValue(config.PLL2phasefreq);
	ui->pll3_frequency->setValue(config.PLL3phasefreq);

	ui->master_osc_frequency->setValue(config.masterOscilatorFrequency);

	ui->dds1_bandwidth->setValue(config.dds1Filterbandwidth);
	ui->dds3_bandwidth->setValue(config.dds3Filterbandwidth);

	ui->pll1_inverted->setChecked(config.PLL1phasepolarity_inverted);
	ui->pll2_inverted->setChecked(config.PLL2phasepolarity_inverted);
	ui->pll3_inverted->setChecked(config.PLL3phasepolarity_inverted);

	ui->pdm_inversion_degrees->setValue(config.PDMInversion_degrees);
	ui->pdm_max_out->setValue(int(config.PDMMaxOut));

	foreach(QString name, config.videoFilters.keys()) {
		ui->video_filters_table->insertRow(ui->video_filters_table->rowCount());
		ui->video_filters_table->setItem(ui->video_filters_table->rowCount()-1, 0,  new QTableWidgetItem(name));
		ui->video_filters_table->setItem(ui->video_filters_table->rowCount()-1, 1,  new QTableWidgetItem(QString::number((config.videoFilters.value(name).value))));
		ui->video_filters_table->setItem(ui->video_filters_table->rowCount()-1, 2,  new QTableWidgetItem(QString::number((config.videoFilters.value(name).address))));
	}


	foreach(calParser::magPhaseCalData d, config.pathCalibrationList) {
		ui->resolution_filters_table->insertRow(ui->resolution_filters_table->rowCount());
		ui->resolution_filters_table->setItem(ui->resolution_filters_table->rowCount()-1, 0,  new QTableWidgetItem(d.pathName));
		ui->resolution_filters_table->setItem(ui->resolution_filters_table->rowCount()-1, 1,  new QTableWidgetItem(QString::number(d.centerFreq_MHZ)));
		ui->resolution_filters_table->setItem(ui->resolution_filters_table->rowCount()-1, 2,  new QTableWidgetItem(QString::number(d.bandwidth_MHZ)));
		ui->resolution_filters_table->setItem(ui->resolution_filters_table->rowCount()-1, 3,  new QTableWidgetItem(QString::number(d.controlPin)));
		ui->resolution_filters_table->setItem(ui->resolution_filters_table->rowCount()-1, 4,  new QTableWidgetItem(QString::number(d.calFrequency)));
		ui->resolution_filters_table->setItem(ui->resolution_filters_table->rowCount()-1, 5,  new QTableWidgetItem(d.calDate));
	}

	ui->resolution_filters_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->video_filters_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	switch (config.PLL1pin14Output) {
	case 0:
		ui->cbPLL1pin14->setCurrentIndex(0);
		break;
	case 1:
		ui->cbPLL1pin14->setCurrentIndex(4);
		break;
	case 2:
		ui->cbPLL1pin14->setCurrentIndex(2);
		break;
	case 3:
		ui->cbPLL1pin14->setCurrentIndex(6);
		break;
	case 4:
		ui->cbPLL1pin14->setCurrentIndex(1);
		break;
	case 5:
		ui->cbPLL1pin14->setCurrentIndex(5);
		break;
	case 6:
		ui->cbPLL1pin14->setCurrentIndex(3);
		break;
	case 7:
		ui->cbPLL1pin14->setCurrentIndex(7);
		break;
	}

	switch (config.PLL3pin14Output) {
	case 0:
		ui->cbPLL3pin14->setCurrentIndex(0);
		break;
	case 1:
		ui->cbPLL3pin14->setCurrentIndex(4);
		break;
	case 2:
		ui->cbPLL3pin14->setCurrentIndex(2);
		break;
	case 3:
		ui->cbPLL3pin14->setCurrentIndex(6);
		break;
	case 4:
		ui->cbPLL3pin14->setCurrentIndex(1);
		break;
	case 5:
		ui->cbPLL3pin14->setCurrentIndex(5);
		break;
	case 6:
		ui->cbPLL3pin14->setCurrentIndex(3);
		break;
	case 7:
		ui->cbPLL3pin14->setCurrentIndex(7);
		break;
	}
	ui->resolution_filters_table->setCurrentCell(0,0);
}

void hardwareConfigWidget::on_pb_add_video_filter_clicked()
{
	ui->video_filters_table->insertRow(ui->video_filters_table->rowCount());
}

void hardwareConfigWidget::on_pb_delete_video_filter_clicked()
{
	ui->video_filters_table->removeRow(ui->video_filters_table->currentRow());
}

void hardwareConfigWidget::on_pb_add_resolution_filter_clicked()
{
	pathwiz = new pathCalibrationWiz("", 10.7, 0, calParser::magPhaseCalData());
	pathwiz->show();
}

void hardwareConfigWidget::on_pb_delete_resolution_filter_clicked()
{
	ui->resolution_filters_table->removeRow(ui->resolution_filters_table->currentRow());
}

void hardwareConfigWidget::on_pb_load_defaults_clicked()
{
	loadSavedSettings(true);
	loadSettingsToGui();
}

void hardwareConfigWidget::on_pb_save_and_exit_clicked()
{
	setSettingsFromGui();
	saveSettings();
	close();
}

void hardwareConfigWidget::on_pb_apply_clicked()
{
	setSettingsFromGui();
}

void hardwareConfigWidget::on_pb_cancel_clicked()
{
	loadSettingsToGui();
	close();
}

void hardwareConfigWidget::loadCalibrationFiles()
{
	bool s;
	QString err;
	config.frequencyCalibration = m_calParser.loadFreqCalDataFromFile( m_calParser.getConfigLocation() + QDir::separator() + STANDARD_FREQ_CAL_FILENAME, s, err);
	if(!s) {
		emit triggerMessage(WARNING, "Could not load Freq cal file, trying to create default", err, 5);
		bool ss = m_calParser.createDefaultFreqCalData();
		if(ss) {
			emit triggerMessage(INFO, "Default freq cal created", "Proceding with loading it", 5);
			config.frequencyCalibration = m_calParser.loadFreqCalDataFromFile("", s, err);
			if(!s) {
				emit triggerMessage(WARNING, "Failed to open recent created file", err, 5);
				config.frequencyCalibration.freqToPower.insert(0, 0);
				config.frequencyCalibration.freqToPower.insert(1000, 0);
			}
		} else {
			emit triggerMessage(WARNING, "Could not create default file", "No idea what went wrong!", 5);
			config.frequencyCalibration.freqToPower.insert(0, 0);
			config.frequencyCalibration.freqToPower.insert(1000, 0);
		}
	}
	bool useDummyMagCal = false;
	config.pathCalibrationList = m_calParser.loadMagPhaseCalDataFromFile(m_calParser.getConfigLocation() + QDir::separator() + STANDARD_PATHS_CAL_FILENAME, s, err);
	if(!s) {
		emit triggerMessage(WARNING, "Could not load Mag Phase cal file, trying to create default", err, 5);
		bool ss = m_calParser.createDefaultMagPhaseCalData();
		if(ss) {
			emit triggerMessage(INFO, "Default Mag Phase cal created", "Proceding with loading it", 5);
			config.pathCalibrationList = m_calParser.loadMagPhaseCalDataFromFile("", s, err);
			if(!s) {
				emit triggerMessage(WARNING, "Failed to open recent created file", err, 5);
				useDummyMagCal = true;
			}
		}
	}
	else {
		emit triggerMessage(WARNING, "Could not create default file", "No idea what went wrong!", 5);
		useDummyMagCal = true;
	}
	if(useDummyMagCal) {
		emit triggerMessage(WARNING, "Mag Phase Cal", "Using dummy values", 5);
		config.pathCalibration.pathName = "DUMMY";
		config.pathCalibration.controlPin = -1;
		config.pathCalibration.bandwidth_MHZ = 0.00015;
		config.pathCalibration.centerFreq_MHZ = 10.7;
		calParser::magCalFactors f;
		f.dbm_val = -120;
		f.phase_val = 0;
		config.pathCalibration.adcToMagCalFactors.insert(0, f);
		f.dbm_val = 0;
		config.pathCalibration.adcToMagCalFactors.insert(32767, f);
	}
	if(config.pathCalibrationList.length() == 1) {
		config.currentFinalFilterName = config.pathCalibrationList.first().pathName;
		config.pathCalibration = config.pathCalibrationList.first();
	}
	else if (config.pathCalibrationList.length() > 1) {
		foreach(calParser::magPhaseCalData p, config.pathCalibrationList) {
			if(p.pathName == config.currentFinalFilterName) {
				config.pathCalibration = p;
				break;
			}
		}
	}
}

void hardwareConfigWidget::on_pb_CommandDDS1_clicked()
{
	config.forcedDDS1.isForced = true;
	config.forcedDDS1.outputFreq = ui->dsDDS1Output->value();
	config.forcedDDS1.oscFreq = ui->dsDDSClock->value();
	emit requiresReinit();
}

void hardwareConfigWidget::on_pbResetDDS1_clicked()
{
	config.forcedDDS1.isForced = false;
	emit requiresReinit();
}

void hardwareConfigWidget::on_pb_CommandDDS3_clicked()
{
	config.forcedDDS3.isForced = true;
	config.forcedDDS3.outputFreq = ui->dsDDS3Output->value();
	config.forcedDDS3.oscFreq = ui->dsDDSClock->value();
	emit requiresReinit();
}

void hardwareConfigWidget::on_pbResetDDS3_clicked()
{
	config.forcedDDS3.isForced = false;
	emit requiresReinit();
}

void hardwareConfigWidget::on_resolution_filters_table_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
	Q_UNUSED(currentColumn);
	Q_UNUSED(previousRow);
	Q_UNUSED(previousColumn);
	ui->path_calibration_table->clearContents();
	QString path = ui->resolution_filters_table->itemAt(0, currentRow)->text();
	foreach (calParser::magPhaseCalData data, config.pathCalibrationList) {
		if(data.pathName == path)
		{
			pathCalCurrentData = data;
			QList<uint> keys = data.adcToMagCalFactors.keys();
			std::sort(keys.begin(), keys.end());
			int count = 0;
			foreach(uint key, keys) {
				ui->path_calibration_table->insertRow(count);
				ui->path_calibration_table->setItem(count, 0,  new QTableWidgetItem(QString::number(key)));
				ui->path_calibration_table->setItem(count, 1,  new QTableWidgetItem(QString::number(data.adcToMagCalFactors.value(key).dbm_val)));
				ui->path_calibration_table->setItem(count, 2,  new QTableWidgetItem(QString::number(data.adcToMagCalFactors.value(key).phase_val)));
				++count;
			}
		}
	}
}

void hardwareConfigWidget::on_pb_edit_resolution_filter_clicked()
{
	QString name = ui->resolution_filters_table->item(ui->resolution_filters_table->currentRow(), 0)->text();
	QString cfreq = ui->resolution_filters_table->item(ui->resolution_filters_table->currentRow(), 1)->text();
	QString bw = ui->resolution_filters_table->item(ui->resolution_filters_table->currentRow(), 2)->text();
	qDebug() << name << cfreq << bw;
	pathwiz = new pathCalibrationWiz(name, cfreq.toDouble(), bw.toDouble(), pathCalCurrentData);
	pathwiz->show();
}
