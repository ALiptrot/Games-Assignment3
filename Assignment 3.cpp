// Assignment 3.cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <iostream>
#include <sstream>
#include <math.h>
#include <vector>
#include <fstream>
using namespace tle;
using namespace std;
int numberOfCheckpoints = 0;//number of checkpoints on the track
int numberOfIsles = 0;//number of isles on the track
int numberOfWalls = 0;//number of walls on the track
int numberOfVehicles = 0;//number of vehicles on the track
int numberOfWaterTanks = 0;//number of water tanks on the track
int startRead = 0;//starting position to read the data of the object
int endRead = 0;//end position for reading the data of the object
const int knumberOfObjects = 5;//number of unique objects in the game
string fileName = "RaceTrack.txt";//location of the RaceTrack file
vector <string> raceTrackData;//holds all the data from the text file
enum MoveState
{
	Stationary = 0,
	Reverse = 1,
	Forward = 2,
	Collision = 3,
};
struct Objectstructure
{
	float xPos;
	float zPos;
	float rotationX;
	float rotationY;
	float rotationZ;
	IModel* Model;
};
struct checkpointstructure
{
	bool isPastCheckpoint;
};
struct islestructure
{
	float maxX;
	float minX;
	float maxZ;
	float minZ;
};
struct wallstructure
{
	float maxX;
	float minX;
	float maxZ;
	float minZ;
};
struct Vehicle
{
	MoveState carMoveState;//the move state (Stationary, Reverse or Forward)
	MoveState lastCarMoveState;
	float moveXOld;//the old movement x units of the model
	float moveZOld;//the old movement z units of the model
	float rotation;//the rotation value of the model
	float moveX;//the current movement x units of the model
	float moveZ;//the current movement z units of the model
	int carRaceState;//ie start, checkpoint1, finish
	int lastCarRaceState;
};
struct watertankstructure
{
	float xPos;
	float zPos;
	float rotationX;
	float rotationY;
	float rotationZ;
};
vector <Objectstructure> checkpoints;//holds the X and Z positions and the rotation values
vector <Objectstructure> isles;//holds the X and Z positions and the rotation values
vector <Objectstructure> walls;//holds the X and Z positions and the rotation values
vector <Objectstructure> vehicles;//holds the X and Z positions and the rotation valuesv
vector <Objectstructure> waterTanks;//holds the X and Z positions and the rotation values

float DegreesToRadians(float degrees);
float Hover(float Xcounter);
string PlayerRaceStateText(int raceState);
void DetectCollision(int kNumberOfWalls, int kNumberOfIsles, vector <Vehicle> &hoverCarProp, vector <wallstructure> wallProp, vector <islestructure> isleProp, float waterTankRadius, int kNumberOfWaterTanks, float kCarRadius);
int ReadObjectPositions(vector <Objectstructure> &objects);
void OpenTrack(string fileName, IMesh* checkpointMesh, IMesh* wallMesh, IMesh* hoverCarMesh, IMesh* isleMesh, IMesh* waterTankMesh);
void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine( kTLX );
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder( "C:\\Users\\BEAST\\Documents\\Assignment 3 Models" );

	/**** Set up your scene here ****/
		ICamera* myCamera = myEngine->CreateCamera(kManual);//creating and setting up the camera
		IFont* myFont = myEngine->LoadFont("Comic Sans MS", 36);//setting up the font used with any text displayed

		IMesh* checkpointMesh = myEngine->LoadMesh("Checkpoint.x");
		IMesh* isleMesh = myEngine->LoadMesh("IsleStraight.x");
		IMesh* wallMesh = myEngine->LoadMesh("Wall.x");
		IMesh* waterTankMesh = myEngine->LoadMesh("TankSmall1.x");
		IMesh* skyMesh = myEngine->LoadMesh("Skybox 07.x");
		IMesh* floorMesh = myEngine->LoadMesh("ground.x");
		IMesh* hoverCarMesh = myEngine->LoadMesh("race2.x");
		/*
		Loading in all the meshes for the objects
		*/

		OpenTrack(fileName, checkpointMesh, wallMesh, hoverCarMesh, isleMesh, waterTankMesh);//opens the track file

		//constants
		const int kNumberOfCheckpoints = numberOfCheckpoints;//number of checkpoints on the track
		const int kNumberOfIsles = numberOfIsles;//number of isles on the track
		const int kNumberOfWalls = numberOfWalls;//number of walls on the track
		const int kNumberOfWaterTanks = numberOfWaterTanks;//number of water tanks on the track
		const float waterTankRadius = 5;//radius of the water tank
		const int kbottomOfScreen = 666;//bottom of the screen to draw the UI and text
		const float kMaxSpeedForward = 120*120;//top speed of a vehicle going forwards
		const float kMaxSpeedBackward = 60*60;//top speed of a vehicle going backwards
		const int kGameSpeed = 50;//gamespeed
		const float playerLeanLimit = 40;
		const int kNumberOfCars = numberOfVehicles;//number of hovercars in the world
		const int kAccel = 2;//acceleration used in the formula to get the new speed of the vehicles
		const int kCheckpointRadius = 3;//radius of the collision for the inside of the checkpoint
		const int kCarRadius = 4.5;//radius of the hovercar
		const float kIsleLengthZFromOrigin = 4.72;//used to detect collision for box-sphere
		const float kIsleLengthXFromOrigin = 0.1;//used to detect collision for box-sphere
		const float kWallLengthZFromOrigin = 4;//used to detect collision for box-sphere
		const float kWallLengthXFromOrigin = 1;//used to detect collision for box-sphere

		vector <islestructure> isleProp;//properties of the isle object
		isleProp.resize(kNumberOfIsles);//setting the size of the vector

		for (int i = 0; i < kNumberOfIsles; i++)
		{
			if (isles[i].rotationY == 0)
			{
				isleProp[i].maxX = isles[i].xPos + kIsleLengthXFromOrigin + kCarRadius, isleProp[i].minX = isles[i].xPos - kIsleLengthXFromOrigin - kCarRadius; 
				isleProp[i].maxZ = isles[i].zPos + kIsleLengthZFromOrigin + kCarRadius, isleProp[i].minZ = isles[i].zPos - kIsleLengthZFromOrigin - kCarRadius;
			}
			if (isles[i].rotationY == 90)
			{
				isleProp[i].maxX = isles[i].xPos + kIsleLengthZFromOrigin + kCarRadius, isleProp[i].minX = isles[i].xPos - kIsleLengthZFromOrigin - kCarRadius; 
				isleProp[i].maxZ = isles[i].zPos + kIsleLengthXFromOrigin + kCarRadius, isleProp[i].minZ = isles[i].zPos - kIsleLengthXFromOrigin - kCarRadius;
			}
			/*
			Setting up the values for collision detection, if the wall has been rotated, then the values are chenged according to that
			*/
		}

		vector <wallstructure> wallProp;//properties of the wall object
		wallProp.resize(kNumberOfWalls);//setting the size of the vector

		for (int i = 0; i < kNumberOfWalls; i++)
		{
			if (walls[i].rotationY == 0)
			{
				wallProp[i].maxX = walls[i].xPos + kWallLengthXFromOrigin + kCarRadius, wallProp[i].minX = walls[i].xPos - kWallLengthXFromOrigin - kCarRadius; 
				wallProp[i].maxZ = walls[i].zPos + kWallLengthZFromOrigin + kCarRadius, wallProp[i].minZ = walls[i].zPos - kWallLengthZFromOrigin - kCarRadius;
			}
			if (walls[i].rotationY == 90)
			{
				wallProp[i].maxX = walls[i].xPos + kWallLengthZFromOrigin + kCarRadius, wallProp[i].minX = walls[i].xPos - kWallLengthZFromOrigin - kCarRadius; 
				wallProp[i].maxZ = walls[i].zPos + kWallLengthXFromOrigin + kCarRadius, wallProp[i].minZ = walls[i].zPos - kWallLengthXFromOrigin - kCarRadius;
			}
			/*
			Setting up the values for collision detection, if the wall has been rotated, then the values are chenged according to that
			*/
		}

		vector <checkpointstructure> checkpointProp;//properties of the checkpoint object
		checkpointProp.resize(numberOfCheckpoints);//setting the size of the vector
		for (int i = 0; i < kNumberOfCheckpoints; i++)
		{
			checkpointProp[i].isPastCheckpoint = true;
		}

		//sky
		IModel* sky = skyMesh->CreateModel(0,-960,0);//loading in the sky

		//floor
		IModel* floor = floorMesh->CreateModel();//loading in the floor

		//Hovercars
		vector <Vehicle> hoverCarProp;//properties of  the hoverCar object
		hoverCarProp.resize(kNumberOfCars);//setting the size of the vector
		for (int i = 0; i < kNumberOfCars; i++)
		{
			hoverCarProp[i].rotation = 0;
			hoverCarProp[i].carMoveState = Stationary;
			hoverCarProp[i].moveX = 0;
			hoverCarProp[i].moveZ = 0;
			hoverCarProp[i].carRaceState = 0;
		}
		/*
		Initialising all the values in the hoverCarProp to 0, or for the moveState Stationary
		*/
		myCamera->SetPosition(0,12,-30);
		myCamera->AttachToParent(vehicles.at(0).Model);//attatches camera to hoverCar so that the camera follows it

		//UI
		ISprite* backdrop = myEngine->CreateSprite( "ui_backdrop.jpg", 0, kbottomOfScreen );

		//controls
		const EKeyCode quitKey = Key_Escape;
		const EKeyCode forwardKey = Key_W;
		const EKeyCode backwardKey = Key_S;
		const EKeyCode leftKey = Key_A;
		const EKeyCode rightKey = Key_D;
		const EKeyCode cameraUpKey = Key_Up;
		const EKeyCode cameraDownKey = Key_Down;
		const EKeyCode cameraRightKey = Key_Right;
		const EKeyCode cameraLeftKey = Key_Left;
		const EKeyCode cameraResetKey = Key_1;
		const EKeyCode mouseShowKey = Key_Tab;
		const EKeyCode camera1stPersonKey = Key_2;

		float fontX = 0;//texts x position
		float fontY = kbottomOfScreen + 10;//texts y position
		float acceleration = 0;//acceleration of the player
		float playerRotation = 0;//rotation of the car
		float playerMoveX = 0;//the players move vector in the x direction
		float playerMoveZ = 0;//the players move vector in the z direction
		float Xcounter = 0;//used to get the Y values of the sine wave, so the car appears to hover
		float playerLean = 0;//used to store how much the player is leaning when turning
		float collisionDist = 0;//used to store the distance between the car and checkpoint
		bool is1stPerson = false;
	// The main game loop, repeat until engine is stopped
		myEngine->Timer();//starts the timer
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();
		/**** Update your scene each frame here ****/
		float frameTime = myEngine->Timer();//amount of time passed to complete the frame
		const float fps = 1/frameTime;//the frames per second
		vehicles[0].xPos = vehicles[0].Model->GetX();//gets the x position of the player
		vehicles[0].zPos = vehicles[0].Model->GetZ();//gets the z position of the player


		float mouseMoveX = myEngine->GetMouseMovementX();//the amount the mouse has moved on the X axis since the last frame
		float mouseMoveY = myEngine->GetMouseMovementY();//the amount the mouse has moved on the Z axis since the last frame
		myCamera->RotateLocalY((kGameSpeed*mouseMoveX*3)/fps);
		myCamera->RotateLocalX((kGameSpeed*mouseMoveY*3)/fps);

		if (playerRotation >= 360)
		{
			playerRotation = 0;//sets the player rotation back to 0 so that the rotation value cannot go above 360 degrees
		}

		switch(hoverCarProp[0].carMoveState)
		{
		case Stationary:
			acceleration = 0;
			if (myEngine->KeyHeld(forwardKey))
			{
				acceleration = kAccel;
				hoverCarProp[0].carMoveState = Forward;
			}
			if (myEngine->KeyHeld(backwardKey))
			{
				acceleration = -kAccel;
				hoverCarProp[0].carMoveState = Reverse;
			}
			break;
			/*
			in the stationary state the hover car can go either backwards or forwards
			*/
		case Reverse:
			if (!myEngine->KeyHeld(backwardKey))
			{
				acceleration = kAccel/2;
			}
			if (myEngine->KeyHeld(backwardKey))
			{
				acceleration = -kAccel;
				if (playerMoveZ*playerMoveZ > kMaxSpeedBackward || playerMoveX*playerMoveX > kMaxSpeedBackward)
				{
					acceleration = 0;//keeps the speed constant
				}
			}
			if (myEngine->KeyHeld(forwardKey))
			{
				acceleration = kAccel*2;
			}
			if (hoverCarProp[0].moveZ > 0)
			{
				hoverCarProp[0].carMoveState = Stationary;
			}
			break;
			/*
			in the reverse state the car accelerates if the backwards key is held, when it reaches the top speed the 
			the speed is constant unless the player lets go of the backwards key. If the backwards key is not held down then the car deaccelerates
			until it's speed reaches 0. The car deaccelerates at a faster rate if the forwards key is held
			*/
		case Forward:
			if (!myEngine->KeyHeld(forwardKey))
			{
				acceleration = -kAccel/2;
			}
			if (myEngine->KeyHeld(forwardKey))
			{
				acceleration = kAccel;
				if (playerMoveZ*playerMoveZ > kMaxSpeedForward || playerMoveX*playerMoveX > kMaxSpeedForward)
				{
					acceleration = 0;
				}
			}
			if (myEngine->KeyHeld(backwardKey))
			{
				acceleration = -kAccel*2;
			}
			if (hoverCarProp[0].moveZ < 0)
			{
				hoverCarProp[0].carMoveState = Stationary;
			}
			break;
			/*
			in the forward state the car accelerates if the forwards key is held, when it reaches the top speed the 
			the speed is constant unless the player lets go of the forward key. If the forward key is not held down then the car deaccelerates
			until it's speed reaches 0. The car deaccelerates at a faster rate if the backwards key is held
			*/
		case Collision:
			if (hoverCarProp[0].moveX > 0 || hoverCarProp[0].moveZ > 0)
			{
				float tempX = hoverCarProp[0].moveX;
				float tempZ = hoverCarProp[0].moveZ;
				hoverCarProp[0].moveX = -tempX/2;
				hoverCarProp[0].moveZ = -tempZ/2;
				if (hoverCarProp[0].lastCarMoveState == Forward)
				{
					hoverCarProp[0].carMoveState = Reverse;
				}
				if (hoverCarProp[0].lastCarMoveState == Reverse)
				{
					hoverCarProp[0].carMoveState = Forward;
				}
			}
			else
			{
				hoverCarProp[0].carMoveState = Stationary;
			}
			break;
		}//end of switch

		DetectCollision(kNumberOfWalls, kNumberOfIsles, hoverCarProp, wallProp, isleProp, waterTankRadius, kNumberOfWaterTanks, kCarRadius);//detecting collisions

		float playerVectorAngle = 90 - playerRotation;
		playerVectorAngle = DegreesToRadians(playerVectorAngle);//converts the angle of the car rotation to radians
		playerMoveX = cos(playerVectorAngle) * (kGameSpeed*hoverCarProp[0].moveX);//calculates the X direction vector
		playerMoveZ = sin(playerVectorAngle) * (kGameSpeed*hoverCarProp[0].moveZ);//calculates the Z direction vector
		vehicles[0].Model->MoveX(playerMoveX/fps);//moves the car in the x axis according to the current fps
		vehicles[0].Model->MoveZ(playerMoveZ/fps);//moves the car in the z axis according to the current fps

		hoverCarProp[0].moveXOld = hoverCarProp[0].moveX;
		hoverCarProp[0].moveZOld = hoverCarProp[0].moveZ;
		hoverCarProp[0].moveX = hoverCarProp[0].moveXOld + acceleration * frameTime;//calculating the new speed in the x direction
		hoverCarProp[0].moveZ = hoverCarProp[0].moveZOld + acceleration * frameTime;//calculating the new speed in the z direction

		

		for (int i = 0; i < kNumberOfCheckpoints; i++)
		{
			float x,z;
			x = vehicles[0].xPos - checkpoints[i].xPos;
			z = vehicles[0].zPos - checkpoints[i].zPos;
			collisionDist = sqrt(x*x + z*z);
			if (collisionDist < (kCarRadius + kCheckpointRadius))
			{
				checkpointProp[i].isPastCheckpoint = false;
			}
			else if (collisionDist > (kCarRadius + kCheckpointRadius) && !checkpointProp[i].isPastCheckpoint)
			{
				checkpointProp[i].isPastCheckpoint = true;
				hoverCarProp[0].lastCarRaceState = hoverCarProp[0].carRaceState;
				hoverCarProp[0].carRaceState++;
				if (hoverCarProp[0].carRaceState - hoverCarProp[0].lastCarRaceState != 1)
				{
					hoverCarProp[0].carRaceState = hoverCarProp[0].lastCarRaceState;
				}
			}
		}//end of for loop
		/*
		This for loop detects of the car reaches the checkpoint, if so then the it detects when the car has left the checkpoint.
		Once the car has left the checkpoint then the car race state is increased by 1 (how many checkpoints the car has gone through).
		It also checks the last checkpoint the car went through, and if the difference between them is not 1 then the car race state is
		not increased.
		*/

		if (myEngine->KeyHeld(cameraDownKey))
		{
			myCamera->MoveLocalZ(-kGameSpeed/fps);
		}
		if (myEngine->KeyHeld(cameraUpKey))
		{
			myCamera->MoveLocalZ(kGameSpeed/fps);
		}
		if (myEngine->KeyHeld(cameraRightKey))
		{
			myCamera->MoveLocalX(kGameSpeed/fps);
		}
		if (myEngine->KeyHeld(cameraLeftKey))
		{
			myCamera->MoveLocalX(-kGameSpeed/fps);
		}
		if (myEngine->KeyHit(cameraResetKey))
		{
			myCamera->SetPosition(vehicles[0].xPos,0,vehicles[0].zPos);
			myCamera->SetLocalPosition(0,12,-30);
			myCamera->ResetOrientation();
			is1stPerson = false;
		}
		if (myEngine->KeyHeld(mouseShowKey))
		{
			myEngine->StopMouseCapture();
		}
		if (!myEngine->KeyHeld(mouseShowKey))
		{
			myEngine->StartMouseCapture();
		}
		if (myEngine->KeyHit(camera1stPersonKey) && !is1stPerson)
		{
			myCamera->Move(0,-6.3,32);
			is1stPerson = true;
		}
		if (myEngine->KeyHit(quitKey))
		{
			myEngine->Stop();
		}
		if (myEngine->KeyHeld(leftKey))
		{
			float moveSpeed = kGameSpeed/fps;
			playerRotation -= moveSpeed;
			vehicles[0].Model->RotateLocalY(-moveSpeed);
			/*
			if (!(playerLean > playerLeanLimit))
			{
				playerLean += moveSpeed;
				hoverCar[0].hoverCarModel->RotateLocalZ(moveSpeed);
				myCamera->RotateLocalZ(-moveSpeed);
			}
			*/
		}
		if (myEngine->KeyHeld(rightKey))
		{
			float moveSpeed = kGameSpeed/fps;
			playerRotation += moveSpeed;
			vehicles[0].Model->RotateLocalY(moveSpeed);
			/*
			if (!(playerLean < -playerLeanLimit))
			{
				playerLean -= moveSpeed;
				hoverCar[0].hoverCarModel->RotateLocalZ(-moveSpeed);
				myCamera->RotateLocalZ(moveSpeed);
			}
			*/
		}

		Xcounter += 1 / fps;
		if (Xcounter > 3)
		{
			Xcounter = 0;
		}
		float playerYPos = 0;
		playerYPos = Hover(Xcounter);
		vehicles[0].Model->SetY(playerYPos/2);
		stringstream outText;
		stringstream playerCheckpoint;
		playerCheckpoint << PlayerRaceStateText(hoverCarProp[0].carRaceState);
		float playerSpeedReadout = sqrt(playerMoveZ*playerMoveZ);
		outText << "FPS: " << fps << "                   Speed:" << playerSpeedReadout;//outputs the fps and speed in the UI
		myFont->Draw(outText.str(), fontX, fontY, kBlue, kLeft, kVCentre);
		myFont->Draw(playerCheckpoint.str(), kbottomOfScreen + 10, 80, kRed, kLeft, kVCentre);
	}
	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}//end of main()
float DegreesToRadians(float degrees)
{
	const float kPI = 3.14159265359f;
	float PIover = kPI/180;
	float radians = degrees * PIover;

	return radians;
	/*
	This function converts an angle in degrees to an angle in radians so it can be used in calculations
	that require the angle in radians
	*/
}
float Hover(float Xcounter)
{
	const float kPi = 3.14159265359f;
    float playerY = sin(Xcounter);
	playerY *= 2;

	return playerY;
}
string PlayerRaceStateText(int raceState)
{
	string playerCheckpoint;
	if (raceState == 0)
	{
		playerCheckpoint = "Start";
	}
	else if (raceState == 1)
	{
		playerCheckpoint = "Checkpoint 1";
	}
	else if (raceState == 2)
	{
		playerCheckpoint = "Checkpoint 2";
	}
	else if (raceState == 3)
	{
		playerCheckpoint = "Checkpoint 3";
	}
	else if (raceState == 4)
	{
		playerCheckpoint = "Checkpoint 4";
	}
	else if (raceState == 5)
	{
		playerCheckpoint = "Checkpoint 5";
	}
	if (raceState == numberOfCheckpoints)
	{
		playerCheckpoint = "Finish";
	}
	
	return playerCheckpoint;
}
void DetectCollision(int kNumberOfWalls, int kNumberOfIsles, vector <Vehicle> &hoverCarProp, vector <wallstructure> wallProp, vector <islestructure> isleProp, float waterTankRadius, int kNumberOfWaterTanks, float kCarRadius)
{
		for (int i = 0; i < kNumberOfIsles; i++)
		{
			if ((vehicles[0].xPos > isleProp[i].minX && vehicles[0].xPos < isleProp[i].maxX) && (vehicles[0].zPos > isleProp[i].minZ && vehicles[0].zPos < isleProp[i].maxZ))
			{
				hoverCarProp[0].lastCarMoveState = hoverCarProp[0].carMoveState;
				hoverCarProp[0].carMoveState = Collision;
			}
		}
		for (int i = 0; i < kNumberOfWalls; i++)
		{
			if ((vehicles[0].xPos > wallProp[i].minX && vehicles[0].xPos < wallProp[i].maxX) && (vehicles[0].zPos > wallProp[i].minZ && vehicles[0].zPos < wallProp[i].maxZ))
			{
				hoverCarProp[0].lastCarMoveState = hoverCarProp[0].carMoveState;
				hoverCarProp[0].carMoveState = Collision;
			}
		}
		for (int i = 0; i < kNumberOfWaterTanks; i++)
		{
			float x,z;
			x = vehicles[0].xPos - waterTanks[i].xPos;
			z = vehicles[0].zPos - waterTanks[i].zPos;
			float collisionDist = sqrt(x*x + z*z);
			if (collisionDist < (kCarRadius + waterTankRadius))
			{
				hoverCarProp[0].lastCarMoveState = hoverCarProp[0].carMoveState;
				hoverCarProp[0].carMoveState = Collision;
			}
		}
}
void OpenTrack(string fileName, IMesh* checkpointMesh, IMesh* wallMesh, IMesh* hoverCarMesh, IMesh* isleMesh, IMesh* waterTankMesh)
{
	ifstream infile;
	infile.open(fileName);
	if (!infile)
	{
		infile.close();
		ofstream outfile;
		outfile.open(fileName);
		for (int i = 0; i < knumberOfObjects; i++)
		{
			outfile << "0" << endl;
		}
		outfile.close();
		infile.open(fileName);
	}
	string line;
	while (getline(infile, line))
	{
		raceTrackData.push_back(line);
	}
	
	numberOfVehicles = ReadObjectPositions(vehicles);
	for (int i = 0; i < numberOfVehicles; i++)
	{
		vehicles.at(i).Model = hoverCarMesh->CreateModel(vehicles.at(i).xPos, 0, vehicles.at(i).zPos);
		vehicles.at(i).Model->RotateX(vehicles.at(i).rotationX);
		vehicles.at(i).Model->RotateY(vehicles.at(i).rotationY);
		vehicles.at(i).Model->RotateZ(vehicles.at(i).rotationZ);
	}
	numberOfWalls = ReadObjectPositions(walls);
	for (int i = 0; i < numberOfWalls; i++)
	{
		walls.at(i).Model = wallMesh->CreateModel(walls.at(i).xPos, 0, walls.at(i).zPos);
		walls.at(i).Model->RotateX(walls.at(i).rotationX);
		walls.at(i).Model->RotateY(walls.at(i).rotationY);
		walls.at(i).Model->RotateZ(walls.at(i).rotationZ);
	}
	numberOfIsles = ReadObjectPositions(isles);
	for (int i = 0; i < numberOfIsles; i++)
	{
		isles.at(i).Model = isleMesh->CreateModel(isles.at(i).xPos, 0, isles.at(i).zPos);
		isles.at(i).Model->RotateX(isles.at(i).rotationX);
		isles.at(i).Model->RotateY(isles.at(i).rotationY);
		isles.at(i).Model->RotateZ(isles.at(i).rotationZ);
	}
	numberOfCheckpoints = ReadObjectPositions(checkpoints);
	for (int i = 0; i < numberOfCheckpoints; i++)
	{
		checkpoints.at(i).Model = checkpointMesh->CreateModel(checkpoints.at(i).xPos, 0, checkpoints.at(i).zPos);
		checkpoints.at(i).Model->RotateX(checkpoints.at(i).rotationX);
		checkpoints.at(i).Model->RotateY(checkpoints.at(i).rotationY);
		checkpoints.at(i).Model->RotateZ(checkpoints.at(i).rotationZ);
	}
	numberOfWaterTanks = ReadObjectPositions(waterTanks);
	for (int i = 0; i < numberOfWaterTanks; i++)
	{
		waterTanks.at(i).Model = waterTankMesh->CreateModel(waterTanks.at(i).xPos, 0, waterTanks.at(i).zPos);
		waterTanks.at(i).Model->RotateX(waterTanks.at(i).rotationX);
		waterTanks.at(i).Model->RotateY(waterTanks.at(i).rotationY);
		waterTanks.at(i).Model->RotateZ(waterTanks.at(i).rotationZ);
	}

	infile.close();
}
int ReadObjectPositions(vector <Objectstructure> &objects)
{
	int j = 0;
	int numberOfObjects = atoi(raceTrackData.at(startRead).c_str());
	endRead = startRead + numberOfObjects + 1;
	objects.resize(numberOfObjects);
	int commaPos = 0;
	for (int i = startRead+1; i < endRead; i++)
	{
		string searchData = raceTrackData.at(i).substr(commaPos, raceTrackData.at(i).length());
		commaPos = searchData.find(",");
		objects.at(j).xPos = stof(searchData.substr(0,commaPos));

		searchData = searchData.substr(commaPos+1, searchData.length());
		commaPos = searchData.find(",");
		objects.at(j).zPos = stof(searchData.substr(0,commaPos));

		searchData = searchData.substr(commaPos+1, searchData.length());
		commaPos = searchData.find(",");
		objects.at(j).rotationX = stof(searchData.substr(0,commaPos));

		searchData = searchData.substr(commaPos+1, searchData.length());
		commaPos = searchData.find(",");
		objects.at(j).rotationY = stof(searchData.substr(0,commaPos));

		searchData = searchData.substr(commaPos+1, searchData.length());
		commaPos = searchData.find(",");
		objects.at(j).rotationZ = stof(searchData.substr(0,commaPos));

		commaPos = 0;
		j++;
	}
	startRead = endRead;
	return numberOfObjects;
}
