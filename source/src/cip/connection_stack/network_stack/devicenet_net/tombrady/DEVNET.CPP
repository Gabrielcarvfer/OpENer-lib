//---------------------------------------------------------------------------------
//		DEVNET.CPP
//    by Jim Brady
//    Last Revised August 1998
//
//		This is the source code for the DeviceNet Weather Station
//		as presented in the Circuit Cellar INK article.  It runs on a
//    Micro/sys SBC1386 CPU board which uses a 386EX running at 25MHz.
//
//		Compiler: Borland C++ Version 3.1
//  	Settings: C++ mode, Large memory model, no optimization
//		Note: The compiler option: "Out-of-line inline functions" determines
//		whether peek() and poke() is expanded inline.  Inline expansion
//		executes much faster.
//
//		The DeviceNet hardware interface uses an Intel 82527 mapped into
//		memory space at 0xA0000.  The DeviceNet MAC ID is read from port 2
// 	of the 82527.  The baud rate is read from port C of an 82C55A I/O
//		chip located on the same board.  The 82C55A is at address 0xA1000.
//
//		The entire program is contained in DEVNET.CPP and DEVNET.H
//
//		This program passed the DeviceNet Protocol Conformance
//		Test Software, version 2.0-1.00.  I make no claims beyond that.
//		You are welcome to use the source code as an education tool if
//		you find it useful. Please do not use it directly in a commercial
//		product.
//
//		Please feel free to contact me if you have any questions.
//
//		Jim Brady
//		EbaraJim@aol.com
//
//
//---------------------------------------------------------------------------------
/*
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <bios.h>
#include <dos.h>
#include <math.h>
#include <devnet.h>



// Define global variables
UCHAR global_CAN_buf[BUFSIZE];  		// Temporary buffer for incoming CAN messages
UINT global_event;            		// Device Events - see DEVNET.H for bit defines
UINT global_status;                 // Device Status - see DEVNET.H for bit defines
UINT global_timer[10];              // Global timers - see DEVNET.H for bit defines



//---------------------------------------------------------------------------------
// This is the interrupt handler for receipt of incoming CAN messages
// The three dots are required in C++ mode.  It copies the contents of the
// Intel 82527 CAN controller into a temporary global buffer. It also saves
// the message length.  With the 25MHz 386EX, this ISR this takes 100 us with
// peek() & poke() expanded inline, 160 us otherwise.
//---------------------------------------------------------------------------------
void interrupt far can_isr(...)
{
	UCHAR i, int_source, addr, mailbox, length;


	int_source = peekb(CAN_BASE, 0x5F);    // read interrupt register
	if ((int_source < 3) || (int_source > 7)) return;

	mailbox = int_source - 2;
   // clear out first 10 bytes
	for (i=0; i < 10; i++) global_CAN_buf[i] = 0;

	// compute address of config register in msg object of interest
	addr = 6 + (mailbox << 4);             // multiply by 16 and add 6
	length = peekb(CAN_BASE, addr);
	length = length >> 4;
	global_CAN_buf[LENGTH] = length;   		// save length of message
	for (i=0; i < length; i++)      			// move message from 82527
	{
		addr++;
		global_CAN_buf[i] = peekb(CAN_BASE, addr);
	}

	addr = 1 + (mailbox << 4);	  					// point to control 1 register
	pokeb(CAN_BASE, addr, 0x55);					// clear INT_PND bit
	addr--;                    					// point to control 0 register
	pokeb(CAN_BASE, addr, 0xFD);					// clear NEWDAT
	global_event |= 0x0001 << (mailbox - 1);	// set the bit in global_event
	outp(0x20,0x20);                       	// non-specific EOI
}



//---------------------------------------------------------------------------------
// This is the interrupt handler for the timer ticks.  It is chained to
// the BIOS clock, and happens at 20.0 Hz rate.  It is not 18.2 Hz because
// I have adjusted TIMER0 of the 386EX.  See the setup code in main().
// The BIOS clock ISR, which is part of the MicroSys BIOS code, was measured
// at 16 us.  On the 25MHz 386EX, this timer ISR takes about 40 us, depending on
// how many timers are active (non-zero).
//---------------------------------------------------------------------------------
void interrupt far timer(...)
{
	UCHAR can_status;

	// check for CAN errors by reading CAN status register
	can_status = peekb(CAN_BASE, 0x01);
	pokeb(CAN_BASE, 0x01, 0x00);

	if (can_status & 0x80)  global_status |= BUS_OFF;  // stays in bus off condition
	if (can_status == 0x43) global_status |= LONELY_NODE;
	else global_status &= (~LONELY_NODE);

	// If a timer is active (non-zero) it is decremented.  When it has reached
	// zero, a bit in the global event word is set, to be later handled by the
	// RTOS loop running in main(). The bit position determines its priority.
	if (global_timer[EXPLICIT])            // Explicit Connection timer
	{
		global_timer[EXPLICIT]--;           // decrement until zero
		if (global_timer[EXPLICIT] == 0) global_event |= EXPLICIT_TIMEOUT;
	}

	if (global_timer[IO_POLL])             // I/O Poll Connection timer
	{
		global_timer[IO_POLL]--;            // decrement until zero
		if (global_timer[IO_POLL] == 0) global_event |= IO_POLL_TIMEOUT;
	}

	if (global_timer[ACK_WAIT])          	// Acknowlege message timer
	{
		global_timer[ACK_WAIT]--;   	      // decrement until zero
		if (global_timer[ACK_WAIT] == 0) global_event |= ACK_WAIT_TIMEOUT;
	}

	if (global_timer[UPDATE])              // Timer for updating the device
	{
		global_timer[UPDATE]--;   	         // decrement until zero
		if (global_timer[UPDATE] == 0) global_event |= DEVICE_UPDATE;
	}
}



//---------------------------------------------------------------------------------
// Analog Input Point Class, class ID = 0x0A
// It is used for the Temperature and Humidity Sensor Objects.
// I extended it to handle A/D conversion and computation for the
// Weather Station sensors.
//
// Class Attributes supported:      Revision
// Class Services supported: 		 	Get Attribute Single
// Instance Attributes supported:   Value, Status, Data Type
// Instance Services supported:     Get Attribute Single
//---------------------------------------------------------------------------------
class ANALOG_INPUT_POINT
{
	private:
	UCHAR value;
	BOOL status;
	UCHAR data_type;
	UCHAR instance;
	ULONG sensor_clock;
	static UINT class_revision;


	public:
	static void handle_class_inquiry(UCHAR*, UCHAR*);
	void handle_explicit(UCHAR*, UCHAR*);
	void select_mux(UCHAR);
	double read_a2d(void);
	void compute_value(void);
	UCHAR get_value(void);
	ANALOG_INPUT_POINT(UCHAR inst)  // constructor
	{
      instance = inst;
      sensor_clock = 0;
		value = 0;
		status = 0;		        // no alarms or faults
   	data_type = 2;         // indicates that value is an unsigned char
	}
};



// Handle Explicit requests to the class
void ANALOG_INPUT_POINT::handle_class_inquiry(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;

	service = request[1];
	attrib = request[4];
	error = 0;
	memset(response, 0, BUFSIZE);

	switch(service)
	{
		case GET_REQUEST:
		switch(attrib)
		{
			case 1:  // get revision attribute
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(class_revision);
			response[3] = HIBYTE(class_revision);
			response[LENGTH] = 4;
			break;

			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;


		default:
		error = SERVICE_NOT_SUPPORTED;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}




// Handles Explicit request to Analog Input Point Object
void ANALOG_INPUT_POINT::handle_explicit(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;

	service = request[1];
	attrib = request[4];
	error = 0;
	memset(response, 0, BUFSIZE);

	switch(service)
	{
		case GET_REQUEST:
		switch(attrib)
		{
			case 3:  // value
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = value;
			response[LENGTH] = 3;
			break;

			case 4:  // status
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = (UCHAR)status;
			response[LENGTH] = 3;
			break;

			case 8:  // data type
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = data_type;
			response[LENGTH] = 3;
			break;

			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;

		default:
		error = SERVICE_NOT_SUPPORTED;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}



// Returns the sensor's value
UCHAR ANALOG_INPUT_POINT::get_value(void)
{
	return value;
}



// Select input to A/D converter.  The settling time of the A/D converter
// is 0.20 secs, so must wait this long after changing input before A/D
// output is valid
void ANALOG_INPUT_POINT::select_mux(UCHAR channel)
{
	// Note: The MUX only accepts a select input when it's
	// WR line is low.  
	channel = channel << 3;
	channel = channel | 0x02;			// take WR low, keep CS high
	pokeb(PIO_BASE, PORTA, channel);
	channel = channel | 0x42;			// take WR high, keep CS high
	pokeb(PIO_BASE, PORTA, channel);
}




// This function reads the A/D converter and returns actual input voltage.
// The A/D is a Cirrus Logic/Crystal Semicinductor CS5501 delta sigma unit.
// It provides a 16 bit output via a 4 wire serial interface
double ANALOG_INPUT_POINT::read_a2d(void)
{
	
	UINT result;
	UCHAR i, dummy, temp;
	UINT a2d_count;
	double volts;            				// A/D input voltage

	a2d_count = 0;
	dummy = 0;
	pokeb(PIO_BASE, PORTA, 0x40); 		// bring CS low, keep WR high
	dummy++; dummy++;         				// allow CS low > 1.0 us before clocking
	dummy++; dummy++;
	for (i=0; i < 16; i++)
	{
		pokeb(PIO_BASE, PORTA, 0x41); 	// bring clock high, keep WR high
		dummy++;									// short delay
		temp = peekb(PIO_BASE, PORTB);
		if (temp & 0x01) a2d_count |= 0x0001;
		pokeb(PIO_BASE, PORTA, 0x40); 	// bring clock low;
		if (i < 15) a2d_count <<= 1;     // shift left one bit
	}
	pokeb(PIO_BASE, PORTA, 0x42);			// bring CS high (deselect)
	volts = (double)a2d_count;
	volts = (volts * 2.500) / 65536.00;
	// Check limits and alarm if outside limits
	if (volts > 2.40)
	{
		volts = 2.40;
		status = TRUE;
		global_status |=DEVICE_FAULT;  		// generate alarm
	}
	else
	{
		status = FALSE;
		global_status &= (~DEVICE_FAULT); 	// OK
	}
	if (volts < 0.10)
	{
		volts = 0.10;
		status = TRUE;
		global_status |= DEVICE_FAULT;  		// generate alarm
	}
	else
	{
		status = FALSE;
		global_status &= (~DEVICE_FAULT); 	// OK
	}
	return volts;
}




// Reads A/D converter, then depending on which instance it is,computes
// temperature or humidity.  The temperature sensor is a Keystone
// RL1008-5820-97-SP thermistor. This thermistor is 10.00K ohms at 25C
// and is accurate to +/- 0.2C.  The humidity sensor is a General Eastern
// G-CAP2-V-C capacitance type sensor.  It is calibrated to be 148pF at
// 0% RH and is accurate to +/- 3% RH.
// Note: this funciton gets called every 0.25 seconds
void ANALOG_INPUT_POINT::compute_value(void)
{
	double volts;           // A/D input voltage (volts)
	double Rt;					// Thermistor resistance (K ohms)
	double Tc, Tf;				// Temperature, Centigrade & Farenheit
	double Ctotal;          // Total capacitance: sensor + cable (pF)
   double Csensor;			// Capacitance of humidity sensor itself (pF)
	double RH;        		// Relative humidity (percent)

   sensor_clock++;
   
	if ((global_status & OPERATIONAL) == 0) return;

	// go through a sequence through switching the mux and making
	// a measurement.  A complete cycle through takes 4 seconds
	switch (sensor_clock % 16)
	{

		case 0:  // Start by selecting mux temperature input
		if (instance == TEMPERATURE) select_mux(0);
		break;


		case 4:  // After 1 second, read A/D and compute temperature
		if (instance == TEMPERATURE)
		{
			volts = read_a2d();     // get A/D input voltage
			// Compute thermistor resistance.
			Rt = 10.00 * (volts / (2.500 - volts));
			// Use curve-fit equation to compute temperature
			Tc =  -139.163 + 226.222 * pow(Rt, -0.139322);
			// Convert to Farehheit scale
			Tf = 32.00 + 1.80 * Tc;
			// Limit the result to between 0 and 255
			if (Tf < 0.00) Tf = 0.00;
			if (Tf > 255.00) Tf = 255.00;

			// round off and convert to unsigned char
			value = (UCHAR)(Tf + 0.50);
		}
		break;

		case 8:  // After another second, Switch mux to humidity
		if (instance == HUMIDITY) select_mux(1);
		break;

		case 12:  // After another second, read A/D and compute humidity
		if (instance == HUMIDITY)
		{
			volts = read_a2d();     // get A/D input voltage
			// First convert this voltage to total capaciance using
         // a curve-fit equation.
			Ctotal = -49.00 + 109.872 / volts;
			Csensor = Ctotal - 35.00;	 // 35pF cable + stray capacitance
			// Next, convert sensor capacitance to relative humidity
			// Again, use a curve-fit equation
         RH = -494.2 + 3.2845 * Csensor;
         // Limit relative humidity to between 0 and 100
			if (RH < 0.00) RH = 0.00;
			if (RH > 100.00) RH = 100.00;
			// round off and convert to unsigned char
			value = (UCHAR)(RH + 0.50);
		}
	}
}





//---------------------------------------------------------------------------------
// Connection Class, class ID = 5
// It is used for the Explicit and I/O Poll Connection Objects. Since the same
// class is used for both connections, there is some code that checks to see
// which instance it is.
//
// Class Attributes supported:      Revision
// Class Services supported: 		 	Get Attribute Single
// Instance Attributes supported:   State, Instance type, Transport class trigger,
//												Produced conxn id, Consumed conxn id, Initial
//												comm characteristics, Produced conxn size,
//												Consumed conxn size, Expected packet rate,
//												Watchdog timeout action, Produced conxn path
//												length, Produced conxn path, Consumed conxn path
//												length, Consumed conxn path, Production inhibit
//												time.
// Instance Services supported:     Get Attribute Single, Set Attribute Single
//---------------------------------------------------------------------------------
class CONNECTION
{
	private:
	static UINT class_revision;
	// Path for the data produced by the I/O Poll connection.  It points to
	// the assembly object
	static UCHAR path[10];
	UCHAR state, instance, instance_type, transport_class_trigger;
	UINT produced_conxn_id, consumed_conxn_id;
	UCHAR initial_comm_characteristics;
	UINT produced_conxn_size, consumed_conxn_size;
	UINT expected_packet_rate;
	UCHAR watchdog_timeout_action;
	UINT produced_conxn_path_length;
	UCHAR produced_conxn_path[10];
	UINT consumed_conxn_path_length;
	UCHAR consumed_conxn_path[10];
	UINT production_inhibit_time;
	UCHAR rcve_index, xmit_index;
	UCHAR my_rcve_fragment_count, my_xmit_fragment_count;
	UCHAR xmit_fragment_buf[BUFSIZE];
	UCHAR rcve_fragment_buf[BUFSIZE];
	UCHAR ack_timeout_counter;

	public:
	UCHAR get_state(void);
	UCHAR get_timeout_action(void);
	void set_state(UCHAR);

	CONNECTION(UCHAR inst) 	// constructor
	{
		instance = inst;
		if (instance == EXPLICIT) instance_type = 0;
		else instance_type = 1;
		set_state(NON_EXISTENT);         // set initial power-up state
   }

	static void handle_class_inquiry(UCHAR*, UCHAR*);
	void handle_explicit(UCHAR*, UCHAR*);
	int link_consumer(UCHAR*);
	void link_producer(UCHAR*);
};



// Returns the state of a connection
UCHAR CONNECTION::get_state(void)
{
	return state;
}

// Returns the "timeout action" attribute of a connection
UCHAR CONNECTION::get_timeout_action(void)
{
	return watchdog_timeout_action;
}


// Sets the state of a connection
void CONNECTION::set_state(UCHAR st)
{
	state = st;
	if ((instance == EXPLICIT) && (state == ESTABLISHED))
	{
		transport_class_trigger = 0x83;
		produced_conxn_id = 0x5FB;
		consumed_conxn_id = 0x5FC;
		initial_comm_characteristics = 0x21;
		produced_conxn_size = 80;
		consumed_conxn_size = 80;
		expected_packet_rate = 2500;
		watchdog_timeout_action = 0x01;  	// auto delete
		produced_conxn_path_length = 0;
		consumed_conxn_path_length = 0;
		memset(produced_conxn_path, 0, 10);
		memset(produced_conxn_path, 0, 10);
		production_inhibit_time = 0;
		rcve_index = 0;
		xmit_index = 0;
		my_rcve_fragment_count = 0;
		my_xmit_fragment_count = 0;
		memset(rcve_fragment_buf, 0, BUFSIZE);
		memset(xmit_fragment_buf, 0, BUFSIZE);
		ack_timeout_counter = 0;
		global_timer[EXPLICIT] = (expected_packet_rate / 50) * 4;
		global_timer[ACK_WAIT] = 0;
	}
	if ((instance == IO_POLL) && (state == CONFIGURING))
	{
		transport_class_trigger = 0x82;
		produced_conxn_id = 0x3FF;
		consumed_conxn_id = 0x5FD;
		initial_comm_characteristics = 0x01;
		produced_conxn_size = 3;            // i/o poll response length
		consumed_conxn_size = 0;
		expected_packet_rate = 0;
		watchdog_timeout_action = 0x00;  	// go to timed-out
		produced_conxn_path_length = 6;
		consumed_conxn_path_length = 0;
		memcpy(produced_conxn_path, path, 6);
		production_inhibit_time = 0;
		rcve_index = 0;
		xmit_index = 0;
		my_rcve_fragment_count = 0;
		my_xmit_fragment_count = 0;
		memset(rcve_fragment_buf, 0, BUFSIZE);
		memset(xmit_fragment_buf, 0, BUFSIZE);
		ack_timeout_counter = 0;
		global_timer[ACK_WAIT] = 0;
	}

	if (state == NON_EXISTENT)
	{
		global_timer[instance] = 0;   // stop connection timer
	}
}



// Handles Explicit request to the class
void CONNECTION::handle_class_inquiry(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;

	service = request[1];
	attrib = request[4];
	error = 0;
	memset(response, 0, BUFSIZE);

	switch(service)
	{
		case GET_REQUEST:
		switch(attrib)
		{
			case 1:  // get revision attribute
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(class_revision);
			response[3] = HIBYTE(class_revision);
			response[LENGTH] = 4;
			break;

			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;


		default:
		error = SERVICE_NOT_SUPPORTED;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}


// Handles Explicit requests to the connection objects
void CONNECTION::handle_explicit(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;
	UINT test_val, temp;					// for EPR calculation

	service = request[1];
	attrib = request[4];
	error = 0;
	memset(response, 0, BUFSIZE);

	switch(service)
	{
		case GET_REQUEST:
		switch(attrib)
		{
			case 1:   // connection state
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = state;
			response[LENGTH] = 3;
			break;

			case 2:   // instance type
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = instance_type;
			response[LENGTH] = 3;
			break;

			case 3:   // transport class trigger
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = transport_class_trigger;
			response[LENGTH] = 3;
			break;

			case 4:   // produced connection id
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(produced_conxn_id);
			response[3] = HIBYTE(produced_conxn_id);
			response[LENGTH] = 4;
			break;

			case 5:   // consumed connection id
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(consumed_conxn_id);
			response[3] = HIBYTE(consumed_conxn_id);
			response[LENGTH] = 4;
			break;

			case 6:   // initial comm characteristics
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = initial_comm_characteristics;
			response[LENGTH] = 3;
			break;

			case 7:   // produced connection size
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(produced_conxn_size);
			response[3] = HIBYTE(produced_conxn_size);
			response[LENGTH] = 4;
			break;

			case 8: 	// consumed connection size
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(consumed_conxn_size);
			response[3] = HIBYTE(consumed_conxn_size);
			response[LENGTH] = 4;
			break;

			case 9: // EPR
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(expected_packet_rate);
			response[3] = HIBYTE(expected_packet_rate);
			response[LENGTH] = 4;
			break;

			case 12:   // watchdog timeout action
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = watchdog_timeout_action;
			response[LENGTH] = 3;
			break;

			case 13: // produced conxn path length
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(produced_conxn_path_length);
			response[3] = HIBYTE(produced_conxn_path_length);
			response[LENGTH] = 4;
			break;

			case 14:   // produced conxn path
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			if (instance_type == EXPLICIT_TYPE)
			{
				// Path is NULL for Explicit Connection
				response[LENGTH] = 2;
			}
			else  // Path for data sent in I/O Poll response
			{
				memcpy(&response[2], produced_conxn_path, 6);
				response[LENGTH] = 8;
			}
			break;

			case 15: // consumed conxn path length
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(consumed_conxn_path_length);
			response[3] = HIBYTE(consumed_conxn_path_length);
			response[LENGTH] = 4;
			break;

			case 16:  // consumed conxn path
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			// path is NULL for both Explicit and I/O Poll Connections
			response[LENGTH] = 2;
			break;

			case 17:  // production inhibit time
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(production_inhibit_time);
			response[3] = HIBYTE(production_inhibit_time);
			response[LENGTH] = 4;
			break;


			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;


		case SET_REQUEST:
		switch(attrib)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			error = ATTRIB_NOT_SETTABLE;
			break;

			case 9: // Set EPR
			test_val = request[5] + 256 * request[6];

			// Limit EPR so rounding up doesn't take it over 65,535
			if (test_val > 65000) error = INVALID_ATTRIB_VALUE;
			else
			{
				// This tweaks the EPR to be a multiple of 50
				temp = test_val / 50;
				if ((temp * 50) != test_val) temp++;   // round up
				expected_packet_rate = temp * 50;

				// see if I/O Poll connection should be established
				if ((instance == IO_POLL) && (state == CONFIGURING))
				{
					set_state(ESTABLISHED);
					// IF EPR > 2500 use it, else use 2500 (200 counts)
					// This ensures that timeout time is at least 10 seconds
					if ((expected_packet_rate > 2500) ||
						 (expected_packet_rate == 0))
					{
						global_timer[IO_POLL] = (expected_packet_rate / 50) * 4;
					}
					else global_timer[IO_POLL] = 200;
				}
				else global_timer[instance] = (expected_packet_rate / 50) * 4;

				response[0] = request[0] & NON_FRAGMENTED;
				response[1] = service | SUCCESS_RESPONSE;
				response[2] = LOBYTE(expected_packet_rate);
				response[3] = HIBYTE(expected_packet_rate);
				response[LENGTH] = 4;
			}
			break;

			case 12:  // Set watchdog timeout action
			if (instance == IO_POLL) error = ATTRIB_NOT_SETTABLE;
			else if ((request[5] != 1) &&
						(request[5] != 3)) error = INVALID_ATTRIB_VALUE;
			else
			{
				watchdog_timeout_action = request[5];
				response[0] = request[0] & NON_FRAGMENTED;
				response[1] = service | SUCCESS_RESPONSE;
				response[LENGTH] = 2;
			}
			break;


			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;

		default:
		error = SERVICE_NOT_SUPPORTED;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}



// This consumes incoming DeviceNet messages.  It handles the following:
//    Explicit requests (fragmented and unfragmented)
//    I/O Poll requests (no data is sent with these)
//    Acks to explicit fragments I sent earlier
int CONNECTION::link_consumer(UCHAR request[])
{
	UCHAR i, length, fragment_count, fragment_type, fragment_flg;
	UCHAR temp_buffer[BUFSIZE];
   
	if ((instance == EXPLICIT) && (state == DEFERRED)) state = ESTABLISHED;
	if (state != ESTABLISHED) return NO_RESPONSE;

	global_timer[instance] = (expected_packet_rate / 50) * 4;    // restart connxn timer

	// Handle an I/O Poll request
	if (instance == IO_POLL)
	{
		// The Master should not send me any data in I/O message
		if (request[LENGTH] == 0) return OK;
		else return NO_RESPONSE;

	}
	// From this point on we are dealing with an Explicit message
	// If it is not fragmented, reset frag counters to initial state
	// and process the current incoming message.
	if ((request[0] & 0x80) == 0)
	{
		rcve_index = 0;
		xmit_index = 0;
		my_rcve_fragment_count = 0;
		my_xmit_fragment_count = 0;
		ack_timeout_counter = 0;
		global_timer[ACK_WAIT] = 0;
		return OK;
	}

	// At this point we are dealing with either an explicit fragment
	// or an ACK to an explicit message we sent earlier
	fragment_type =  request[1] & 0xC0;
	fragment_count = request[1] & 0x3F;
	length = request[LENGTH];


	switch(fragment_type)
	{
		case ACK_FRAG:
		// Received an ack from the master to an explicit fragment I sent earlier
		// Send the request to the link producer along with a tag so it knows
		// what do do with the message
		request[MESSAGE_TAG] = RECEIVED_ACK;
		link_producer(request);
		break;


		case FIRST_FRAG:
		if (fragment_count != 0)
		{
			// Reset fragment counters to initial state and drop fragment
			rcve_index = 0;
			my_rcve_fragment_count = 0;
			return NO_RESPONSE;
		}

		// get rid of any existing fragments and process first fragment
		rcve_index = 0;
		my_rcve_fragment_count = 0;
		memset(rcve_fragment_buf, 0, BUFSIZE);
		rcve_fragment_buf[rcve_index] = request[0] & 0x7F; // remove fragment flag
		rcve_index++;
		for (i = 2; i < length; i++)      // do not copy fragment info
		{
			rcve_fragment_buf[rcve_index] = request[i];
			rcve_index++;
		}

		// Check to see if Master has exceeded byte count limit
		if (rcve_index > consumed_conxn_size)
		{
			rcve_index = 0;                       // reset fragment counters
			my_rcve_fragment_count = 0;
			request[MESSAGE_TAG] = ACK_ERROR;
			link_producer(request);
		}
		else  // fragment checked out OK so send ACK
		{
			request[MESSAGE_TAG] = SEND_ACK;
			link_producer(request);
			my_rcve_fragment_count++;
		}
		break;



		case MIDDLE_FRAG:
		if (my_rcve_fragment_count == 0) return NO_RESPONSE;  // just drop fragment

		// See if Master has sent the same fragment again.  If so just send ACK
		if (fragment_count == (my_rcve_fragment_count - 1))
		{
			 request[MESSAGE_TAG] = SEND_ACK;
			 link_producer(request);
		}
		// See if received fragment count does not agree with my count
		// If so, reset to beginning
		else if (fragment_count != my_rcve_fragment_count)
		{
				rcve_index = 0;
				my_rcve_fragment_count = 0;
		}

		else  // Fragment was validated so process it
		{
			// Copy the fragment to reassembly buffer, omit first 2 bytes
			for (i = 2; i < length; i++)
			{
				rcve_fragment_buf[rcve_index] = request[i];
				rcve_index++;
			}
			// Check to see if Master has exceeded byte count limit
			if (rcve_index > consumed_conxn_size)
			{
				rcve_index = 0;
				my_rcve_fragment_count = 0;
				request[MESSAGE_TAG] = ACK_ERROR;
				link_producer(request);
			}
			else  // send good ACK back to Master
			{
				request[MESSAGE_TAG] = SEND_ACK;
				link_producer(request);
				my_rcve_fragment_count++;
			}
		}
		break;



		case LAST_FRAG:
		if (my_rcve_fragment_count == 0) return NO_RESPONSE;  // just drop fragment


		// See if received fragment count does not agree with my count
		// If so, reset to beginning
		else if (fragment_count != my_rcve_fragment_count)
		{
				rcve_index = 0;
				my_rcve_fragment_count = 0;
		}
		else  // Fragment was validated, so process it
		{
			// Copy the fragment to reassembly buffer, omit first 2 bytes
			for (i = 2; i < length; i++)
			{
				rcve_fragment_buf[rcve_index] = request[i];
				rcve_index++;
			}

			// Check to see if Master has exceeded byte count limit
			if (rcve_index > consumed_conxn_size)
			{
				rcve_index = 0;
				my_rcve_fragment_count = 0;
				memset(rcve_fragment_buf, 0, BUFSIZE);
				request[MESSAGE_TAG] = ACK_ERROR;
				link_producer(request);
			}
			else  // Fragment was OK, send ACK and reset everything
			{
				request[MESSAGE_TAG] = SEND_ACK;
				link_producer(request);
				// We are done receiving a fragmented explicit message
				// Copy from temporary buffer back to request buffer
				// and process complete Explicit message
				memcpy(request, rcve_fragment_buf, BUFSIZE);
				request[LENGTH] = rcve_index;
				rcve_index = 0;
				my_rcve_fragment_count = 0;
				memset(rcve_fragment_buf, 0, BUFSIZE);
				return OK;
			}
		}
		break;


		default:
		break;
	}

	return NO_RESPONSE;
}



// This produces outgoing DeviceNet messages.  It handles the following:
//    Explicit responses (fragmented and unfragmented)
//    I/O Poll responses (Always sends 3 bytes of data)
//    Fragmented ack messages (to ack the Master's Explicit fragments)
void CONNECTION::link_producer(UCHAR response[])
{
	UCHAR length, bytes_left, i, fragment_count, ack_status;
	static UCHAR copy[BUFSIZE];

	length = response[LENGTH];
   
	if (instance == IO_POLL)
	{
		// load io poll response into can chip object #9
		for (i=0; i < length; i++)  						// load CAN data
		{
			pokeb(CAN_BASE, (0x57 + i), response[i]);
		}
		pokeb(CAN_BASE, 0x56, ((length << 4) | 0x08));	// load config register
		pokeb(CAN_BASE, 0x51, 0x66);      					// set transmit request
	}


	else if (response[MESSAGE_TAG] == ACK_TIMEOUT)
	{
		ack_timeout_counter++;
		if (ack_timeout_counter == 1)
		{
			// Load last explicit fragment send again
			length = copy[LENGTH];
			for (i=0; i < length; i++)  						// load data into CAN
			{
				pokeb(CAN_BASE, (0x67 + i), copy[i]);
			}
			pokeb(CAN_BASE, 0x66, ((length << 4) | 0x08));	// load config resister
			pokeb(CAN_BASE, 0x61, 0x66);      					// set transmit request
			global_timer[ACK_WAIT] = 20;

		}
		if (ack_timeout_counter == 2)
		{
			// abort trying to send fragmented message
			xmit_index = 0;
			my_xmit_fragment_count = 0;
			ack_timeout_counter = 0;
			global_timer[ACK_WAIT] = 0;
		}
	}


	else if (response[MESSAGE_TAG] == RECEIVED_ACK)
	{
		fragment_count = response[1] & 0x3F;
		ack_status = response[2];
		if (my_xmit_fragment_count == fragment_count)
		{
			// If Master returned a bad ACK status, reset everything
			// and abort the attempt to send message
			if (ack_status != 0)
			{
				xmit_index = 0;
				my_xmit_fragment_count = 0;
				ack_timeout_counter = 0;
				global_timer[ACK_WAIT] = 0;
			}

			// Master's ACK was OK, so send next fragment unless we were done sending
			// Keep a copy of what we are sending
			else
			{
				if (xmit_index >= xmit_fragment_buf[LENGTH])
				{
					// got ACK to out final fragment so reset everything
					xmit_index = 0;
					my_xmit_fragment_count = 0;
					ack_timeout_counter = 0;
					global_timer[ACK_WAIT] = 0;
				}
				else
				{
					// Send another fragment
					// Figure out how many bytes are left to send
					bytes_left = xmit_fragment_buf[LENGTH] - xmit_index;
					my_xmit_fragment_count++;
					ack_timeout_counter = 0;
					global_timer[ACK_WAIT] = 20;				// restart timer for 1 second
					// Load the first byte of the fragment
					copy[0] = response[0] | 0x80;
					pokeb(CAN_BASE, 0x67, copy[0]);
					if (bytes_left > 6)			// this is a middle fragment
					{
						copy[1] = MIDDLE_FRAG | my_xmit_fragment_count;
						pokeb(CAN_BASE, 0x68, copy[1]);
						length = 8;
					}
					else   							// this is the last fragment
					{
						copy[1] = LAST_FRAG | my_xmit_fragment_count;
						pokeb(CAN_BASE, 0x68, copy[1]);
						length = bytes_left + 2;
					}
					// Put in actual data
					for (i = 2; i < length; i++)  // put up to 6 more bytes in CAN chip
					{
						copy[i] = xmit_fragment_buf[xmit_index];
						pokeb(CAN_BASE, (0x67 + i), copy[i]);
						xmit_index++;
					}
					copy[LENGTH] = length;
					pokeb(CAN_BASE, 0x66, ((length << 4) | 0x08));	// load config resister
					pokeb(CAN_BASE, 0x61, 0x66);      // set msg object transmit request
				}
			}
		}
	}

   
	// Send this message in response to receiving an explicit fragment
	// from the Master that the link comsumer has validated as OK
	else if (response[MESSAGE_TAG] == SEND_ACK)
	{
		length = 3;												// This is a 3 byte message
		pokeb(CAN_BASE, 0x67, (response[0] | 0x80));
		pokeb(CAN_BASE, 0x68, (response[1] | ACK_FRAG));
		pokeb(CAN_BASE, 0x69, 0);								// ack status = OK
		pokeb(CAN_BASE, 0x66, ((length << 4) | 0x08));	// load config resister
		pokeb(CAN_BASE, 0x61, 0x66);      					// set transmit request
	}



	// Send this message in response to receiving an explicit fragment
	// from the Master that exceeded the byte count limit for the connection
	else if (response[MESSAGE_TAG] == ACK_ERROR)
	{
		length = 3;												// This is a 3 byte message
		pokeb(CAN_BASE, 0x67, (response[0] | 0x80));
		pokeb(CAN_BASE, 0x68, (response[1] | ACK_FRAG));
		pokeb(CAN_BASE, 0x69, 1);								// ack status = TOO MUCH DATA
		pokeb(CAN_BASE, 0x66, ((length << 4) | 0x08));	// load config resister
		pokeb(CAN_BASE, 0x61, 0x66);      					// set transmit request
	}


   
	else if (length <= 8)		// Send complete Explicit message
	{
		// load explicit response into can chip object #3
		for (i=0; i < length; i++)  						// load data into CAN
		{
			pokeb(CAN_BASE, (0x67 + i), response[i]);
		}
		pokeb(CAN_BASE, 0x66, ((length << 4) | 0x08));	// load config resister
		pokeb(CAN_BASE, 0x61, 0x66);      					// set transmit request
	}



	else if (length > 8)       // Send first Explicit message fragment
	{
		// load explicit response into a buffer to keep around a while
		memcpy(xmit_fragment_buf, response, BUFSIZE);
		length = 8;
		xmit_index = 0;
		my_xmit_fragment_count = 0;
		ack_timeout_counter = 0;
		// Load first fragment into can chip object #3
		copy[0] = response[0] | 0x80;
		pokeb(CAN_BASE, 0x67, copy[0]);
		xmit_index++;
		// Put in fragment info
		copy[1] = FIRST_FRAG | my_xmit_fragment_count;
		pokeb(CAN_BASE, 0x68, copy[1]);
		// Put in actual data
		for (i = 2; i < 8; i++)  // put 6 more bytes in CAN chip
		{
			copy[i] = xmit_fragment_buf[xmit_index];
			pokeb(CAN_BASE, (0x67 + i), copy[i]);
			xmit_index++;
		}
		copy[LENGTH] = length;
		pokeb(CAN_BASE, 0x66, ((length << 4) | 0x08));	// load config resister
		pokeb(CAN_BASE, 0x61, 0x66);      					// set msg object transmit request
		global_timer[ACK_WAIT] = 20;							// start timer to wait for ack
	}
}




//---------------------------------------------------------------------------------
// Identity Class, class ID = 1
// It is used for the Identity Object. I extended this object to handle the
// device self-test, diagnostics, and handle LED states
//
// Class Attributes supported:      Revision
// Class Services supported: 		 	Get Attribute Single
// Instance Attributes supported:   Vendor ID, Device type, Porduct code, Revision,
//												Status, Serial number, Product name, State.
// Instance Services supported:     Get Attribute Single, Reset (type 0 & type 1)
//---------------------------------------------------------------------------------
class IDENTITY
{
	private:
	static UINT class_revision;
	UINT vendor_id;
	UINT device_type;
	UINT product_code;
	struct
	{
		UCHAR major;
		UCHAR minor;
	} revision;

	ULONG serial;
	char product_name[40];
	ULONG device_clock, dup_mac_clock;
	CONNECTION *explicit, *io_poll;


	public:
	UINT status;
	UCHAR state;
	static void handle_class_inquiry(UCHAR*, UCHAR*);
	void handle_explicit(UCHAR*, UCHAR*);
	UCHAR get_state(void);
	void device_self_test(void);
	void send_dup_mac_check_message(void);
	void update_device(void);
	IDENTITY(UINT id, ULONG s, CONNECTION *ex, CONNECTION *io)  // constructor
	{
		device_clock = 0;
		dup_mac_clock = 0;
		vendor_id = id;
		serial = s;
		explicit = ex;
		io_poll = io;
		device_type = 0;  				// generic device
		product_code = 1;       		// product model within a device type
		revision.major = 1;     		// rev level of product model
		revision.minor = 0;
		status = 0;             		// status of the device
		state = 0;                    // state of the device
		memset(product_name, 0, 40);
		strcpy(product_name, "WEATHER STATION");
	}
};



// Handle Explicit requests to class
void IDENTITY::handle_class_inquiry(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;

	service = request[1];
	attrib = request[4];
	error = 0;
	memset(response, 0, BUFSIZE);

	switch(service)
	{
		case GET_REQUEST:
		switch(attrib)
		{
			case 1:  // get revision attribute
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(class_revision);
			response[3] = HIBYTE(class_revision);
			response[LENGTH] = 4;
			break;

			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;


		default:
		error = SERVICE_NOT_SUPPORTED;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}


// Handle Explicit requests to Identity Object
void IDENTITY::handle_explicit(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;
	UCHAR reset_type;

	service = request[1];
	attrib = request[4];
	error = 0;
	memset(response, 0, BUFSIZE);

	switch(service)
	{
		case GET_REQUEST:
		switch(attrib)
		{
			case 1:  // get ODVA vendor ID
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(vendor_id);
			response[3] = HIBYTE(vendor_id);
			response[LENGTH] = 4;
			break;

			case 2:  // get device type
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(device_type);
			response[3] = HIBYTE(device_type);
			response[LENGTH] = 4;
			break;

			case 3:  // get product code
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(product_code);
			response[3] = HIBYTE(product_code);
			response[LENGTH] = 4;
			break;

			case 4:  // get revision level
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = revision.major;
			response[3] = revision.minor;
			response[LENGTH] = 4;
			break;

			case 5:  // get summary status
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(status);
			response[3] = HIBYTE(status);
			response[LENGTH] = 4;
			break;

			case 6:  // get serial number - 4 byte
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = (UCHAR)(serial);
			response[3] = (UCHAR)(serial >> 8);
			response[4] = (UCHAR)(serial >> 16);
			response[5] = (UCHAR)(serial >> 24);
			response[LENGTH] = 6;
			break;

			case 7:  // get product name (short string format)
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = (UCHAR)strlen(product_name);
			strcpy(&response[3], product_name);
			response[LENGTH] = response[2] + 3;
			break;

			case 8:  // get device state
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = state;
			response[LENGTH] = 3;
			break;


			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;


		case RESET_REQUEST:
      // If reset type is not specified, default to type zero
		if (request[LENGTH] == 4) reset_type = 0;
		else reset_type = request[4];
      
		if (reset_type == 0)    		// simulate off/on cycle
		{
			// return success response now, can't do it after reset
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[LENGTH] = 2;
			global_event |= FULL_RESET;      // set reset bit
		}
		else if (reset_type == 1)
		{
			// reset to out-of-box condition, then simulate power off/on cycle
			// return success response before if can't do it after
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[LENGTH] = 2;
			global_event |= FULL_RESET;      // set reset bit
		}
		else  error = INVALID_PARAMETER;
		break;


		default:
		error = SERVICE_NOT_SUPPORTED;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}




// Returns the state of the device
UCHAR IDENTITY::get_state(void)
{
	return state;
}





// Handles device startup & LED states
void IDENTITY::device_self_test(void)
{
   // During startup sequence, flash module led, then network led
   // The device clock happens every 0.25 second , so it takes
	// 2.0 seconds to run the self test.
	switch(device_clock)
	{
		case 0: // both leds off
		pokeb(PIO_BASE, PORTC, 0xF0); // high nibble is LEDs
		break;

		case 1:
		// module LED on green
		pokeb(PIO_BASE, PORTC, 0xE0);
		break;

		case 2:
		// module LED on red
		pokeb(PIO_BASE, PORTC, 0xD0);
		break;

		case 3:
      // both LEDs off
		pokeb(PIO_BASE, PORTC, 0xF0); // high nibble is LEDs
		break;

		case 4:
		// module LED off, network LED green
		pokeb(PIO_BASE, PORTC, 0xB0);
		break;

		case 5:
		// network LED red
		pokeb(PIO_BASE, PORTC, 0x70);
		break;

		case 6:
		// both LEDs off
		pokeb(PIO_BASE, PORTC, 0xF0);
		break;

		case 7:
		// transition from self test to operational if no faults
		global_status &= (~SELF_TESTING);
		if ((global_status & DEVICE_FAULT) == 0)
		{
			global_status |= OPERATIONAL;
			// module LED green
			state = 3;		// indicate device is operational
			pokeb(PIO_BASE, PORTC, 0xE0);
		}
		else
		{
			// module LED red
			state = 5;			// indicate a fault
			pokeb(PIO_BASE, PORTC, 0xD0);
		}
	}
}




// Sends a DUP MAC ID check request.
// Two of these are sent during the startup sequence.
void IDENTITY::send_dup_mac_check_message(void)
{
	// Send first dup MAC ID check message
	// Put message into CAN chip msg object #7 and send
	// Load CAN message config register - msg length = 7
	pokeb(CAN_BASE, 0x76, 0x78);

	// Load data area of CAN chip
	pokeb(CAN_BASE, 0x77, 0);		  // request flag (not response)
	pokeb(CAN_BASE, 0x78, LOBYTE(vendor_id));
	pokeb(CAN_BASE, 0x79, HIBYTE(vendor_id));
	pokeb(CAN_BASE, 0x7A, (UCHAR)(serial));
	pokeb(CAN_BASE, 0x7B, (UCHAR)(serial >> 8));
	pokeb(CAN_BASE, 0x7C, (UCHAR)(serial >> 16));
	pokeb(CAN_BASE, 0x7D, (UCHAR)(serial >> 24));
	pokeb(CAN_BASE, 0x71, 0x66);      // set msg object #7 transmit request

}



// Handles operation of overall device, checks for errors, handles LEDs,
// This runs every 0.25 second.  If the startup self-test is OK,
// the Device sends two dup MAC ID check messages
void IDENTITY::update_device(void)
{
	UCHAR explicit_conxn_state, io_poll_conxn_state;
	UCHAR temp;


	// For first 2 seconds (8 clock ticks) do self-test
	if (device_clock < 8) device_self_test();
	else if ((dup_mac_clock <= 8) &&
			  ((global_status & DEVICE_FAULT) == 0) &&
			  ((global_status & NETWORK_FAULT) == 0))
	{
		if (global_status & LONELY_NODE) dup_mac_clock = 0;
		else
		{
			// This generates two dup mac check messages one second apart
			// If we are the only node on the network, keep sending
			switch(dup_mac_clock)
			{
				case 0:      // start
				send_dup_mac_check_message();
				break;

				case 4:      // 1 second has elapsed
				send_dup_mac_check_message();
				break;

				case 8:      // 2 seconds have elapsed
				global_status |= ON_LINE;
				break;
			}
			dup_mac_clock++;
		}
	}

	if (device_clock >= 8) // self-test done, update status and module LED
	{
		// Copy the global status into the identity object status attribute
		status = global_status & 0x0F05;  				// zero out some bits
		if (global_status & DEVICE_FAULT)
		{
			// module LED red
			state = 5;		// indicate device fault
			temp = peekb(PIO_BASE, PORTC);
			temp &= 0xD0;		// red on
			temp |= 0x10;		// grn off
			pokeb(PIO_BASE, PORTC, temp);
		}
		else if (global_status & OPERATIONAL)
		{
			// module LED green
			state = 3;			// indicate device is operational
			temp = peekb(PIO_BASE, PORTC);
			temp &= 0xE0;		// grn on
			temp |= 0x20;		// red off
			pokeb(PIO_BASE, PORTC, temp);
		}

		// Next, handle network bicolor LED
		// Check state of connections
		explicit_conxn_state = explicit->get_state();
		io_poll_conxn_state = io_poll->get_state();

		if (global_status & NETWORK_FAULT)
		{
			// network LED steady red
			temp = peekb(PIO_BASE, PORTC);
			temp &= 0x70;		// red on
			temp |= 0x40;		// grn off

			pokeb(PIO_BASE, PORTC, temp);
		}
		else if ((global_status & ON_LINE) == 0)
		{
			// network LED off
			temp = peekb(PIO_BASE, PORTC);
			temp |= 0xC0;		// both off
			pokeb(PIO_BASE, PORTC, temp);
		}
		else if (io_poll_conxn_state == TIMED_OUT)
		{
			// network LED flashing red
			temp = peekb(PIO_BASE, PORTC);
			if (((device_clock / 2) % 2) == 0)
			{
				temp &= 0x70;		// red on
				temp |= 0x40;		// grn off
				pokeb(PIO_BASE, PORTC, temp);
			}
			else
			{
				temp |= 0xC0;		// both off
				pokeb(PIO_BASE, PORTC, temp);
			}
		}
		else if ((io_poll_conxn_state != ESTABLISHED) &&
					(explicit_conxn_state != ESTABLISHED))
		{
			// network LED flashing green
			temp = peekb(PIO_BASE, PORTC);
			if (((device_clock / 2) % 2) == 0)
			{
				temp &= 0xB0;		// grn on
				temp |= 0x80;		// red off
				pokeb(PIO_BASE, PORTC, temp);
			}
			else
			{
				temp |= 0xC0;		// both on
				pokeb(PIO_BASE, PORTC, temp);
			}
		}
		else if ((io_poll_conxn_state == ESTABLISHED) ||
					(explicit_conxn_state == ESTABLISHED))
		{
			// at least one connection is established
			// network LED steady green
			temp = peekb(PIO_BASE, PORTC);
			temp &= 0xB0;		// grn on
			temp |= 0x80;		// red off
			pokeb(PIO_BASE, PORTC, temp);
		}
	}
	device_clock++;
}




//---------------------------------------------------------------------------------
// Assembly Class, class ID = 4
// It is used for the Assemly Object. This device only has one assembly.  It is
//	an input assembly which assembles Device State, Temperature, Humidity
// for a total of 3 bytes.  Device State is attribute 8 of the Identity Object.
//
// Class Attributes supported:      Revision
// Class Services supported: 		 	Get Attribute Single
// Instance Attributes supported:   Data (array of three unsigned chars)
// Instance Services supported:     Get Attribute Single
//---------------------------------------------------------------------------------
class ASSEMBLY
{
	private:
	static UINT class_revision;
	UCHAR data[3];
   
	IDENTITY *identity;
	ANALOG_INPUT_POINT *temperature_sensor, *humidity_sensor;
   
	public:
	ASSEMBLY(IDENTITY *id, ANALOG_INPUT_POINT *ts, ANALOG_INPUT_POINT *hs)
	{
		identity = id;
		temperature_sensor = ts;
		humidity_sensor = hs;
   }
	static void handle_class_inquiry(UCHAR*, UCHAR*);
	void handle_explicit(UCHAR*, UCHAR*);
	void update_data(void);
	void handle_io_poll_request(UCHAR*, UCHAR*);

};





// Handle an Explicit request directed to the class
void ASSEMBLY::handle_class_inquiry(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;

	service = request[1];
	attrib = request[4];
	error = FALSE;
	memset(response, 0, BUFSIZE);

	switch(service)
	{
		case GET_REQUEST:
		switch(attrib)
		{
			case 1:  // handle revision attribute of class
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(class_revision);
			response[3] = HIBYTE(class_revision);
			response[LENGTH] = 4;
			break;

			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;


		default:
		error = SERVICE_NOT_SUPPORTED;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}




// Handle an explicit request to the assembly object
void ASSEMBLY::handle_explicit(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;
	UCHAR temp;
	service = request[1];
	attrib = request[4];
	error = FALSE;
	memset(response, 0, BUFSIZE);

	switch(service)
	{
		case GET_REQUEST:
		switch(attrib)
		{
			case 3:   // get data - array of 3 bytes
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = data[0];
			response[3] = data[1];
         response[4] = data[2];
			response[LENGTH] = 5;
			break;

			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;


		default:
		error = SERVICE_NOT_SUPPORTED;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}



// Runs every 0.25 seconds to update the local copy
// This keeps the values up to date, and ready to send.
void ASSEMBLY::update_data(void)
{
	// the assembly object keeps local copies of the values it needs to
	// send in the I/O POLL response.  It is faster that way.
	data[0] = identity->get_state();
	data[1] = temperature_sensor->get_value();
	data[2] = humidity_sensor->get_value();
}



// Handle an I/O Poll request for data
void ASSEMBLY::handle_io_poll_request(UCHAR request[], UCHAR response[])
{
	// This is an input assembly so it does not expect to receive data
	// from master. It returns 3 bytes of data to send in the io poll response
   memset(response, 0, BUFSIZE);
	memcpy(response, data, 3);
	response[LENGTH] = 3;
}




//---------------------------------------------------------------------------------
// DeviceNet Class, class ID = 3
// It is used for the DeviceNet Object.
//
// Class Attributes supported:      Revision
// Class Services supported: 		 	Get Attribute Single
// Instance Attributes supported:   MAC ID, Baud rate, BOI, Allocation info.
// Instance Services supported:     Get Attribute Single, Allocage master/slave
//												conxn set, Release master/slave conxn set.
//---------------------------------------------------------------------------------
class DEVICENET  // DeviceNet class
{
	private:
	static UINT class_revision;
	// declare pointers of type CONNECTION to allow
	// sending messages to the connection objects
	CONNECTION *explicit_conxn, *io_poll_conxn;

	public:
	UCHAR mac_id;
	ULONG serial;
	UCHAR baud_rate;
	UINT vendor_id;
	BOOL bus_off_int;
	struct
	{
		UCHAR choice;
		UCHAR my_master;
	} allocation;
	UCHAR physical_port;
	static void handle_class_inquiry(UCHAR*, UCHAR*);
	int handle_unconnected_port(UCHAR*, UCHAR*);
	void handle_timeout(UCHAR);
	void handle_explicit(UCHAR*, UCHAR*);
	int consume_dup_mac(UCHAR*);
	void send_dup_mac_response(void);
	DEVICENET(UCHAR mac, UCHAR baud, UINT id, ULONG ser, CONNECTION *exp, CONNECTION *io)
	{
		explicit_conxn = exp;
		io_poll_conxn = io;
		mac_id = mac;
		baud_rate = baud;
		vendor_id = id;
		serial = ser;
		allocation.choice = 0;                      		// no connections
		allocation.my_master = DEFAULT_MASTER_MAC_ID;  	// device not yet allocated
		bus_off_int = 0;
		physical_port = 0;
	}
};




// Handle Explicit request to class
void DEVICENET::handle_class_inquiry(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;

	service = request[1];
	attrib = request[4];
	error = 0;
	memset(response, 0, BUFSIZE);

	switch(service)
	{
		case GET_REQUEST:
		switch(attrib)
		{
			case 1:  // get revision attribute
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(class_revision);
			response[3] = HIBYTE(class_revision);
			response[LENGTH] = 4;
			break;

			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;


		default:
		error = SERVICE_NOT_SUPPORTED;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}




// When a connection timer expires, this function transitions the connection
// to the appropriate state.  After this, if neither connection is in
// the established state, it releases all connections and the device is no
// longer "owned" by the master. 
void DEVICENET::handle_timeout(UCHAR conxn)
{
	UCHAR timeout_action;

  	if (conxn == EXPLICIT_CONXN)
	{
      timeout_action = explicit_conxn->get_timeout_action();
		// If in auto-delete mode, or if in deferred delete mode
      // and I/O Poll conxn is not established, delete Explicit conxn.
		if ((timeout_action == 1) ||
			((timeout_action == 3) && (io_poll_conxn->get_state() != ESTABLISHED)))
		{  
			allocation.choice &= (~EXPLICIT_CONXN);
			explicit_conxn->set_state(NON_EXISTENT);
      }
		// If in deferred-delete mode and I/O Poll conxn is established
		// go to deferred state (still considered allocated)
		else if ((timeout_action == 3) && (io_poll_conxn->get_state() == ESTABLISHED))
		{
			explicit_conxn->set_state(DEFERRED);
		}
	}

	if (conxn == IO_POLL_CONXN)
	{
		allocation.choice &= (~IO_POLL_CONXN);
		io_poll_conxn->set_state(TIMED_OUT);
		// If Explicit is in deferred state, delete Explicit connnection
		if (explicit_conxn->get_timeout_action() == 3)
		{
			allocation.choice &= (~EXPLICIT_CONXN);
			explicit_conxn->set_state(NON_EXISTENT);
		}
	}

	// If neither connection is in established state, release everything
	if ((explicit_conxn->get_state() != ESTABLISHED) &&
		(io_poll_conxn->get_state() != ESTABLISHED))
	{
		explicit_conxn->set_state(NON_EXISTENT);
		io_poll_conxn->set_state(NON_EXISTENT);
		allocation.choice = 0;
		global_status &= (~OWNED);
		allocation.my_master = DEFAULT_MASTER_MAC_ID;
	}
}



// Handles a request to the unconnected port.  This will be a request
// from the Master to allocate or release connections.
int DEVICENET::handle_unconnected_port(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;
   UCHAR additional_code, master_mac_id, alloc_request, release_request;

	service = request[1];
	error = 0;
	additional_code = 0;
	memset(response, 0, BUFSIZE);

	// Note:  In this context, "allocated" means that a connection is in
	// either the CONFIGURING, ESTABLISHED, or DEFERRED states, as opposed
	// to being in the NON_EXISTENT or TIMED_OUT states

	switch(service)
	{
		case ALLOCATE_CONNECTIONS:      // request to allocate connections
		// first validate the request
		alloc_request = request[4];
		master_mac_id = request[5] & 0x3F;
		if ((master_mac_id != allocation.my_master) &&
			 (allocation.my_master != DEFAULT_MASTER_MAC_ID))
		{
			error = OBJECT_STATE_CONFLICT;
			additional_code = ALLOCATION_CONFLICT;
		}
		else if (alloc_request == 0)   	// master must allocate something
		{
			error = INVALID_ATTRIB_VALUE;
			additional_code = INVALID_ALLOC_CHOICE;
		}
		// see if the master sets ack bit but neither COS nor Cyclic are set
		else if ((alloc_request & 0x40) && ((alloc_request & 0x30) == 0))
		{
			error = INVALID_ATTRIB_VALUE;
			additional_code = INVALID_ALLOC_CHOICE;
		}
		// see if master tries to allocate something not supported
		else if (alloc_request & 0xFC)
		{
			error = RESOURCE_UNAVAILABLE;
			additional_code = INVALID_ALLOC_CHOICE;
		}
		// Master must at least allocate explicit unless it is already allocated
		else if (((alloc_request & EXPLICIT_CONXN) == 0) &&
				  ((allocation.choice & EXPLICIT_CONXN) == 0))
		{
			error = INVALID_ATTRIB_VALUE;
			additional_code = INVALID_ALLOC_CHOICE;
		}
		// Master must allocate either explicit or poll or both
		else if ((alloc_request & (EXPLICIT_CONXN | IO_POLL_CONXN)) == 0)
		{
			error = INVALID_ATTRIB_VALUE;
			additional_code = INVALID_ALLOC_CHOICE;
		}
		// check to see if Master is trying to allocate a connection already allocated
		else if (((alloc_request & EXPLICIT_CONXN) && (allocation.choice & EXPLICIT_CONXN)) ||
				  ((alloc_request & IO_POLL_CONXN) && (allocation.choice & IO_POLL_CONXN)))
		{
			error = ALREADY_IN_STATE;
			additional_code = INVALID_ALLOC_CHOICE;
		}
		else // passed validation tests
		{
			allocation.my_master = master_mac_id;
			global_status |= OWNED;

			// send message to explicit connection object
			if (alloc_request & EXPLICIT_CONXN)
			{
				allocation.choice |= EXPLICIT_CONXN;
				explicit_conxn->set_state(ESTABLISHED);
			}

			if (alloc_request & IO_POLL_CONXN)
			{
				allocation.choice |= IO_POLL_CONXN;
				io_poll_conxn->set_state(CONFIGURING);
			}

			// prepare response
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = 0;   // 8/8 message format
			response[LENGTH] = 3;
		}
		break;



		case RELEASE_CONNECTIONS:
		release_request = request[4];
		if (release_request == 0)
		{
			error = INVALID_ATTRIB_VALUE;
			additional_code = INVALID_ALLOC_CHOICE;
		}
		// must not try to release something we don't support
		else if (release_request & 0xFC)
		{
			error = RESOURCE_UNAVAILABLE;
			additional_code = INVALID_ALLOC_CHOICE;
		}
		// the connection must exist to be released
		else if (((release_request & EXPLICIT_CONXN) &&
				  (explicit_conxn->get_state() == NON_EXISTENT)) ||
				  ((release_request & IO_POLL_CONXN) &&
				  (io_poll_conxn->get_state() == NON_EXISTENT)))
		{
			error = ALREADY_IN_STATE;
			additional_code = INVALID_ALLOC_CHOICE;
		}
		else // passed validation tests
		{
			if (release_request & EXPLICIT_CONXN)
			{
				allocation.choice &= (~EXPLICIT_CONXN);
				explicit_conxn->set_state(NON_EXISTENT);
			}
			if (release_request & IO_POLL_CONXN)
			{
				allocation.choice &= (~IO_POLL_CONXN);
				io_poll_conxn->set_state(NON_EXISTENT);
			}
			// If neither connection is in established state, release everything
			if ((explicit_conxn->get_state() != ESTABLISHED) &&
				 (io_poll_conxn->get_state() != ESTABLISHED))
			{
	   		explicit_conxn->set_state(NON_EXISTENT);
				io_poll_conxn->set_state(NON_EXISTENT);
				allocation.choice = 0;
            global_status &= (~OWNED);
				allocation.my_master = DEFAULT_MASTER_MAC_ID;
         }
         
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[LENGTH] = 2;
		}
		break;


		default:
		error = RESOURCE_UNAVAILABLE;
		additional_code = INVALID_UNC_REQUEST;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = additional_code;
		response[LENGTH] = 4;
	}
	return OK;
}




// Handles Explicit request to the DeviceNet Object
void DEVICENET::handle_explicit(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;

	service = request[1];
	attrib = request[4];
	error = 0;
	memset(response, 0, BUFSIZE);

	switch(service)
	{
		case GET_REQUEST:
		switch(attrib)
		{
			case 1: // get this device's mac id
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = mac_id;
			response[LENGTH] = 3;
			break;

			case 2: // get baud rate
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = baud_rate;
			response[LENGTH] = 3;
			break;

			case 3: // get bus off int value
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = bus_off_int;
			response[LENGTH] = 3;
			break;

			case 5: // get allocation information struct
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = allocation.choice;
			response[3] = allocation.my_master;
			response[LENGTH] = 4;
			break;

			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;


		case ALLOCATE_CONNECTIONS:
		// request to allocate connections received through the message router
		// forward the request to the unconnected port handler
		handle_unconnected_port(request, response);
		break;


		case RELEASE_CONNECTIONS:
		// request to release connections received through the message router
		// forward the request to the unconnected port handler
		handle_unconnected_port(request, response);
		break;



		default:
		error = SERVICE_NOT_SUPPORTED;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}


// Send a DUP MAC check response message.  This is a message sent in
// response to receiving a DUP MAC check message (with my MAC ID) from
// another device which is hoping to go on-line.  Sending this message
// keeps the offending device from going on-line. 
void DEVICENET::send_dup_mac_response(void)
{
	// put bytes into CAN chip msg object #7 and send
   // load CAN message config register - msg length = 7
	pokeb(CAN_BASE, 0x76, 0x78);

	// load data area of CAN chip
	pokeb(CAN_BASE, 0x77, 0x80);			// indicate response message
	pokeb(CAN_BASE, 0x78, LOBYTE(vendor_id));
	pokeb(CAN_BASE, 0x79, HIBYTE(vendor_id));
	pokeb(CAN_BASE, 0x7A, (UCHAR)(serial));
	pokeb(CAN_BASE, 0x7B, (UCHAR)(serial >> 8));
	pokeb(CAN_BASE, 0x7C, (UCHAR)(serial >> 16));
	pokeb(CAN_BASE, 0x7D, (UCHAR)(serial >> 24));

	pokeb(CAN_BASE, 0x71, 0x66);      // set msg object #7 transmit request
}


// Consume an incoming DUP MAC check message from another device 
int DEVICENET::consume_dup_mac(UCHAR request[])
{
	// If message is a response to our dup MAC ID check message
	// or if we are not on-line yet, error out.  Do not go on-line
	if (((request[0] & 0x80) != 0) ||      	// message is a response
		((global_status & ON_LINE) == 0))      // we are off line
	{
		global_status |= DUP_MAC_FAULT;        // set dup MAC error
		return NO_RESPONSE;               		// send no response
	}
	return OK;                                // message consumed successfully
}







//---------------------------------------------------------------------------------
// Router Class, class ID = 2
// It is used for the Router Object. 
//
// Class Attributes supported:      Revision
// Class Services supported: 		 	Get Attribute Single
// Instance Attributes supported:   None
// Instance Services supported:     None
//---------------------------------------------------------------------------------
class ROUTER
{
	private:
	static UINT class_revision;
	// address of objects the router will need to send messages to
	ANALOG_INPUT_POINT *temperature_sensor, *humidity_sensor;
	ASSEMBLY *assembly;
	IDENTITY *identity;
	DEVICENET *devicenet;
	CONNECTION *explicit, *io_poll;


	public:
	static void handle_class_inquiry(UCHAR*, UCHAR*);
	void handle_explicit(UCHAR*, UCHAR*);
	void route(UCHAR*, UCHAR*);
	ROUTER(ANALOG_INPUT_POINT *ts, ANALOG_INPUT_POINT *hs, IDENTITY *id,
			 DEVICENET *dn, CONNECTION *ex, CONNECTION *io, ASSEMBLY *as)
	{
      // Initialize address of other objects
		temperature_sensor = ts;
		humidity_sensor = hs;
		identity = id;
		devicenet = dn;
		explicit = ex;
		io_poll = io;
      assembly = as;
	}
};




// handle Explicit request to class
void ROUTER::handle_class_inquiry(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;

	service = request[1];
	attrib = request[4];
	error = 0;
	memset(response, 0, BUFSIZE);

	switch(service)
	{
		case GET_REQUEST:
		switch(attrib)
		{
			case 1:  // get revision attribute
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(class_revision);
			response[3] = HIBYTE(class_revision);
			response[LENGTH] = 4;
			break;

			default:
			error = ATTRIB_NOT_SUPPORTED;
			break;
		}
		break;


		default:
		error = SERVICE_NOT_SUPPORTED;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}



// Handles Explicit request to the Router Object
void ROUTER::handle_explicit(UCHAR request[], UCHAR response[])
{
	UINT service, attrib, error;

	service = request[1];
	attrib = request[4];
	error = 0;
	memset(response, 0, BUFSIZE);

	error = SERVICE_NOT_SUPPORTED;  // no services supported

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}




// Pass Explicit requests to appropriate object and get response
void ROUTER::route(UCHAR request[], UCHAR response[])
{
	UINT class_id, instance, error;

	class_id = request[2];
	instance = request[3];
	error = 0;
	memset(response, 0, BUFSIZE);

	switch(class_id)
	{
		case ANALOG_INPUT_POINT_CLASS:   
		switch(instance)
		{
			case 0:  // direct this to the class
			ANALOG_INPUT_POINT::handle_class_inquiry(request, response);
			break;

			case 1:  // direct this to instance 1
			temperature_sensor->handle_explicit(request, response);
			break;

			case 2:  // direct this to instance 2
			humidity_sensor->handle_explicit(request, response);
			break;

			default:
			error = OBJECT_DOES_NOT_EXIST;
			break;
		}
		break;


		case ASSEMBLY_CLASS:
		switch(instance)
		{
			case 0:  // direct this to the class
			ASSEMBLY::handle_class_inquiry(request, response);
			break;

			case 1:  // direct this to instance 1
			assembly->handle_explicit(request, response);
			break;

			default:
			error = OBJECT_DOES_NOT_EXIST;
			break;
		}
		break;



		case IDENTITY_CLASS:
		switch(instance)
		{
			case 0:     // direct this to the class
			IDENTITY::handle_class_inquiry(request, response);
			break;

			case 1:     // direct this to instance 1
			identity->handle_explicit(request, response);
			break;

			default:
			error = OBJECT_DOES_NOT_EXIST;
			break;
		}
		break;


		case DEVICENET_CLASS:
		switch(instance)
		{
			case 0:    // direct this to the class
			DEVICENET::handle_class_inquiry(request, response);
			break;

			case 1:    // direct this to instance 1
			devicenet->handle_explicit(request, response);
			break;

			default:
			error = OBJECT_DOES_NOT_EXIST;
			break;
		}
		break;


		case CONNECTION_CLASS:
		switch(instance)
		{
			case 0:    		// direct this to the class
			CONNECTION::handle_class_inquiry(request, response);
			break;

			case 1:        // direct this to instance 1
			explicit->handle_explicit(request, response);
         break;

			case 2:        // direct this to instance 2
			io_poll->handle_explicit(request, response);
			break;

			default:
			error = OBJECT_DOES_NOT_EXIST;
			break;
		}
		break;


		case ROUTER_CLASS:
		switch(instance)
		{
			case 0:		// direct this to the class
			handle_class_inquiry(request, response);
			break;

			case 1:     // direct this to instance 1
			handle_explicit(request, response);
			break;

			default:
			error = OBJECT_DOES_NOT_EXIST;
			break;

		}
		break;


		default:  // no such class
		error = OBJECT_DOES_NOT_EXIST;
		break;
	}

	if (error)
	{
		response[0] = request[0] & NON_FRAGMENTED;
		response[1] = ERROR_RESPONSE;
		response[2] = error;
		response[3] = NO_ADDITIONAL_CODE;
		response[LENGTH] = 4;
	}
}



//---------------------------------------------------------------------------------
// Initialize the Intel 82527 CAN controller.  It is mapped into memory
// space at base address 0xA0000.
// It's crystal frequency is 16.00MHz
// DSC = 1  so MCLK is 8 MHz which gives best access time
// SJW = 1  synchronization jump width
// SPL = 0  sample mode (single sample)
// TSEG1 = 12
// TSEG2 = 1
//---------------------------------------------------------------------------------
void init_can(UCHAR mac_id, UCHAR baud_rate)
{
	UCHAR brp, temp;
	UINT id;

	// Compute value of baud rate prescaler to load into CAN chip
   // Note: baud_rate = 0 is 125K, baud_rate = 1 is 250K, etc. 
	switch(baud_rate)
	{
		case 0: 	brp = 3; break;  // 125Kbps
		case 1: 	brp = 1; break;  // 250Kbps
		case 2: 	brp = 0; break;  // 500Kbps
		default: brp = 0; break;
	}

	pokeb(CAN_BASE, 0x00, 0x41);  			// control register
	pokeb(CAN_BASE, 0x1F, 0x30);				// clock out register
	pokeb(CAN_BASE, 0x01, 0x00);				// status register
	pokeb(CAN_BASE, 0x02, 0x40);				// CPU interface register
	pokeb(CAN_BASE, 0x2F, 0x00);				// bus configuration
	pokeb(CAN_BASE, 0x3F, (0x40 | brp)); 	// jump width & baud rate prescaler
	pokeb(CAN_BASE, 0x4F, 0x1C);				// sample mode & segment timing
	pokeb(CAN_BASE, 0x0C, 0xFF);         	// message 15 mask
	pokeb(CAN_BASE, 0x0D, 0xFF);         	// message 15 mask
	pokeb(CAN_BASE, 0x0E, 0xFF);         	// message 15 mask
	pokeb(CAN_BASE, 0x0F, 0xFF);         	// message 15 mask
	pokeb(CAN_BASE, 0x06, 0xFF);         	// global mask
	pokeb(CAN_BASE, 0x07, 0xFF);         	// global mask
	pokeb(CAN_BASE, 0x08, 0xFF);         	// global mask
	pokeb(CAN_BASE, 0x09, 0xFF);         	// global mask
	pokeb(CAN_BASE, 0x0A, 0xFF);         	// global mask
	pokeb(CAN_BASE, 0x0B, 0xFF);         	// global mask

   // Setup registers for message objects
	// Message objects are assigned in order of the priority I wanted
   // Highest priority is message object 1 which is I/O Poll request
   temp = GROUP2 | mac_id;       		// Object 1 - Master's I/O Poll request (rcve)
	pokeb(CAN_BASE, 0x10, 0x55);     	// DeviceNet Message ID = 5
	pokeb(CAN_BASE, 0x11, 0x55);
	pokeb(CAN_BASE, 0x12, temp);
	pokeb(CAN_BASE, 0x13, 0xA0);
	pokeb(CAN_BASE, 0x14, 0x00);
	pokeb(CAN_BASE, 0x15, 0x00);
	pokeb(CAN_BASE, 0x16, 0x10);
	pokeb(CAN_BASE, 0x10, 0x99);
   
	temp = GROUP2 | mac_id;       		// Object 2 -  Master's Explicit request (rcve)
	pokeb(CAN_BASE, 0x20, 0x55);     	// DeviceNet Message ID = 4
	pokeb(CAN_BASE, 0x21, 0x55);
	pokeb(CAN_BASE, 0x22, temp);
	pokeb(CAN_BASE, 0x23, 0x80);
	pokeb(CAN_BASE, 0x24, 0x00);
	pokeb(CAN_BASE, 0x25, 0x00);
	pokeb(CAN_BASE, 0x26, 0x10);
	pokeb(CAN_BASE, 0x20, 0x99);
   
	temp = GROUP2 | mac_id;       		// Object 3 - DUP MAC ID chk (rcve)
	pokeb(CAN_BASE, 0x30, 0x55);     	// DeviceNet Message ID = 7
	pokeb(CAN_BASE, 0x31, 0x55);
	pokeb(CAN_BASE, 0x32, temp);
	pokeb(CAN_BASE, 0x33, 0xE0);
	pokeb(CAN_BASE, 0x34, 0x00);
	pokeb(CAN_BASE, 0x35, 0x00);
	pokeb(CAN_BASE, 0x36, 0x10);
	pokeb(CAN_BASE, 0x30, 0x99);
   		
	temp = GROUP2 | mac_id;        		// Object 4 - Unconnected port request (rcve)
	pokeb(CAN_BASE, 0x40, 0x55);      	// DeviceNet Message ID = 6
	pokeb(CAN_BASE, 0x41, 0x55);
	pokeb(CAN_BASE, 0x42, temp);
	pokeb(CAN_BASE, 0x43, 0xC0);
	pokeb(CAN_BASE, 0x44, 0x00);
	pokeb(CAN_BASE, 0x45, 0x00);
	pokeb(CAN_BASE, 0x46, 0x10);
	pokeb(CAN_BASE, 0x40, 0x99);
   
	id = (mac_id & 0x07);         		// Object 5 - Slave's I/O Poll response (xmit)
	id = id << 13;                		// DeviceNet Message ID = 15 (Group 1 message)
	id |= (GROUP1 | 0x38);
	id |= (mac_id >> 3);
	pokeb(CAN_BASE, 0x50, 0x55);
	pokeb(CAN_BASE, 0x51, 0x55);
	pokeb(CAN_BASE, 0x52, LOBYTE(id));
	pokeb(CAN_BASE, 0x53, HIBYTE(id));
	pokeb(CAN_BASE, 0x54, 0x00);
	pokeb(CAN_BASE, 0x55, 0x00);
	pokeb(CAN_BASE, 0x56, 0x18);
	pokeb(CAN_BASE, 0x50, 0x95);
      
	temp = GROUP2 | mac_id;     			// Object 6 - Slave's Explicit response (xmit)
	pokeb(CAN_BASE, 0x60, 0x55);     	// DeviceNet Message ID = 3
	pokeb(CAN_BASE, 0x61, 0x55);
	pokeb(CAN_BASE, 0x62, temp);
	pokeb(CAN_BASE, 0x63, 0x60);
	pokeb(CAN_BASE, 0x64, 0x00);
	pokeb(CAN_BASE, 0x65, 0x00);
	pokeb(CAN_BASE, 0x66, 0x18);
	pokeb(CAN_BASE, 0x60, 0x95);
   
   temp = GROUP2 | mac_id;       		// Object 7 - DUP MAC ID check (xmit)
	pokeb(CAN_BASE, 0x70, 0x55);      	// DeviceNet Message ID = 7
	pokeb(CAN_BASE, 0x71, 0x55);
	pokeb(CAN_BASE, 0x72, temp);
	pokeb(CAN_BASE, 0x73, 0xE0);
	pokeb(CAN_BASE, 0x74, 0x00);
	pokeb(CAN_BASE, 0x75, 0x00);
	pokeb(CAN_BASE, 0x76, 0x18);
	pokeb(CAN_BASE, 0x70, 0x95);
   	
   pokeb(CAN_BASE, 0x80, 0x55);    		// Object 8 - not used
	pokeb(CAN_BASE, 0x81, 0x55);
   
	pokeb(CAN_BASE, 0x90, 0x55);    		// Object 9 - not used
	pokeb(CAN_BASE, 0x91, 0x55);
   
	pokeb(CAN_BASE, 0xA0, 0x55);    		// Object 10 - not used
	pokeb(CAN_BASE, 0xA1, 0x55);

	pokeb(CAN_BASE, 0xB0, 0x55);    		// Object 11 - not used
	pokeb(CAN_BASE, 0xB1, 0x55);

	pokeb(CAN_BASE, 0xC0, 0x55);    		// Object 12 - not used
	pokeb(CAN_BASE, 0xC1, 0x55);

	pokeb(CAN_BASE, 0xD0, 0x55);    		// Object 13 - not used
	pokeb(CAN_BASE, 0xD1, 0x55);

	pokeb(CAN_BASE, 0xE0, 0x55);    		// Object 14 - not used
	pokeb(CAN_BASE, 0xE1, 0x55);

	pokeb(CAN_BASE, 0xF0, 0x55);    		// Object 15 - not used
	pokeb(CAN_BASE, 0xF1, 0x55);

	// now allow the CAN to send & rcve messages
	pokeb(CAN_BASE, 0x00, 0x02);			// CAN control - set IE, clear INIT

}




// Initialize static data (at file scope). These are the rev levels
// of the objects as specified in the DeviceNet spec release II.
UINT ANALOG_INPUT_POINT::class_revision = 2;
UINT ASSEMBLY::class_revision = 2;
UINT IDENTITY::class_revision = 1;
UINT DEVICENET::class_revision = 2;
UINT CONNECTION::class_revision = 1;
UINT ROUTER::class_revision = 1;

// Initialize static data - path to the assembly object
UCHAR CONNECTION::path[10] =  {0x20, 0x04, 0x24, 0x01, 0x30, 0x03};



//----------------------------------------------------------------------------------
void main(void)
{
	UCHAR i, k, length, temp;     // Variables local to main()
	UCHAR request[BUFSIZE];
	UCHAR response[BUFSIZE];
	UCHAR state;
	UINT e;
	UCHAR mask, test;
	UCHAR mac_id;
	UCHAR	baud_rate;
	UINT  vendor_id = 0x1234;		// dummy value
	ULONG serial_number = 0x2345;	// dummy value

	disable();            			// disable interrupts during initialization

	
	// Initialize global variables and Timer values
	memset(global_CAN_buf, 0, BUFSIZE);
	global_timer[EXPLICIT] = 0;        	// explicit connection timer
	global_timer[IO_POLL] = 0;         	// io poll connection timer
	global_timer[ACK_WAIT] = 0;         // only used for sending fragmented msg
	global_timer[UPDATE] = 10;          // do first device update in 0.5 secs
	global_event = 0;                   // no events to service
	global_status = SELF_TESTING; 		// off-line at this point


	// Setup the 82C55A parallel I/O port on the DeviceNet board
	pokeb(PIO_BASE, CONTROL, 0x83);	// Port A = OUT,    B = IN,
												// Port C(lo) = IN, C(hi) = OUT

   pokeb(PIO_BASE, PORTC, 0xF0);		// LEDs off

   // Set CS5501 A/D chip select HI, and MUX WR line HI
	// This also starts self-calibration of A/D by taking CAL low
	// Self calibration cycle takes 1.2 - 1.3 seconds
	pokeb(PIO_BASE, PORTA, 0x42);
		
	temp = peekb(PIO_BASE, PORTC); 	// read baud rate switch
	temp = ~temp;           	     	// invert bits
	baud_rate = temp & 0x0F;   		// want lower 4 bits only


	// set up port 2 on the 82527 and read MAC ID switches
	pokeb(CAN_BASE, 0xAF, 0x00);	// P2 config
	temp = peekb(CAN_BASE, 0xCF); // read MAC ID switches
	temp = ~temp;
	// convert BCD switch values to decimal number
	mac_id = 10 * ((temp & 0xF0) >> 4) + (temp & 0x0F);
   
	init_can(mac_id, baud_rate);  // setup 82527 CAN chip registers


	mask = inp(0x21);             // Get 80386EX interrupt mask
	mask = mask & 0xDF;
	outp(0x21, mask); 				// unmask IRQ5 (used for incoming messages) 
	setvect(0x0D, can_isr); 		// make INT 0D (IRQ 5) call my CAN ISR
	setvect(0x1C, timer);         // make INT 1C (user clock) call my timer ISR
 

	// Now create objects
	// For some objects, pass the address of other objects to allow
	// referencing functions in those other objects
	ANALOG_INPUT_POINT temperature_sensor(TEMPERATURE);
	ANALOG_INPUT_POINT humidity_sensor(HUMIDITY);
	CONNECTION explicit(EXPLICIT);
	CONNECTION io_poll(IO_POLL);
	DEVICENET devicenet(mac_id, baud_rate, vendor_id, serial_number, &explicit, &io_poll);
	IDENTITY identity(vendor_id, serial_number, &explicit, &io_poll);
	ASSEMBLY assembly(&identity, &temperature_sensor, &humidity_sensor);
	ROUTER router(&temperature_sensor, &humidity_sensor, &identity,
	 				  &devicenet, &explicit, &io_poll, &assembly);


	// Adjust MicroSys board BIOS clock ticks to 20.00Hz by setting TIMER0
	// TIMER0 initial settings are: Mode 3, write LSB then MSB, 16 bit binary
 	outp(0x40, 0x84);  		// counter LSB
	outp(0x40, 0xE8);			// counter MSB

	// Enable 386EX watchdog Timer
	// Timer clocked at 25MHz, so 17D8000H gives a timeout time of 1.0 secs
	outpw(WDTRLDH, 0x017D);    // set high word
	outpw(WDTRLDL, 0x8000);    // set low word
	outpw(WDTCLR,  0xF01E);    // enable watchdog
	outpw(WDTCLR,  0x0FE1);

	enable();            		// enable interrupts



	while (1)            			// main loop - do forever
	{
		outpw(WDTCLR, 0xF01E);     // Strobe the 386EX watchdog.  Must do this
		outpw(WDTCLR, 0x0FE1);     // at least once per second

		// If a bit in "global_event" is set, it indicates that a task needs to
		// be done. Lower order bits have higher priority.  If an event bit is
		// set and no higher priority events bits exist, the corresponding task
		// is done. An incoming I/O Poll request has the highest priority.
		// The temporary variable "e" is used to prevent interrupt interference.
		// Note:  It is not necessary to disable interrupts when clearing a bit
		// in global_event because it is done with a single machine instruction.


		// Incoming I/O Poll request message
		e = global_event;
		if ((e & IO_POLL_REQUEST) && !(e & (IO_POLL_REQUEST - 1)))
		{
			global_event &= ~IO_POLL_REQUEST; // clear bit
			// must be online with no critical errors
			if ((global_status & ON_LINE) && !(global_status & NETWORK_FAULT))
			{
				// Get request message from global ISR buffer
				disable();
				memcpy(request, global_CAN_buf, BUFSIZE);
				enable();
            if (io_poll.link_consumer(request))  	// try to consume the request
				{
				  	assembly.handle_io_poll_request(request, response);
					io_poll.link_producer(response); 	// produce response
				}
			}
		}


		// Incoming Explicit request message
		e = global_event;
		if ((e & EXPLICIT_REQUEST) && !(e & (EXPLICIT_REQUEST - 1)))
		{
			global_event &= ~EXPLICIT_REQUEST; // clear bit
			// must be online with no critical errors
			if ((global_status & ON_LINE) && !(global_status & NETWORK_FAULT))
			{
				// make sure message is from my master
				if ((global_CAN_buf[0] & 0x3F) == devicenet.allocation.my_master)
				{
					// Get request message from global ISR buffer
					disable();
					memcpy(request, global_CAN_buf, BUFSIZE);
					enable();
					if (explicit.link_consumer(request)) 	// try to consume the request
					{
						router.route(request, response);  	// route request & get response
						explicit.link_producer(response);   // produce response
					}
				}
			}
		}

      		
      // Incoming DUP MAC check message (from another device)
		e = global_event;
		if ((e & DUP_MAC_REQUEST) && !(e & (DUP_MAC_REQUEST - 1)))
		{
			global_event &= ~DUP_MAC_REQUEST;		// clear bit
			// received a dup MAC ID check from another device !
			// Get dup mac message from global ISR buffer
			disable();
			memcpy(request, global_CAN_buf, BUFSIZE);
			enable();
			if (devicenet.consume_dup_mac(request))
			{
				// This response keeps offending device from going on-line
				devicenet.send_dup_mac_response();
			}
		}

		
      // Incoming message to the Unconnected Port 
		e = global_event;
		if ((e & UNC_PORT_REQUEST) && !(e & (UNC_PORT_REQUEST - 1)))
		{
			global_event &= ~UNC_PORT_REQUEST;				// clear bit
			// must be online with no critical errors
			if ((global_status & ON_LINE) && !(global_status & NETWORK_FAULT))
			{
				// Get request message from global ISR buffer
				disable();
				memcpy(request, global_CAN_buf, BUFSIZE);
				enable();
				if (devicenet.handle_unconnected_port(request, response));
            {
					// Note: I am not using link producer to send response because
					// the Unconnected port should have its own direct connection to
					// the network - So load response directly into can chip object #3
            	length = response[LENGTH];
					for (i=0; i < length; i++)  						// load data into CAN
					{
						pokeb(CAN_BASE, (0x67 + i), response[i]);
					}
					pokeb(CAN_BASE, 0x66, ((length << 4) | 0x08));	// load config resister
					pokeb(CAN_BASE, 0x61, 0x66);      					// set transmit request
         	}
	      }
		}

     
      // Timeout of the Explicit connection
		e = global_event;
		if ((e & EXPLICIT_TIMEOUT) && !(e & (EXPLICIT_TIMEOUT - 1)))
		{
			global_event &= ~EXPLICIT_TIMEOUT;	// clear bit
			devicenet.handle_timeout(EXPLICIT_CONXN);
		}


      // Timeout of the I/O Poll connection
		e = global_event;
		if ((e & IO_POLL_TIMEOUT) && !(e & (IO_POLL_TIMEOUT - 1)))
		{
			global_event &= ~IO_POLL_TIMEOUT;	// clear bit
			devicenet.handle_timeout(IO_POLL_CONXN);
		}

		
		// Do self-test, update device status & LEDs, update analog sensors
		// Happens every 0.25 second.  The main reason 0.25 sec was chosen
      // is that the LEDs green/red flash time during self-test is 0.25 sec 
		e = global_event;
		if ((e & DEVICE_UPDATE) && !(e & (DEVICE_UPDATE - 1)))
		{
			global_event &= ~DEVICE_UPDATE; 		// clear bit
			global_timer[UPDATE] = 5;				// reload timer to 0.25 secs
			identity.update_device();
			temperature_sensor.compute_value();
         humidity_sensor.compute_value();
			assembly.update_data();
		}

		      		
      // Timeout waiting for an acknowlege message(to a fragment I sent)
		e = global_event;
		if ((e & ACK_WAIT_TIMEOUT) && !(e & (ACK_WAIT_TIMEOUT - 1)))
		{
			global_event &= ~ACK_WAIT_TIMEOUT;
			// Notify link producer that we have timed-out while waiting for an ACK
			request[MESSAGE_TAG] = ACK_TIMEOUT;
			explicit.link_producer(request);
		}


      // Perform a full reset of the device
		e = global_event;
		if ((e & FULL_RESET) && !(e & (FULL_RESET - 1)))
		{
			global_event &= ~FULL_RESET;
			// Go into loop and let watchdog reset the device
			while (1);
		}
	}
}



*/