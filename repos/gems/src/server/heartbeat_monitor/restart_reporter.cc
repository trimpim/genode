/*
 * \brief  implementation of restart information reporter
 * \author Pirmin Duss
 * \date   2020-04-22
 *
 */


/* local includes */
#include "restart_reporter.h"

/*
 *
 * Report format:
 *
 * <restart_info>
 *     <slice name="vbox5" restarts="3">
 *         <failed label="vbox5 -> vfs" ram_requests="1" skipped_heartbeats="1"/>
 *         <failed label="vbox5 -> init -> log_terminal" caps_requests="5"/>
 *     </slice>
 *     <slice name="fs"/>
 * </restart_info>
 *
 */

void Heartbeat_monitor::Restart_reporter::report_state()
{
	_restart_reporter.generate([this] (Xml_generator &xml) {

		_packages.for_each([&xml] (Slice &slice) {

			xml.node("component", [&slice, &xml] () {

				xml.attribute("name", slice.name());

				slice.update_internal_version();

				if (slice.combined_version() > 0) {
					/* check if the slice contains failed components */
					slice.for_each([&xml] (Failed_component const &failed) {

						xml.node("failed", [&failed, &xml] () {

							xml.attribute("label", failed.label);

							if (failed.caps_count) {
								xml.attribute("caps_requests", failed.caps_count);
							}

							if (failed.cpu_count) {
								xml.attribute("cpu_faults", failed.cpu_count);
							}

							if (failed.heartbeat_count) {
								xml.attribute("skipped_heartbeats", failed.heartbeat_count);
							}

							if (failed.ram_count) {
								xml.attribute("ram_requests", failed.ram_count);
							}

							if (failed.segfault_count) {
								xml.attribute("segfaults", failed.segfault_count);
							}
						});
					});

					xml.attribute("restarts", slice.combined_version());
				}
			});
		});
	});
}
