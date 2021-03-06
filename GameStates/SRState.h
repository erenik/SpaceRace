/// Emil Hedemalm
/// 2014-02-02
/// Middle-class that provides general functionality like retrieving active players via the game session, etc.

#ifndef SPACE_RACE_GAME_STATE_H
#define SPACE_RACE_GAME_STATE_H

#include "AppStates/AppState.h"
#include "AppStates/AppStates.h"
#include "Game/GameConstants.h"
#include "../Network/SRSession.h"
#include "../SRPlayer.h"

enum 
{
	LOW,
	MEDIUM,
	HIGH,
};

extern int graphicsPreset;

class SRState : public AppState
{
public:
	/// Function when entering this state, providing a pointer to the previous StateMan.
	virtual void OnEnter(AppState * previousState);
	/// Main processing function, using provided time since last frame.
	virtual void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	virtual void OnExit(AppState * nextState);
	/// o-o
	virtual void ProcessMessage(Message * message);
	virtual void CreateDefaultBindings();

	/*
	/// Retrieves the active gaming session (this assumes only one is active at a time).
	SRSession * GetSession();
	/// Returns player for target index. Returns NULL for invalid indices.
	SRPlayer* GetPlayer(int byIndex);
	/// Gets index for target player. Returns -1 for invalid players.
	int GetPlayerIndex(SRPlayer * player);
	/// Retrieves a list of active players.
	List<SRPlayer*> GetPlayers();
	/// Returns list of all local players.
	List<SRPlayer*> GetLocalPlayers();


	/// Creates camera key-bindings! :)
	void CreateCameraBindings();
	/// Call this in the ProcessMessage() if you want the base state to handle camera movement! Returns true if the message was indeed a camera-related message.
	bool HandleCameraMessages(String message);
	/// Modes for the camera. Creates and makes the camera active if it wasn't already.
	void SetCameraTrackEntity(Entity * entity);
	void SetCameraFreeFly();	
	void SetCameraOrthogonal();
	void SetCameraProjection3D();
	/// Assume one main camera for this game. Add more cameras in your states if you're gonna do magic stuff.
	static Camera * mainCamera;
 */
private:

	void EvaluateLine(String line);
	/*
	/// Creates the camera.
	void CreateCamera();
	/// Registers it with the graphics-manager!
	void MakeCameraActive();
	/// Resets velocities and stuff
	void ResetCamera();
	*/
};

#endif