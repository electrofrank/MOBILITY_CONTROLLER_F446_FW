/*
*******************************************************************************
  * @file    Odrive_CanIf.h 
  * @author  (.)(.)
  * @brief   Header to collect all the CanBus ID's and specs for Odrive interface by edovlo
  ******************************************************************************
  * @attention  READ THE FUCKING DOCUMENTATION OF THE ODRIVE !!!!!
******************************************************************************
*/

/*
            FRAME DESCRIPTION:

    - Classic: 11 bit identifier (used by default)
        --  upper 6 bits: Node ID, 0 to 0x3F
        -- lower 5 bits: Command ID, 0 to 0x1F
    - Extended: 29 bit identifier (NOT USED for now)

    Complete ID:
        nodeID << 5 | (CommandID & 0x1F)

    Example: ID creation
        nodeID = 0x10,  allows ID's from 0x200 to 0x21F
        commandID = 0x0D

        tot_ID = nodeID << 5 | (commandID & 0x1F)  ===>  0x10 << 5 | (0x0D & 0x1F)
        tot_ID = 0x200 | 0x0D  ======>  0x20D 

*/

#ifndef __ODRIVE_CANIF_H
#define __ODRIVE_CANIF_H

// Include
#include <stdint.h>

// Odrive ID's
#define Odrive_High_Left 		0x1     // front left
#define Odrive_High_Right 		0x2     // front right
#define Odrive_Middel_Left 		0x3     // middle left
#define Odrive_Middle_Right 	0x4     // middle right
#define Odrive_Low_Left 		0x5     // rear left
#define Odrive_Low_Right 		0x6     // rear right

// Baud define
#define CAN_Baud_125  			/*default*/
#define CAN_Baud_250   
#define CAN_Baud_500
#define CAN_Baud_1000

                                /*  Command ID Description  */
/*  No response command   */
#define ODR_CanOpen_NMT 0x000                       // not used in classic Can
#define ODR_CanOpen_Heartbeat 0x700                 // not used in classin Can

#define ODR_HeartBeat 0x001                         //data: unsigned int
#define ODR_Estop 0x002
#define ODR_Set_AxisID 0x006                        //data: unsigned int
#define ODR_Set_AxisReqState 0x007                  //data: unsigned int
#define ODR_Set_Axis_StartupConf 0x008              // NOT IMPLEMENTED
#define ODR_Set_ControllerModes 0x00B               //data: unsigned int
#define ODR_Set_InputPos 0x00C                      //data: IEEE 754 float
#define ODR_Set_InputVel 0x00D                      //data: IEEE 754 float
#define ODR_Set_InputTorque 0x00E                   //data: IEEE 754 float
#define ODR_Set_VelocityLimit 0x00F                 //data: IEEE 754 float
#define ODR_StartAnticogging 0x010
#define ODR_Set_TrajVelLimit 0x011                  //data: IEEE 754 float
#define ODR_Set_TrajAccelLimit 0x012                //data: IEEE 754 float
#define ODR_Set_TrajInertia 0x013                   //data: IEEE 754 float
#define ODR_Clear_Errors 0x018 
#define ODR_Reboot 0x016

/*  Call & Response command */
#define ODR_Get_MotorERR 0x003                          // reply with Unsigned
#define ODR_Get_EncoderERR 0x004                        // reply with Unsigned
#define ODR_Get_SensoressERR 0x005                      // reply with Unsigned
#define ODR_Get_EncoderEstimate 0x009                   // reply with IEEE 754 float
#define ODR_Get_EncoderCount 0x00A                      // reply with Int
#define ODR_Get_IQ 0x014                                // reply with IEEE 754 float
#define ODR_Get_SensorlessEstimate 0x015                // reply with IEEE 754 float
#define ODR_Get_VbusVoltage 0x017                       // reply with IEEE 754 float

/*  Functions    */
uint16_t CanID_Generate(uint8_t Axis_ID, uint8_t CMD);
float BytesToFloat_32(uint8_t* Vect);
void Float_32ToBytes(float val, uint8_t* out_8b_vect);

#endif  /*__ODRIVE_CANIF_H*/
