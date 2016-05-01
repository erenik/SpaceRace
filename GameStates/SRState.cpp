/// Emil Hedemalm
/// Rework of 2014-02-02, 2016-05-01
/// Middle-class that provides general functionality like retrieving active players via the game session, etc.

#include "SRState.h"
#include "SRDirectories.h"
#include "ShipManager.h"

#include "SRTrack.h"
#include "Physics/SRCR.h"
#include "Physics/SRIntegrator.h"

#include "Input/Action.h"
#include "Input/Keys.h"
#include "Graphics/Camera/CameraUtil.h"
#include "Graphics/Camera/Camera.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/Messages/GraphicsMessages.h"

#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"

#include "StateManager.h"
#include "Application/Application.h"
#include "Network/NetworkManager.h"
#include "Game/GameVariableManager.h"
#include "StateManager.h"
#include "Audio/TrackManager.h"

Camera * mapPreviewCamera = 0, * firstPersonCamera = 0, * activeCamera = 0;

void SetApplicationDefaults()
{
	Application::name = "Space Race";
	TextFont::defaultFontSource = "img/fonts/font3.png";
};
void RegisterStates()
{
	SRState * srs = new SRState();
	StateMan.RegisterState(srs);
	StateMan.QueueState(srs);
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void SRState::OnEnter(AppState * previousState)
{
//	LoadPreferences();
	SRSession * srs = new SRSession("Game name");
	NetworkMan.AddSession(srs);
	if (!ShipManager::IsAllocated()){
		ShipManager::Allocate();
		ShipMan.LoadFromDirectory(SHIP_DIR);
	}
	GameVars.CreateInt("Laps", 3);
	QueuePhysics(new PMSet(new SRCR()));
	QueuePhysics(new PMSet(new SRIntegrator()));
	QueuePhysics(new PMSet(PT_GRAVITY, -250.f));
	QueueGraphics(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));
	QueueGraphics(new GraphicsMessage(GM_CLEAR_UI));
	TrackMan.CreateTrack("Bitsurf", "sound/bgm/SpaceRace/2013-08-26_Bitsurf.ogg", "Race");
	TrackMan.CreateTrack("Space race", "sound/bgm/SpaceRace/2013-03-25 Space race.ogg", "Race");
	TrackMan.CreateTrack("Wapp", "sound/bgm/SpaceRace/2014-02-18_Wapp.ogg", "Race");

	// Set a default camera.
	if (!mapPreviewCamera)
		mapPreviewCamera = CameraMan.NewCamera("MapPreviewCamera", true);
	if (!firstPersonCamera)
		firstPersonCamera = CameraMan.NewCamera("FirstPersonCamera", true);
//	cameras.Add(mapPreviewCamera);
//	cameras.Add(firstPersonCamera);

	QueueGraphics(new GMSetCamera(mapPreviewCamera, CT_ROTATION, Vector3f()));
	QueueGraphics(new GMSetCamera(mapPreviewCamera, CT_DISTANCE_FROM_CENTER_OF_MOVEMENT, 3.f));
	QueueGraphics(new GMSetCamera(firstPersonCamera, CT_TRACKING_MODE, TrackingMode::FOLLOW_AND_LOOK_AT));
	QueueGraphics(new GMSetCamera(firstPersonCamera, CT_DISTANCE_FROM_CENTER_OF_MOVEMENT, 0.f));
	QueueGraphics(new GMSetCamera(firstPersonCamera, CT_ROTATIONAL_SMOOTHNESS, 0.000001f));
	QueueGraphics(new GMSetCamera(firstPersonCamera, CT_SMOOTHING, 0.05f)); 

	// o-o
	activeCamera = mapPreviewCamera;
	QueueGraphics(new GMSetCamera(mapPreviewCamera));

	// Create a dummy entity.
	MapMan.CreateEntity("Lall", ModelMan.GetModel("obj/cube.obj"), TexMan.GetTexture("0xFFFFFFFF"));

}

/// Main processing function, using provided time since last frame.
void SRState::Process(int timeInMs)
{

}
/// Function when leaving this state, providing a pointer to the next StateMan.
void SRState::OnExit(AppState * nextState)
{

}

void SRState::CreateDefaultBindings()
{
	InputMapping * mapping = &this->inputMapping;
	List<Binding*> & bindings = mapping->bindings;
	bindings.Add(CreateDefaultCameraBindings());
	bindings.AddItem(new Binding(Action::FromString("OpenStatusScreen"), List<int>(KEY::CTRL, KEY::S)));
	mapping->bindings.Add(new Binding(Action::FromString("ToggleAutorun"), KEY::R));
	mapping->bindings.Add(new Binding(Action::FromString("ToggleHeal"), KEY::H));
	bindings.AddItem((new Binding(Action::FromString("NextTarget"), KEY::TAB))->SetActivateOnRepeat(true));
	bindings.AddItem((new Binding(Action::FromString("PreviousTarget"), List<int>(KEY::SHIFT, KEY::TAB)))->SetActivateOnRepeat(true));
	bindings.AddItem(new Binding(Action::FromString("Interact"), KEY::ENTER));
	bindings.AddItem(new Binding(Action::FromString("OpenInputLine"), KEY::SPACE));
	bindings.AddItem(new Binding(Action::FromString("TargetSelf"), KEY::F1));
	bindings.AddItem(new Binding(Action::FromString("Cancel/Escape"), KEY::ESCAPE)); // Mainly removing main target.
	bindings.AddItem(new Binding(Action::FromString("OpenMainMenu"), KEY::PLUS));
	bindings.AddItem(new Binding(Action::FromString("OpenMainMenu"), KEY::F));
}

void SRState::ProcessMessage(Message * message)
{

	String msg = message->msg;
	ProcessCameraMessages(msg, activeCamera);
	switch(message->type)
	{
		case MessageType::SET_STRING:
		{
			SetStringMessage * ssm = (SetStringMessage*) message;
			if (msg == "SetInputLine")
			{
				std::cout<<"\nSetInputLine: "<<ssm;
				/// Good. Evaluate it.
				String input = ssm->value;
				EvaluateLine(input);
				/// Hide input box thingy.
				QueueGraphics(new GMPopUI("UIInputLine", 0));
			}
			break;
		}
		case MessageType::STRING:
			if (msg == "OpenInputLine")
			{
				QueueGraphics(new GMPushUI("gui/UIInputLine.gui"));
				/// Make it active for input?
				QueueGraphics(new GMSetUIs("InputLine", GMUI::STRING_INPUT_TEXT, ""));
				QueueGraphics(new GMSetUIb("InputLine", GMUI::ACTIVE, true));
			}
			break;
	}
};

void SRState::EvaluateLine(String line)
{
	if (!line.StartsWith("/"))
		return;
	if (line == "/gen")
	{
		if (!track)
			track = new SRTrack();
		track->Generate();
		track->GenerateMesh();
		// Redner it?
		track->MakeActive();
	}
}


/*
#include "SpaceRaceGameState.h"
#include "Network/NetworkManager.h"
#include "Network/Session/SessionTypes.h"
#include "Network/Session/GameSessionTypes.h"
#include "Input/Keys.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"

/// Retrieves the active gaming session (this assumes only one is active at a time).
SRSession * SpaceRaceGameState::GetSession()
{
	/// Gets session from network-manager. If no session exists, it will create one.
	Session * s = NetworkMan.GetSession(SessionType::GAME, GameSessionType::SPACE_RACE);
	assert(s);
	return (SRSession*)s;
}

/// 
SRPlayer* SpaceRaceGameState::GetPlayer(int byIndex)
{
	SRSession * s = GetSession();
	assert(s);
	return s->GetPlayer(byIndex);
}

/// Gets index for target player.
int SpaceRaceGameState::GetPlayerIndex(SRPlayer * player)
{
	List<SRPlayer*> players = GetPlayers();
	for (int i = 0; i <  players.Size(); ++i){
		if (players[i] == player)
			return i;
	}
	return -1;
}

/// Retrieves a list of active players.
List<SRPlayer*> SpaceRaceGameState::GetPlayers()
{
	SRSession * s = GetSession();
	assert(s && "No valid session to fetch players from D:");
	return s->GetPlayers();	
}

/// Returns list of all local players.
List<SRPlayer*> SpaceRaceGameState::GetLocalPlayers()
{
	SRSession * s = GetSession();
	assert(s && "No valid session to fetch players from D:");
	return s->GetLocalPlayers();	
}





Camera * SpaceRaceGameState::mainCamera = NULL;

/// Creates camera key-bindings! :)
void SpaceRaceGameState::CreateCameraBindings()
{
	inputMapping.CreateBinding("Up", KEY::E)->stringStopAction = "StopUp";
	inputMapping.CreateBinding("Down", KEY::Q)->stringStopAction = "StopDown";
	inputMapping.CreateBinding("Left", KEY::A)->stringStopAction = "StopLeft";
	inputMapping.CreateBinding("Right", KEY::D)->stringStopAction = "StopRight";
	inputMapping.CreateBinding("Forward", KEY::W)->stringStopAction = "StopForward";
	inputMapping.CreateBinding("Backward", KEY::S)->stringStopAction = "StopBackward";

	inputMapping.CreateBinding("Zoom in", KEY::PG_DOWN)->activateOnRepeat = true;
	inputMapping.CreateBinding("Zoom out", KEY::PG_UP)->activateOnRepeat = true;

	inputMapping.CreateBinding("ResetCamera", KEY::HOME);
	inputMapping.CreateBinding("SetProjection", KEY::F1);
	inputMapping.CreateBinding("SetOrthogonal", KEY::F2);
}
/// Call this in the ProcessMessage() if you want the base state to handle camera movement! Returns true if the message was indeed a camera-related message.
bool SpaceRaceGameState::HandleCameraMessages(String message)
{
	if (mainCamera == NULL)
		return false;
	if (message == "Up")
		mainCamera->Begin(Direction::UP);
	else if (message == "Down")
		mainCamera->Begin(Direction::DOWN);
	else if (message == "Left")
		mainCamera->Begin(Direction::LEFT);
	else if (message == "Right")
		mainCamera->Begin(Direction::RIGHT);
	else if (message == "Forward")
		mainCamera->Begin(Direction::FORWARD);
	else if (message == "Backward")
		mainCamera->Begin(Direction::BACKWARD);
	else if (message == "StopUp")
		mainCamera->End(Direction::UP);
	else if (message == "StopDown")
		mainCamera->End(Direction::DOWN);
	else if (message == "StopLeft")
		mainCamera->End(Direction::LEFT);
	else if (message == "StopRight")
		mainCamera->End(Direction::RIGHT);
	else if (message == "StopForward")
		mainCamera->End(Direction::FORWARD);
	else if (message == "StopBackward")
		mainCamera->End(Direction::BACKWARD);
	else if (message == "ResetCamera"){
		mainCamera->position = Vector3f();
	}
	else if (message == "SetProjection")
		SetCameraProjection3D();
	else if (message == "SetOrthogonal")
		SetCameraOrthogonal();
	else if (message == "Zoom in")
	{
		mainCamera->zoom = mainCamera->zoom * 0.95f - 0.01f;
#define CLAMP_DISTANCE ClampFloat(mainCamera->zoom, 0.01f, 10000.0f);
		CLAMP_DISTANCE;
	}
	else if (message == "Zoom out"){
		mainCamera->zoom = mainCamera->zoom * 1.05f + 0.01f;
		CLAMP_DISTANCE;
	}
	else
		return false;
	return true;
}


/// Modes for the camera. Creates and makes the camera active if it wasn't already.
void SpaceRaceGameState::SetCameraTrackEntity(Entity * entity)
{
	CreateCamera();
	MakeCameraActive();
	mainCamera->entityToTrack = entity;
}

void SpaceRaceGameState::SetCameraFreeFly()
{
	CreateCamera();
	MakeCameraActive();
	mainCamera->entityToTrack = NULL;
}

void SpaceRaceGameState::SetCameraOrthogonal()
{
	CreateCamera();
	MakeCameraActive();
	mainCamera->projectionType = Camera::ORTHOGONAL;
	mainCamera->entityToTrack = NULL;
	mainCamera->rotation = Vector3f();
	mainCamera->distanceFromCentreOfMovement = -5;
	/// Allow for some objects to be seen...
	mainCamera->zoom = 5.0f;
	mainCamera->scaleSpeedWithZoom = true;
	mainCamera->flySpeed = 1.f;
	mainCamera->position = Vector3f();
}

void SpaceRaceGameState::SetCameraProjection3D()
{
	CreateCamera();
	MakeCameraActive();
	/// Normal zoom, please..
	mainCamera->zoom = 0.1f;
	mainCamera->scaleSpeedWithZoom = false;
	mainCamera->flySpeed = 3.0f;
	mainCamera->projectionType = Camera::PROJECTION_3D;
}

/// Creates the camera.
void SpaceRaceGameState::CreateCamera()
{
	if (!mainCamera){
		mainCamera = new Camera();
		ResetCamera();
		SetCameraOrthogonal();
	}
}

void SpaceRaceGameState::MakeCameraActive()
{
	mainCamera->SetRatio(Graphics.width, Graphics.height);
	/// Make the camera active if not already.
	Graphics.QueueMessage(new GMSet(MAIN_CAMERA, mainCamera));
}

/// Resets velocities and stuff
void SpaceRaceGameState::ResetCamera()
{
	mainCamera->flySpeed = 3.0f;
}*/
