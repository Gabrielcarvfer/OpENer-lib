//---------------------------------------------------------
//        Header File for DeviceNet Weather Station program
//        Jim Brady		July 1998
//
//
//---------------------------------------------------------



// Define types
typedef int						BOOL;
typedef unsigned char 		UCHAR;
typedef unsigned int			UINT;
typedef unsigned long		ULONG;

// Define Macros
#define LOBYTE(w)				((UCHAR)(w))
#define HIBYTE(w)				((UCHAR)((int)(w) >> 8))

// Base addresses of segment register
const UINT  CAN_BASE = 0xA000;   	 // Base segment of 82527 CAN chip
const UINT  PIO_BASE = 0xA100;       // BAse segment of 82C55 PIO chip

// Define constants
#define FALSE					0
#define TRUE					1
#define OK						1
#define MATCH					0
#define NO_RESPONSE			0
#define BUFSIZE				80    			// size of message buffers
#define LENGTH 				BUFSIZE - 1		// this location holds length of message
#define MESSAGE_TAG			BUFSIZE - 2    // this location holds a message tag


// MicroSys Board 386EX addresses
#define WDTRLDH					0xF4C0
#define WDTRLDL					0xF4C2
#define WDTCLR						0xF4C8


// Address offsets for the 82C55A I/O chip
#define PORTA						0x00
#define PORTB						0x01
#define PORTC						0x02
#define CONTROL					0x03


// define bit positions for global status word
#define OWNED						0x0001
#define ON_LINE					0x0002
#define CONFIGURED            0x0004
#define SELF_TESTING				0x0008
#define OPERATIONAL				0x0010
#define DEVICE_FAULT				0x0800
#define DUP_MAC_FAULT			0x1000
#define BUS_OFF					0x2000
#define NETWORK_FAULT	  		0x3000
#define LONELY_NODE				0x4000



// define bit positions for global event word
#define IO_POLL_REQUEST			0x0001
#define EXPLICIT_REQUEST 		0x0002
#define DUP_MAC_REQUEST			0x0004
#define UNC_PORT_REQUEST		0x0008
#define EXPLICIT_TIMEOUT		0x0040
#define IO_POLL_TIMEOUT			0x0080
#define DEVICE_UPDATE			0x0200
#define ACK_WAIT_TIMEOUT		0x2000
#define FULL_RESET				0x8000



// CAN controller message objects
#define EXPLICIT_RESPONSE_OBJ		3
#define EXPLICIT_REQUEST_OBJ		4
#define IO_POLL_REQUEST_OBJ		5
#define UNC_PORT_REQUEST_OBJ		6
#define DUP_MAC_OUTGOING_OBJ		7
#define DUP_MAC_INCOMING_OBJ		8
#define IO_POLL_RESPONSE_OBJ	   9



// DeviceNet error codes
#define  RESOURCE_UNAVAILABLE  		0x02
#define  SERVICE_NOT_SUPPORTED 	 	0x08
#define  INVALID_ATTRIB_VALUE  	 	0x09
#define  ALREADY_IN_STATE 	 		 	0x0B
#define  OBJECT_STATE_CONFLICT 	 	0x0C
#define  ATTRIB_NOT_SETTABLE 		 	0x0E
#define  PRIVILEGE_VIOLATION 		 	0x0F
#define  DEVICE_STATE_CONFLICT 	 	0x10
#define  REPLY_DATA_TOO_LARGE 	 	0x11
#define  NOT_ENOUGH_DATA 			 	0x13
#define  ATTRIB_NOT_SUPPORTED 	 	0x14
#define  TOO_MUCH_DATA 				 	0x15
#define  OBJECT_DOES_NOT_EXIST 	 	0x16
#define  NO_STORED_ATTRIB_DATA 	 	0x18
#define  STORE_OP_FAILURE			 	0x19
#define  INVALID_PARAMETER 		 	0x20
#define  INVALID_MEMBER_ID			 	0x28
#define  MEMBER_NOT_SETTABLE   	 	0x29


// DeviceNet additional error codes (object specific)
#define ALLOCATION_CONFLICT			0x01
#define INVALID_ALLOC_CHOICE			0x02
#define INVALID_UNC_REQUEST			0x03


// DeviceNet service codes
#define  RESET_REQUEST 				 	0x05
#define  START_REQUEST 				 	0x06
#define  STOP_REQUEST 				 	0x07
#define  CREATE_REQUEST 			 	0x08
#define  DELETE_REQUEST 			 	0x09
#define  GET_REQUEST 				 	0x0E
#define  SET_REQUEST 				 	0x10
#define  RESTORE_REQUEST			 	0x15
#define  SAVE_REQUEST 				 	0x16
#define	ALLOCATE_CONNECTIONS			0x4B
#define  RELEASE_CONNECTIONS			0x4C

// Class IDs
#define  IDENTITY_CLASS				 	0x01
#define  ROUTER_CLASS				 	0x02
#define  DEVICENET_CLASS			 	0x03
#define  ASSEMBLY_CLASS				 	0x04
#define  CONNECTION_CLASS			 	0x05
#define  REGISTER_CLASS				 	0x07
#define  DIP_CLASS					 	0x08
#define  DOP_CLASS					 	0x09
#define  ANALOG_INPUT_POINT_CLASS	0x0A
#define  PARAMETER_CLASS			 	0x0F
#define  ACK_HANDLER_CLASS			 	0x2B
#define  DEV_SUPERVISOR_CLASS		 	0x30
#define  ANALOG_SENSOR_CLASS		 	0x31
#define	ANALOG_ACTUATOR_CLASS		0x32


// Define Instance IDs and TIMER numbers
#define  EXPLICIT							0x01
#define  IO_POLL							0x02
#define  BIT_STROBE						0x03
#define  COS_CYCLIC						0x04
#define  ACK_WAIT							0x05
#define  UPDATE							0x06

// Define instance IDs of Analog Input Point Objects
#define TEMPERATURE						0x01
#define HUMIDITY							0x02


// define message tags which can be used to identify message
#define  RECEIVED_ACK					0x01
#define  SEND_ACK							0x02
#define  ACK_TIMEOUT						0x03
#define	ACK_ERROR						0x04


// define connection states
#define NON_EXISTENT						0x00
#define CONFIGURING						0x01
#define WAITING_FOR						0x02
#define ESTABLISHED						0x03
#define TIMED_OUT							0x04
#define DEFERRED							0x05



// Define bit positions in Allocation Choice attribute
#define	EXPLICIT_CONXN					0x01
#define  IO_POLL_CONXN					0x02
#define 	BIT_STROBE_CONXN				0x04
#define  COS_CONXN						0x10
#define  CYCLIC_CONXN					0x20


//Define message fragment values
#define  FIRST_FRAG						0x00
#define  MIDDLE_FRAG						0x40
#define  LAST_FRAG						0x80
#define  ACK_FRAG							0xC0


// Additional #defines
#define  SUCCESS_RESPONSE			 	0x80
#define 	RESPONSE_FLG					0x80
#define 	REQUEST_FLG						0x00
#define  ERROR_RESPONSE 			 	0x94
#define  NON_FRAGMENTED				 	0x7F
#define  SEND_NEXT_FRAG					0xFF
#define  NO_ADDITIONAL_CODE 		 	0xFF
#define  GROUP1							0x40
#define  GROUP2							0x80
#define	EXPLICIT_TYPE					0x00
#define  IO_TYPE							0x01
#define  DEFAULT_MASTER_MAC_ID		0xFF




