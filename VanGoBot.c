/*
Group members: Eric Aleong, Owen Leather, Nush Majra, Theresa Nguyen
Group 4-17
Version: 1.0
Description: Functions written in ROBOTC for VanGoBot to be deployed
on EV3 brick.
Drain
*/
#pragma SystemFile

// select EV3 set of functions
#define _EV3FILEIO 1

#include "PC_FileIO.c"

// global constants
const float ENCODER_TO_INCH = 0, PEN_UP = 0, PEN_DOWN = 0, GANTRY_KP = 0,
PEN_KP = 0, Y_AXIS_HOME_DISTANCE = 0, LIFT_PEN_THRESHOLD = 0, GANTRY_THRESHOLD = 0;

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
void convertFileXYToPaperXY(float autoX, float autoY, float size, struct MotorCommand &motorCommand);
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

void liftLowerPen(bool lifted){
	float target = lifted?PEN_UP:PEN_DOWN;
	float measured = nMotorEncoder[motorC];
	while(abs(target-measured)<LIFT_PEN_THRESHOLD && !getButtonPress(BACK_BUTTON)){
		measured = nMotorEncoder[motorC];
		float error = target-measured;
		float output = error*PEN_KP;
		motor[motorC] = output;
		wait1Msec(100);
	}
}

//assume the motorCommand x and y is normalized (between 0 and 1).
void convertFileXYToPaperXY(float autoX, float autoY, float size, struct MotorCommand &motorCommand){
	motorCommand.x = autoX + motorCommand.x*size;
	motorCommand.y = autoY + motorCommand.y*size;
}

void automaticMode(TFileHandle &fin, float x, float y, float size){
	struct MotorCommand motorCommand;
	while(readNextCommand(fin, motorCommand) && getButtonPress(BACK_BUTTON)){
		liftLowerPen(motorCommand.liftPen);
		autoMovePen(motorCommand.x, motorCommand.y);
	}
}

void automaticModeMenu(){

}

bool readNextCommand(TFileHandle &fin, struct MotorCommand &motorCommand){
	int liftPen = 0;
	float x = 0.0;
	float y = 0.0;
	bool success = true;
	success = success && readFloatPC(fin, x);
	success = success && readFloatPC(fin, y);
	success = success && readIntPc(fin, liftPen);
	motorCommand.x = x;
	motorCommand.y = y;
	motorCommand.liftPen = liftPen==1;
	return success;
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
	int count = 0;
	string menuOptions[] = {"Manual", "File Print", "Exit"};

	// title
	displayCenteredBigTextLine(1, "VanGoBot");
	drawEllipse(35, 125, 140, 100);
	drawEllipse(35, 122, 140, 97);
	displayCenteredBigTextLine(4, "Select a mode:");

	// options select
	while (count < 3)
	{
		if(count == 0){
			displayInverseBigStringAt(20, 60, menuOptions[0]);
			displayBigStringAt(20, 40, menuOptions[1]);
			displayBigStringAt(20, 20, menuOptions[2]);
		}
		if(count == 1){
			displayBigStringAt(20, 60, menuOptions[0]);
			displayInverseBigStringAt(20, 40, menuOptions[1]);
			displayBigStringAt(20, 20, menuOptions[2]);
		}
		if(count == 2){
			displayBigStringAt(20, 60, menuOptions[0]);
			displayBigStringAt(20, 40, menuOptions[1]);
			displayInverseBigStringAt(20, 20, menuOptions[2]);
		}
		while (!getButtonPress(DOWN_BUTTON) && !getButtonPress(UP_BUTTON) && !getButtonPress(ENTER_BUTTON))
		{}

		if(getButtonPress(DOWN_BUTTON)){
			count++;
		}
		else if(getButtonPress(UP_BUTTON)){
			count--;
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

		if(count >= 3){
			count = 0;
		}
		if(count <= -1){
			count = 2;
		}

		//Wait until button released
		while (getButtonPress(DOWN_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(ENTER_BUTTON))
		{}
	}
}

void shutcoGoofyAhhDown()
{
	liftLowerPen(true);
	home();
	movePen(0, 0);
}
