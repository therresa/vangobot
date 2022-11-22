/*
Group members: Eric Aleong, Owen Leather, Nush Majra, Theresa Nguyen
Group 4-17
Version: 1.0
Description: Functions written in ROBOTC for VanGoBot to be deployed
on EV3 brick.
*/
#pragma SystemFile

// select EV3 set of functions
#define _EV3FILEIO 1

#include "PC_FileIO.c"

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
bool readNextCommand(TFileHandle &fin, struct MotorCommand &motorCommand);
void manualMove();
void convertFileXYToPaperXY(float autoX, float autoY, float size, struct MotorCommand &motorCommand)
void automaticMode(TFileHandle &fin, float x, float y, float size);
void automaticModeMenu();
void mainMenu();
void configureSensors();

task main()
{

}

void movePen(int xPower, int yPower){
	motor[motorA] = xPower;
	motor[motorB] = yPower;
}

void autoMovePen(float x, float y){

}

void manualMove(){
	while(getButtonPress(buttonBack)){
		if(getButtonPress(buttonUp)){
			movePen(0, 50);
		}
		else if(getButtonPress(buttonDown)){
			movePen(0, -50);
		}
		else if(getButtonPress(buttonLeft)){
			movePen(-50, 0);
		}
		else if(getButtonPress(buttonRight)){
			movePen(50, 0);
		}
		else{
			movePen(0, 0);
		}
	}
}

void home(){
		movePen(50, 0);
		while(!SensorValue[S1]) {}
		movePen(0, 50);
		while(SensorValue[S2] > Y_AXIS_HOME_DISTANCE){}
		movePen(0, 0);
}

bool readNextCommand(TFileHandle &fin, struct MotorCommand &motorCommand){
	int liftPen;
	float x;
	float y;
	readFloatPC(fin, x);
	readFloatPC(fin, y);
	readIntPc(fin, liftPen);
	motorCommand.x = x;
	motorCommand.y = y;
	motorCommand.liftPen = liftPen==1;
}

void automaticMode(TFileHandle &fin, float x, float y, float size){
		struct MotorCommand motorCommand;
		while(!getButtonPress(BACK_BUTTON) && readNextCommand(fin, motorCommand)){
				convertFileXYToPaperXY(x, y, size, motorCommand);
				liftLowerPen(motorCommand.liftPen);
				autoMovePen(motorCommand.x, motorCommand.y);
		}
		movePen(0, 0);
}

void configureSensors()
{
	SensorType[S1] = sensorEV3_Touch;
  wait1Msec(50);
  SensorType[S2] = sensorEV3_Ultrasonic;
  wait1Msec(50);
}

void mainMenu()
{
	//Title
	displayBigTextLine(1, "VanGoBot");
	displayTextLine(3, "Select a drawing mode:");

	//Options
	displayTextLine(5, "Manual");
	displayTextLine(7, "File Print");

	//Display rect around the selected option
	//drawRect()
}
