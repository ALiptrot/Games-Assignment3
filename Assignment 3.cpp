// Assignment 3.cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <iostream>
#include <sstream>
#include <math.h>
#include <vector>
using namespace tle;
using namespace std;
struct checkpointstructure
{
	IModel* checkpointModel;
	float XPos;
	float ZPos;
	bool isPastCheckpoint;
	int number;
};
struct islestructure
{
	IModel* isleModel;
	float xPos;
	float zPos;
	float maxX;
	float minX;
	float maxZ;
	float minZ;
};
struct wallstructure
{
	IModel* wallModel;
	float xPos;
	float zPos;
	float maxX;
	float minX;
	float maxZ;
	float minZ;
};
enum MoveState
{
	Stationary = 0,
	Reverse = 1,
	Forward = 2,
	Collision = 3,
};
enum RaceState
{
	Start = 0,
	Checkpoint1 = 1,
	Finish = 3,
};
struct Vehicle
{
	IModel* hoverCarModel;//vehicle model
	MoveState carMoveState;//the move state (Stationary, Reverse or Forward)
	MoveState lastCarMoveState;
	float XPos;//the x position of the model
	float ZPos;//the z position of the model
	float moveXOld;//the old movement x units of the model
	float moveZOld;//the old movement z units of the model
	float rotation;//the rotation value of the model
	float moveX;//the current movement x units of the model
	float moveZ;//the current movement z units of the model
	int carRaceState;//ie start, checkpoint1, finish
	int lastCarRaceState;
};

float DegreesToRadians(float degrees);
float Hover(float Xcounter);
string PlayerRaceStateText(int raceState);
void DetectCollision(Vehicle hoverCar[], islestructure isles[], wallstructure walls[], int kNumberOfWalls, int kNumberOfIsles);
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

		//constants
		const int kNumberOfCheckpoints = 2;//number of checkpoints on the track
		const int kNumberOfIsles = 4;//number of isles on the track
		const int kNumberOfWalls = 3;//number of walls on the track
		const int kbottomOfScreen = 666;//bottom of the screen to draw the UI and text
		const float kMaxSpeedForward = 120*120;//top speed of a vehicle going forwards
		const float kMaxSpeedBackward = 60*60;//top speed of a vehicle going backwards
		const int kGameSpeed = 50;//gamespeed
		const float playerLeanLimit = 40;
		const int kNumberOfCars = 1;//number of hovercars in the world
		const int kAccel = 2;//acceleration used in the formula to get the new speed of the vehicles
		const int kCheckpointRadius = 3;//radius of the collision for the inside of the checkpoint
		const int kCarRadius = 4.5;//radius of the hovercar
		const float kIsleLengthZFromOrigin = 4.72;
		const float kIsleLengthXFromOrigin = 0.1;
		const float kWallLengthZFromOrigin = 4;
		const float kWallLengthXFromOrigin = 1;
		
		//checkpoints
		IMesh* checkpointMesh = myEngine->LoadMesh("Checkpoint.x");
		checkpointstructure checkpoint[kNumberOfCheckpoints];
		checkpoint[0].checkpointModel = checkpointMesh->CreateModel(0,0,20);
		checkpoint[0].XPos = 0; checkpoint[0].ZPos = 20;
		checkpoint[1].checkpointModel = checkpointMesh->CreateModel(0,0,100);
		checkpoint[1].XPos = 0; checkpoint[1].ZPos = 100;
		for (int i = 0; i < kNumberOfCheckpoints; i++)
		{
			checkpoint[i].isPastCheckpoint = true;
			checkpoint[i].number = i+1;
		}

		//isles
		IMesh* isleMesh = myEngine->LoadMesh("IsleStraight.x");
		islestructure isles[kNumberOfIsles];
		isles[0].xPos = -10, isles[0].zPos = 40;
		isles[1].xPos = 10, isles[1].zPos = 40;
		isles[2].xPos = 10, isles[2].zPos = 53;
		isles[3].xPos = -10, isles[3].zPos = 53;

		for (int i = 0; i < kNumberOfIsles; i++)
		{
			isles[i].maxX = isles[i].xPos + kIsleLengthXFromOrigin + kCarRadius, isles[i].minX = isles[i].xPos - kIsleLengthXFromOrigin - kCarRadius; 
			isles[i].maxZ = isles[i].zPos + kIsleLengthZFromOrigin + kCarRadius, isles[i].minZ = isles[i].zPos - kIsleLengthZFromOrigin - kCarRadius;
			isles[i].isleModel = isleMesh->CreateModel(isles[i].xPos,0,isles[i].zPos);
		}

		//walls
		IMesh* wallMesh = myEngine->LoadMesh("Wall.x");
		wallstructure walls[kNumberOfWalls];
		walls[0].xPos = -10.5, walls[0].zPos = 46;
		walls[1].xPos = 9.5, walls[1].zPos = 46;
		walls[2].xPos = 30, walls[2].zPos = 30;

		for (int i = 0; i < kNumberOfWalls; i++)
		{
			walls[i].maxX = walls[i].xPos + kWallLengthXFromOrigin + kCarRadius, walls[i].minX = walls[i].xPos - kWallLengthXFromOrigin - kCarRadius; 
			walls[i].maxZ = walls[i].zPos + kWallLengthZFromOrigin + kCarRadius, walls[i].minZ = walls[i].zPos - kWallLengthZFromOrigin - kCarRadius;
			walls[i].wallModel = wallMesh->CreateModel(walls[i].xPos,0,walls[i].zPos);
		}

		//sky
		IMesh* skyMesh = myEngine->LoadMesh("Skybox 07.x");
		IModel* sky = skyMesh->CreateModel(0,-960,0);

		//floor
		IMesh* floorMesh = myEngine->LoadMesh("ground.x");
		IModel* floor = floorMesh->CreateModel();

		//Hovercars
		Vehicle hoverCar[kNumberOfCars];
		for (int i = 0; i < kNumberOfCars; i++)
		{
			hoverCar[i].rotation = 0;
			hoverCar[i].carMoveState = Stationary;
			hoverCar[i].moveX = 0;
			hoverCar[i].moveZ = 0;
			hoverCar[i].carRaceState = 0;
		}
		/*
		Initialising all the values in the hoverCar to 0, or for the moveState Stationary
		*/
		IMesh* hoverCarMesh = myEngine->LoadMesh("race2.x");
		for (int i = 0; i < kNumberOfCars; i++)
		{
			hoverCar[i].hoverCarModel = hoverCarMesh->CreateModel();
		}
		myCamera->SetPosition(0,12,-30);
		myCamera->AttachToParent(hoverCar[0].hoverCarModel);

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
		float collisionDist = 0;
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
		hoverCar[0].XPos = hoverCar[0].hoverCarModel->GetX();//gets the x position of the player
		hoverCar[0].ZPos = hoverCar[0].hoverCarModel->GetZ();//gets the z position of the player


		float mouseMoveX = myEngine->GetMouseMovementX();
		float mouseMoveY = myEngine->GetMouseMovementY();
		myCamera->RotateLocalY((kGameSpeed*mouseMoveX*3)/fps);
		myCamera->RotateLocalX((kGameSpeed*mouseMoveY*3)/fps);

		if (playerRotation >= 360)
		{
			playerRotation = 0;//sets the player rotation back to 0 so that the rotation value cannot go above 360 degrees
		}

		switch(hoverCar[0].carMoveState)
		{
		case Stationary:
			acceleration = 0;
			if (myEngine->KeyHeld(forwardKey))
			{
				acceleration = kAccel;
				hoverCar[0].carMoveState = Forward;
			}
			if (myEngine->KeyHeld(backwardKey))
			{
				acceleration = -kAccel;
				hoverCar[0].carMoveState = Reverse;
			}
			break;
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
					acceleration = 0;
				}
			}
			if (myEngine->KeyHeld(forwardKey))
			{
				acceleration = kAccel*2;
			}
			if (hoverCar[0].moveZ > 0)
			{
				hoverCar[0].carMoveState = Stationary;
			}
			break;
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
			if (hoverCar[0].moveZ < 0)
			{
				hoverCar[0].carMoveState = Stationary;
			}
			break;
		case Collision:
			if (hoverCar[0].moveX > 0 || hoverCar[0].moveZ > 0)
			{
				float tempX = hoverCar[0].moveX;
				float tempZ = hoverCar[0].moveZ;
				hoverCar[0].moveX = -tempX/2;
				hoverCar[0].moveZ = -tempZ/2;
				if (hoverCar[0].lastCarMoveState == Forward)
				{
					hoverCar[0].carMoveState = Reverse;
				}
				if (hoverCar[0].lastCarMoveState == Reverse)
				{
					hoverCar[0].carMoveState = Forward;
				}
			}
			else
			{
				hoverCar[0].carMoveState = Stationary;
			}
			break;
		}

		DetectCollision(hoverCar, isles, walls, kNumberOfWalls, kNumberOfIsles);

		float playerVectorAngle = 90 - playerRotation;
		playerVectorAngle = DegreesToRadians(playerVectorAngle);//converts the angle of the car rotation to radians
		playerMoveX = cos(playerVectorAngle) * (kGameSpeed*hoverCar[0].moveX);//calculates the X direction vector
		playerMoveZ = sin(playerVectorAngle) * (kGameSpeed*hoverCar[0].moveZ);//calculates the Z direction vector
		hoverCar[0].hoverCarModel->MoveX(playerMoveX/fps);//moves the car in the x axis according to the current fps
		hoverCar[0].hoverCarModel->MoveZ(playerMoveZ/fps);//moves the car in the z axis according to the current fps

		hoverCar[0].moveXOld = hoverCar[0].moveX;
		hoverCar[0].moveZOld = hoverCar[0].moveZ;
		hoverCar[0].moveX = hoverCar[0].moveXOld + acceleration * frameTime;//calculating the new 
		hoverCar[0].moveZ = hoverCar[0].moveZOld + acceleration * frameTime;

		

		for (int i = 0; i < kNumberOfCheckpoints; i++)
		{
			float x,z;
			x = hoverCar[0].XPos - checkpoint[i].XPos;
			z = hoverCar[0].ZPos - checkpoint[i].ZPos;
			collisionDist = sqrt(x*x + z*z);
			if (collisionDist < (kCarRadius + kCheckpointRadius))
			{
				checkpoint[i].isPastCheckpoint = false;
			}
			else if (collisionDist > (kCarRadius + kCheckpointRadius) && !checkpoint[i].isPastCheckpoint)
			{
				checkpoint[i].isPastCheckpoint = true;
				hoverCar[0].lastCarRaceState = hoverCar[0].carRaceState;
				hoverCar[0].carRaceState = checkpoint[i].number;
				if (hoverCar[0].carRaceState - hoverCar[0].lastCarRaceState != 1)
				{
					hoverCar[0].carRaceState = hoverCar[0].lastCarRaceState;
				}
			}
		}


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
			myCamera->SetPosition(hoverCar[0].XPos,12,hoverCar[0].ZPos-30);
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
			hoverCar[0].hoverCarModel->RotateLocalY(-moveSpeed);
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
			hoverCar[0].hoverCarModel->RotateLocalY(moveSpeed);
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
		hoverCar[0].hoverCarModel->SetY(playerYPos/2);
		stringstream outText;
		stringstream playerCheckpoint;
		playerCheckpoint << PlayerRaceStateText(hoverCar[0].carRaceState);
		float playerSpeedReadout = sqrt(playerMoveZ*playerMoveZ);
		outText << "FPS: " << fps << "                   Speed:" << playerSpeedReadout;//outputs the fps and speed in the UI
		myFont->Draw(outText.str(), fontX, fontY, kBlue, kLeft, kVCentre);
		myFont->Draw(playerCheckpoint.str(), kbottomOfScreen + 10, 80, kRed, kLeft, kVCentre);
	}
	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
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
		playerCheckpoint = "Finish";
	}
	
	return playerCheckpoint;
}
void DetectCollision(Vehicle hoverCar[], islestructure isles[], wallstructure walls[], int kNumberOfWalls, int kNumberOfIsles)
{
	for (int i = 0; i < kNumberOfIsles; i++)
		{
			if ((hoverCar[0].XPos > isles[i].minX && hoverCar[0].XPos < isles[i].maxX) && (hoverCar[0].ZPos > isles[i].minZ && hoverCar[0].ZPos < isles[i].maxZ))
			{
				hoverCar[0].lastCarMoveState = hoverCar[0].carMoveState;
				hoverCar[0].carMoveState = Collision;
			}
		}
		for (int i = 0; i < kNumberOfWalls; i++)
		{
			if ((hoverCar[0].XPos > walls[i].minX && hoverCar[0].XPos < walls[i].maxX) && (hoverCar[0].ZPos > walls[i].minZ && hoverCar[0].ZPos < walls[i].maxZ))
			{
				hoverCar[0].lastCarMoveState = hoverCar[0].carMoveState;
				hoverCar[0].carMoveState = Collision;
			}
		}
}
