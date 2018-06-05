/*
 * \brief  Component to control the runtime of the sculpt systerm
 *         interface definition for data exchange with ADA parts of
 *         the program.
 * \author Pirmin duss
 * \date   2018-05-28
 */


struct cpp_state_t {
	bool power_off_enabled;
};

extern cpp_state_t cpp_state;

/*
 * ADA functions/procedures
 */
extern "C" void ada_create_xml(void);
