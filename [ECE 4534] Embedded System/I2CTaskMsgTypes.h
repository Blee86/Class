#ifndef I2CTASK_MSG_TYPES_H
#define I2CTASK_MSG_TYPES_H

// Here is where I define the types of the messages that I am passing to the I2C task
//   --Note that none of these message types (as I have implemented this) actually go over the I2C bus, but they
//     are useful for matching up what is send to/from the I2C task message queues
//
// I have defined them all here so that they are unique
#define I2CARMInit 0
#define TimerUpdateSensor 1
#define TimerUpdateSensorRequested 2
#define TimerReadSensor 3
#define I2CReadData 4
#define I2CHeartBeat 5
#define I2CRoverCommand 6
#define I2CButtonITR 10
#define I2CRoverEmergency 7
#define I2CJoystick 8
#define TimerStateMachine 9
#endif