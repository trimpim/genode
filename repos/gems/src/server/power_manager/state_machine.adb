with Ada.Text_IO;
use Ada.Text_IO;

package body State_machine
	with SPARK_Mode
is

	function HandleEvent(event : Event_Type) return Machine_State_Type
	is
	begin
		case event is
			when AC_Connected =>
				put("ac connected");
			when AC_Disconnected =>
				put("ac disconnected");
		end case;

		return Invalid;
	end;

end State_machine;
