package State_machine
	with SPARK_Mode
is

	type Machine_State_Type is (Invalid,
								Startup,
								Idle,
								LowPower,
								ShuttingDown);

	type Event_Type is (AC_Connected, AC_Disconnected);


	type Event_Handler_Type is access function return Integer;

	Event_Handlers : array (Event_Type'range) of Event_Handler_Type;

	function HandleEvent(event : Event_Type) return Machine_State_Type;

end State_machine;
