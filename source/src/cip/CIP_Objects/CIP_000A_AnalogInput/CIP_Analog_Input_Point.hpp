#ifndef CIP_CLASSES_ANALOGINPUTPOINT_H
#define CIP_CLASSES_ANALOGINPUTPOINT_H

// Based on Jim Brady's work http://jbrady.com/devicenet.html
//
// Analog Input Point Class, class ID = 0x0A
// It is used for the Temperature and Humidity Sensor Objects.
// I extended it to handle A/D conversion and computation for the
// Weather Station sensors.
//
// Class Attributes supported:      Revision
// Class Services supported: 		 	Get Attribute Single
// Instance Attributes supported:   Value, Status, Data Type
// Instance Services supported:     Get Attribute Single




#include <cip/CIP_Objects/template/CIP_Object_template.hpp>

class CIP_AnalogInputPoint;
class CIP_AnalogInputPoint : public CIP_Object_template<CIP_AnalogInputPoint>
{
private:
	unsigned char value;
	bool status;
	unsigned char data_type;
	unsigned char instance;
	unsigned long long sensor_clock;


public:
	static CipStatus Init();
	static CipStatus Shut();
	static void handle_class_inquiry(unsigned char*, unsigned char*);
	void handle_explicit(unsigned char*, unsigned char*);
	unsigned char get_value(void);
	CIP_AnalogInputPoint(unsigned char inst)  // constructor
	{
		instance = inst;
		sensor_clock = 0;
		value = 0;
		status = 0;		        // no alarms or faults
		data_type = 2;         // indicates that value is an unsigned char
	}
};

#endif //CIP_CLASSES_ANALOGINPUTPOINT_H