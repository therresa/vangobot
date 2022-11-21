/*
Group members: Eric Aleong, Owen Leather, Nush Majra, Theresa Nguyen
Group 4-17
Version: 1.0
Description: Functions written in ROBOTC for VanGoBot to be deployed
on EV3 brick.
*/

//Global constants
const float ENCODER_TO_INCH = 0, PEN_UP = 0, PEN_DOWN = 0, GANTRY_KP = 0,
PEN_KP = 0, Y_AXIS_HOME_DISTANCE = 0;

//MotorCommand struct
struct MotorCommand {
    float x;
    float y;
    bool liftPen;
}

//Prototypes
void movePen(int xPower, int yPower);
void autoMovePen(float x, float y);
void liftLowerPen(bool lifted);
void home();
MotorCommand readNextCommand(TFileHandle &fileIn);
void manualMove();
void automaticMode(TFileHandle &fileIn, float x, float y, float size);
void automaticModeMenu();
void mainMenu();
void configureSensors();

task main()
{

}

void configureSensors()
{
	SensorType[S1] = sensorEV3_Touch;
  wait1Msec(50);
  SensorType[S2] = sensorEV3_Ultrasonic;
  wait1Msec(50);
}
