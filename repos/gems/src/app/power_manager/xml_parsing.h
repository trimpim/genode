/*
 * \brief  Component to control the runtime of the sculpt systerm
 *         functions that will be replaced by ADA functions when ADA
 *         XML parsing is available.
 * \author Pirmin duss
 * \date   2018-05-28
 */

#pragma once

/* Genode includes */
#include <util/xml_node.h>

namespace Power_manager {

	/*
	 * parse the ACPI AC report.
	 *
	 * @param[in]: node    xml from the report.
	 */
	void parse_acpi_ac_report(const Genode::Xml_node& node);

	/*
	 * parse the ACPI battery report.
	 *
	 * @param[in]: node    xml from the report.
	 */
	void parse_acpi_battery_report(const Genode::Xml_node& node);

	/*
	 * parse the ACPI EC report.
	 *
	 * @param[in]: node    xml from the report.
	 */
	void parse_acpi_ec_report(const Genode::Xml_node& node);

	/*
	 * parse the ACPI lid report.
	 *
	 * @param[in]: node    xml from the report.
	 */
	void parse_acpi_lid_report(const Genode::Xml_node& node);

};	// namespace Power_manager
