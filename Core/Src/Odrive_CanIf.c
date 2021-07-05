#include "Odrive_CanIf.h"

uint16_t CanID_Generate(uint8_t Axis_ID, uint8_t CMD)
{
	return Axis_ID << 5 | CMD;
}

// float_val = *(float*)r					// 4 bytes vector to float
float BytesToFloat_32(uint8_t* Vect)
{
	return *(float*)Vect;
}

// *(float*)r = Vel;     			       // float to 4 bytes vector
void Float_32ToBytes(float val, uint8_t* out_8b_vect)
{
	*(float*)out_8b_vect = val;
}
