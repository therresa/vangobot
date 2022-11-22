/*
Group members: Eric Aleong, Owen Leather, Nush Majra, Theresa Nguyen
Group 4-17
Version: 1.0
Description: Functions written in ROBOTC for VanGoBot to be deployed
on EV3 brick.
Drain gang
*/
#pragma SystemFile

// select EV3 set of functions
#define _EV3FILEIO 1

#include "PC_FileIO.c"

// global constants
const float ENCODER_TO_INCH = 0, PEN_UP = 0, PEN_DOWN = 0, GANTRY_KP = 0,
PEN_KP = 0, Y_AXIS_HOME_DISTANCE = 0;

// MotorCommand struct
struct MotorCommand {
    float x;
    float y;
    bool liftPen;
}

// prototypes
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
void shutcoGoofyAhhDown();

task main()
{
	while (true)
	{
		mainMenu();
	}
}

void movePen(int xPower, int yPower)
{
	motor[motorA] = xPower;
	motor[motorB] = yPower;
}

void autoMovePen(float x, float y)
{

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
		while(!getButtonPress(BACK_BUTTON) && readNextCommand(fin, motorCommand))
		{
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
	int count = 1;
	string menuOptions[] = {"Manual", "File Print", "Exit"};

	// title
	displayCenteredBigTextLine(1, "VanGoBot");
	displayCenteredTextLine(4, "Select a drawing mode:");

	// options
	displayInverseBigStringAt(20, 60, menuOptions[0]);
	displayBigStringAt(20, 40, menuOptions[1]);
	displayBigStringAt(20, 20, menuOptions[2]);

	while (count <= 3)
	{
		if (count == 3)
		{
			count = 1;
		}

		while (!getButtonPress(DOWN_BUTTON) && !getButtonPress(UP_BUTTON) && !getButtonPress(ENTER_BUTTON))
		{}

		// could possibly use ternary operators instead here but do we care about efficiency? :D

		// navigating options
		if (getButtonPress(DOWN_BUTTON))
		{
			displayBigStringAt(20, 60 - (20 * (count - 1)), menuOptions[count - 1]);
			displayInverseBigStringAt(20, 60 - (20 * count), menuOptions[count]);
			count++;
		}
		else if (getButtonPress(UP_BUTTON))
		{
			displayBigStringAt(20, 60 - (20 * count), menuOptions[count];
			displayInverseBigStringAt(20, 60 - (20 * (count + 1)), menuOptions[count + 1]));
			count++;
		}

		// if enter button is pressed, call the functions for the correct mode
		else if (getButtonPress(ENTER_BUTTON))
		{
			if (count == 0)
			{
				manualMove();
			}
			else if (count == 1)
			{
				automaticModeMenu();
			}
			else
			{
				// call shutdown procedure
				shutcoGoofyAhhDown();
			}
		}
	}
}

void shutcoGoofyAhhDown()
{

}
