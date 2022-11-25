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
const float ENCODER_TO_INCH = 3.0/228.0, PEN_UP = 30, PEN_DOWN = 0, GANTRY_KP = 2,
						PEN_KP = 0.5, Y_AXIS_HOME_DISTANCE = 5, LIFT_PEN_THRESHOLD = 10,
						GANTRY_THRESHOLD = 5, MIN_X = 0, MAX_X = 420, MIN_Y = 0, MAX_Y = 510;

const string files[] = {
	"test_fileio.txt",
	"square.txt",
	"triangle.txt",
	"circle.txt",
	"test3.txt",
	"test4.txt"
};
// MotorCommand struct
struct MotorCommand {
    float x;
    float y;
    bool liftPen;
}

// prototypes
void movePen(int xPower, int yPower);
void autoMovePen(float targetX, float targetY);
void liftLowerPen(bool lifted);
void home();
bool readNextCommand(TFileHandle &fin, struct MotorCommand &motorCommand);
void manualMode();
void convertFileXYToPaperXY(float autoX, float autoY, float size, struct MotorCommand &motorCommand);
void automaticMode(TFileHandle &fin, float x, float y, float size);
bool automaticModeMenu();
void mainMenu();
void configureSensors();
bool shutDown();
void fileSelectMenu(string &fileName);

task main()
{
	home();
	mainMenu();
}

void movePen(int xPower, int yPower)
{
	if((xPower <= 0 && nMotorEncoder[motorA] >= MIN_X) || (xPower >= 0 && nMotorEncoder[motorA] <= MAX_X)){
		if(xPower < -20){
			xPower = -20;
		}
		if(xPower > 20){
			xPower = 20;
		}
		motor[motorA] = xPower;
	}
	else{
		motor[motorA] = 0;
		playTone(300, 1);
	}
	if((yPower <= 0 && nMotorEncoder[motorB] >= MIN_Y) || (yPower >= 0 && nMotorEncoder[motorB] <= MAX_Y)){
		if(yPower < -20){
			yPower = -20;
		}
		if(yPower > 20){
			yPower = 20;
		}
		motor[motorB] = yPower;
	}
	else{
		motor[motorB] = 0;
		playTone(300, 1);
	}
}

void autoMovePen(float targetX, float targetY)
{
	targetX /= ENCODER_TO_INCH;
	targetY /= ENCODER_TO_INCH;
	float measuredX = nMotorEncoder[motorA];
	float measuredY = nMotorEncoder[motorB];

	while((abs(targetX-measuredX)>GANTRY_THRESHOLD || abs(targetY-measuredY)>GANTRY_THRESHOLD) && !getButtonPress(ENTER_BUTTON)){
		measuredX = nMotorEncoder[motorA];
	 	measuredY = nMotorEncoder[motorB];
		float errorX = targetX-measuredX;
		float outputX = errorX*PEN_KP;
		float errorY = targetY-measuredY;
		float outputY = errorY*PEN_KP;
		movePen(outputX, outputY);
		wait1Msec(100);
	}
	movePen(0, 0);
	wait1Msec(1000);
}

void manualMode(){
	eraseDisplay();
	displayCenteredBigTextLine(2, "Manual Mode");
	displayBigTextLine(4, "Press and hold");
	displayBigTextLine(6, "the 'enter' key");
	displayBigTextLine(8, "to exit");
	float x=0;
	float y=0;
	while(!getButtonPress(buttonBack)){
		if(getButtonPress(buttonUp) && !getButtonPress(buttonDown)){
			y = 20;
		}
		else if(getButtonPress(buttonDown) && !getButtonPress(buttonUp)){
			y = -20;
		}
		else{
			y = 0;
		}
		if(getButtonPress(buttonLeft) && !getButtonPress(buttonRight)){
			x = -20;
		}
		else if(getButtonPress(buttonRight)){
			x = 20;
		}
		else{
			x = 0;
		}

		movePen(x, y);
		if(getButtonPress(buttonEnter)){
			clearTimer(T1);
			while(getButtonPress(buttonEnter)){}
			if(time1[T1] > 1000){
				eraseDisplay();
				break;
			}
			liftLowerPen(nMotorEncoder[motorC] < 10);
		}
	}
	mainMenu();
}

void home(){
	nMotorEncoder[motorA] = 1000;
	nMotorEncoder[motorB] = 1000;
	liftLowerPen(true);
		movePen(-10, 0);
		while(!SensorValue[S1]) {}
		movePen(0, -10);
		while(SensorValue[S2] > Y_AXIS_HOME_DISTANCE){}
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

// assume the motorCommand x and y is normalized (between 0 and 1).
void convertFileXYToPaperXY(float autoX, float autoY, float size, struct MotorCommand &motorCommand){
	motorCommand.x = autoX + motorCommand.x*size;
	motorCommand.y = autoY + motorCommand.y*size;
}


bool automaticModeMenu()
{
	string fileName = "";
	fileSelectMenu(fileName);
	eraseDisplay();

	// set width
	float width = 0;
	displayBigTextLine(1, "Select width:");

	//wait for all buttons to be released
	while (getButtonPress(ENTER_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(DOWN_BUTTON)) { }

	while (!getButtonPress(ENTER_BUTTON))
	{
		displayCenteredBigTextLine(4, "%f", width);
		while (!getButtonPress(ENTER_BUTTON) && !getButtonPress(UP_BUTTON) && !getButtonPress(DOWN_BUTTON)) { }
		if (getButtonPress(DOWN_BUTTON))
		{
			width -= 0.5;
		}
		else if (getButtonPress(UP_BUTTON))
		{
			width += 0.5;
		}
		while (getButtonPress(UP_BUTTON) || getButtonPress(DOWN_BUTTON)) { }
		if(width >= 5){
			width = 5;
		}
		if(width <= 0){
			width = 0;
		}
	}

	// wait for all buttons to be released
	while (getButtonPress(ENTER_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(DOWN_BUTTON)) { }
	eraseDisplay();

	displayCenteredBigTextLine(1, "Select x:");

	// set x coordinate
	float x = 0;
	while (!getButtonPress(ENTER_BUTTON))
	{
		displayCenteredBigTextLine(4, "%f", x);
		while (!getButtonPress(ENTER_BUTTON) && !getButtonPress(UP_BUTTON) && !getButtonPress(DOWN_BUTTON)) { }
		if (getButtonPress(DOWN_BUTTON))
		{
			x -= 0.5;
		}
		else if (getButtonPress(UP_BUTTON))
		{
			x += 0.5;
		}
		while (getButtonPress(UP_BUTTON) || getButtonPress(DOWN_BUTTON)) { }
		if(x <= 0){
			x = 0;
		}
		if(x >= 5){
			x = 5;
		}
	}


	// wait for all buttons to be released
	while (getButtonPress(ENTER_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(DOWN_BUTTON)) { }
	eraseDisplay();

	displayBigTextLine(1, "Select y:");
	float y = 0;
	while (!getButtonPress(ENTER_BUTTON))
	{
		displayCenteredBigTextLine(4, "%f", y);
		while (!getButtonPress(ENTER_BUTTON) && !getButtonPress(UP_BUTTON) && !getButtonPress(DOWN_BUTTON)) { }

		if (getButtonPress(DOWN_BUTTON))
		{
			y -= 0.5;
		}
		else if (getButtonPress(UP_BUTTON))
		{
			y += 0.5;
		}

		while (getButtonPress(UP_BUTTON) || getButtonPress(DOWN_BUTTON)) { }

		if(y <= 0){
			y = 0;
		}
		if(y >= 6.5){
			y = 6.5;
		}
	}

	while (getButtonPress(ENTER_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(DOWN_BUTTON)) { }

	TFileHandle fin;
	bool fileOkay = openReadPC(fin,fileName);

	if ((x + width > 5) || (y + width > 6.5)
	{
		eraseDisplay();
		displayCenteredBigTextLine(1, "Your selected file");
		displayCenteredBigTextLine(3, "will be cut off");
		displayCenteredBigTextLine(5, "by the drawing bound.");
		displayCenteredBigTextLine(7, "Continue?");

		// options
		int option = 2;
		displayBigStringAt(20, 50, "Yes");
		displayInverseBigStringAt(20, 30, "No");

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
					return false;
				}
				else if (option == 1)
				{
					while (getButtonPress(DOWN_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(ENTER_BUTTON))
					{}
					automaticMode(fin, x, y, width);
					return true;
				}
			}
			while (getButtonPress(DOWN_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(ENTER_BUTTON))
			{}
		}
	}
	automaticMode(fin, x, y, width);
	return true;
}

void fileSelectMenu(string &fileName){
	int selected = (sizeof(files)/sizeof(files[0]))-1;
	while(!getButtonPress(ENTER_BUTTON)){
		eraseDisplay();
		displayBigTextLine(1, "Select File");

		for(int i = -1; i <= 1; i++){
			if(i == 0){
					displayInverseBigStringAt(20, 60+20*i, files[selected]);
			}
			else if(i+selected >= 0 && i+selected < (sizeof(files)/sizeof(files[0]))){
					displayBigStringAt(20, 60+20*i, files[selected+i]);
			}
		}

		while(!getButtonPress(UP_BUTTON) && !getButtonpress(DOWN_BUTTON) && !getButtonPress(ENTER_BUTTON)){}
		if(getButtonPress(UP_BUTTON)){
			selected++;
		}
		if(getButtonPress(DOWN_BUTTON)){
			selected--;
		}
		if(selected <= 0){
			selected = 0;
		}
		if(selected >= (sizeof(files)/sizeof(files[0]))-1){
			selected = (sizeof(files)/sizeof(files[0]))-1;
		}
		while(getButtonPress(UP_BUTTON) || getButtonpress(DOWN_BUTTON)){}
	}
	while(getButtonPress(ENTER_BUTTON)){}
	fileName = files[selected];
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
	eraseDisplay();
		struct MotorCommand motorCommand;
		liftLowerPen(true);
		autoMovePen(x, y);
		while(!getButtonPress(ENTER_BUTTON) && readNextCommand(fin, motorCommand))
		{
			convertFileXYToPaperXY(x, y, size, motorCommand);
			displayBigTextLine(2, "x: %f", motorCommand.x);
			displayBigTextLine(6, "y: %f", motorCommand.y);
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

	// options select
	while (count < 3)
	{
		eraseDisplay();
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
				manualMode();
			}
			else if (count == 1)
			{
				while (!automaticModeMenu())
				{ }
			}
			else
			{
				// call shutdown procedure
				if(shutDown()){
					break;
				}
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

bool shutDown()
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
				return false;
			}
			else if (option == 1)
			{
				while (getButtonPress(DOWN_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(ENTER_BUTTON))
				{}
			return true;
			}
		}
		while (getButtonPress(DOWN_BUTTON) || getButtonPress(UP_BUTTON) || getButtonPress(ENTER_BUTTON))
		{}
	}
	return true;
}
