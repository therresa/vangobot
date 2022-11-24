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
const float ENCODER_TO_INCH = 0, PEN_UP = 30, PEN_DOWN = 0, GANTRY_KP = 0,
						PEN_KP = 0.5, Y_AXIS_HOME_DISTANCE = 4.7, LIFT_PEN_THRESHOLD = 10,
						GANTRY_THRESHOLD = 0;

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
		//mainMenu();
	home();
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
	eraseDisplay();
	displayBigTextLine(1, "Manual Move");
	float x=0;
	float y=0;
	while(!getButtonPress(buttonBack)){
		if(getButtonPress(buttonUp)){
			y = 20;
		}
		else if(getButtonPress(buttonDown)){
			y = -20;
		}
		else{
			y = 0;
		}
		if(getButtonPress(buttonLeft)){
			x = 20;
		}
		else if(getButtonPress(buttonRight)){
			x = -20;
		}
		else{
			x = 0;
		}
		movePen(x, y);
		if(getButtonPress(buttonEnter)){
			clearTimer(T1);
			while(getButtonPress(buttonEnter)){}
			if(time1[T1] > 1000){
				break;
			}
			liftLowerPen(nMotorEncoder[motorC] < 10);
		}
	}
}

void home(){
	liftLowerPen(true);
		movePen(0, -10);
		while(!SensorValue[S1]) {}
		movePen(-10, 0);
		while(SensorValue[S2] > Y_AXIS_HOME_DISTANCE){
				displayBigTextLine(1, "Sensor: %f", SensorValue[S2]);
		}
		movePen(0, 0);
		nMotorEncoder[motorA] = 0;
		nMotorEncoder[motorB] = 0;
}

void liftLowerPen(bool lifted){
	float target = lifted?PEN_UP:PEN_DOWN;
	float measured = nMotorEncoder[motorC];

	while(abs(target-measured)>LIFT_PEN_THRESHOLD && !getButtonPress(BACK_BUTTON)){
		measured = nMotorEncoder[motorC];
		float error = target-measured;
		float output = error*PEN_KP;
		motor[motorC] = output;
		wait1Msec(100);
	}
	motor[motorC] = 0;
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

void automaticModeMenu()
{

}

bool readNextCommand(TFileHandle &fin, struct MotorCommand &motorCommand)
{
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
	eraseDisplay();
	int count = 0;
	string menuOptions[] = {"Manual", "File Print", "Exit"};



	// options select
	while (count < 3)
	{
		// title
		displayCenteredBigTextLine(1, "VanGoBot");
		drawEllipse(35, 125, 140, 100);
		drawEllipse(35, 122, 140, 97);
		displayCenteredBigTextLine(4, "Select a mode:");
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
			while (getButtonPress(DOWN_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(ENTER_BUTTON))
			{}
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
	// are you sure?
	eraseDisplay();
	displayCenteredBigTextLine(0, "Are you sure");
	displayCenteredBigTextLine(2, "you want");
	displayCenteredBigTextLine(4, "to shut down");
	displayCenteredBigTextLine(6, "VanGoBot? :(");

	//options
	displayBigStringAt(20, 50, "Yes");
	displayInverseBigStringAt(20, 30, "No");

	//toggle between options
	int option = 2;
	while (true)
	{
		while (!getButtonPress(DOWN_BUTTON) && !getButtonPress(UP_BUTTON) && !getButtonPress(ENTER_BUTTON))
		{}
		if (getButtonPress(DOWN_BUTTON))
		{
			displayBigStringAt(20, 50, "Yes");
			displayInverseBigStringAt(20, 30, "No");
			option = 2;
		}
		else if(getButtonPress(UP_BUTTON))
		{
			displayInverseBigStringAt(20, 50, "Yes");
			displayBigStringAt(20, 30, "No");
			option = 1;
		}
		if (getButtonPress(ENTER_BUTTON))
		{
			if (option == 2)
			{
				while (getButtonPress(DOWN_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(ENTER_BUTTON))
				{}
				mainMenu();
			}
			else if (option == 1)
			{
				while (getButtonPress(DOWN_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(ENTER_BUTTON))
				{}
				liftLowerPen(true);
				home();
				movePen(0, 0);
			}
		}
		while (getButtonPress(DOWN_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(ENTER_BUTTON))
		{}
	}
}
