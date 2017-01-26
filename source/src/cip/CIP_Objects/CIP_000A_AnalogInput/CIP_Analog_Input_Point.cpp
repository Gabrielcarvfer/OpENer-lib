//Includes
#include "CIP_Analog_Input_Point.hpp"

//Variables
//CipUdint CIP_AnalogInputPoint::class_id = 0x0A;

//Methods
CipStatus CIP_AnalogInputPoint::Init()
{

}

// Handle Explicit requests to the class
void CIP_AnalogInputPoint::handle_class_inquiry(unsigned char request[], unsigned char response[])
{
	unsigned int service, attrib, error;

	service = request[1];
	attrib = request[4];
	error = 0;
	/*
	memset(response, 0, BUFSIZE);

	switch (service)
	{
	case GET_REQUEST:
		switch (attrib)
		{
		case 1:  // get revision attribute
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = LOBYTE(class_revision);
			response[3] = HIBYTE(class_revision);
			response[LENGTH] = 4;
			break;

		default:
			error = kCipErrorAttributeNotSupported; 
			break;
		}
		break;


	default: 
		error = kCipErrorServiceNotSupported;
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
	*/
}




// Handles Explicit request to Analog Input Point Object
void CIP_AnalogInputPoint::handle_explicit(unsigned char request[], unsigned char response[])
{
	unsigned int service, attrib, error;

	service = request[1];
	attrib = request[4];
	error = 0;
	/*
	memset(response, 0, BUFSIZE);
	
	switch (service)
	{
	case GET_REQUEST:
		switch (attrib)
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
			response[2] = (unsigned char)status;
			response[LENGTH] = 3;
			break;

		case 8:  // data type
			response[0] = request[0] & NON_FRAGMENTED;
			response[1] = service | SUCCESS_RESPONSE;
			response[2] = data_type;
			response[LENGTH] = 3;
			break;

		default:
			error = kCipErrorAttributeNotSupported;
			break;
		}
		break;

	default:
		error = kCipErrorServiceNotSupported;
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
	*/
}



// Returns the sensor's value
unsigned char CIP_AnalogInputPoint::get_value(void)
{
	return value;
}







