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

hardwareConfigWidget::hardwareConfigWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::hardwareConfigWidget)
{
	ui->setupUi(this);
}

hardwareConfigWidget::~hardwareConfigWidget()
{
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

void hardwareConfigWidget::loadSavedSettings(bool loadDefaults)
{
	settings = new QSettings("JBTech", "OpenMSA", this);
	if(loadDefaults)
		settings->clear();
	appSettings.serverPort = quint16(settings->value("app/serverPort", 1234).toUInt());
	appSettings.debugLevel = settings->value("app/debugLevel", 0).toInt();
	appSettings.currentInterfaceType = interface::interface_types(settings->value("app/connectionType", interface::SIMULATOR).toUInt());
	appSettings.devices.insert(msa::PLL1, hardwareDevice::HWdevice(settings->value("msa/hardwareTypes/PLL1", static_cast <int>(hardwareDevice::LMX2326)).toInt()));
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
	config.currentFinalFilterName = (settings->value("msa/hardwareConfig/finalFilterName", "DUMMY").toString());

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
}

void hardwareConfigWidget::loadSettingsToGui()
{
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
	ui->resolution_filters_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->video_filters_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
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
	ui->resolution_filters_table->insertRow(ui->resolution_filters_table->rowCount());
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
