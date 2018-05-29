/*
 * \brief  Component to control the runtime of the sculpt systerm
 *         functions that will be replaced by ADA functions when ADA
 *         XML parsing is available.
 * \author Pirmin duss
 * \date   2018-05-28
 */

/* Genode includes */
#include <base/log.h>

#include "xml_parsing.h"

void Power_manager::parse_acpi_ac_report(const Genode::Xml_node& node) {

	Genode::log("AC ============================================");
	Genode::log(node);
	Genode::log("===============================================");
}

void Power_manager::parse_acpi_battery_report(const Genode::Xml_node& node) {

	Genode::log("BATTERY =======================================");
	Genode::log(node);
	Genode::log("===============================================");
}

void Power_manager::parse_acpi_ec_report(const Genode::Xml_node& node) {

	Genode::log("EC ============================================");
	Genode::log(node);
	Genode::log("===============================================");
}

void Power_manager::parse_acpi_lid_report(const Genode::Xml_node& node) {

	Genode::log("LID ===========================================");
	Genode::log(node);
	Genode::log("===============================================");
}

