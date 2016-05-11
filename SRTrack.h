/// Emil Hedemalm
/// 2016-05-01
/// Race-track. Generatable.

#include "Maps/Map.h"

class Mesh;
class Checkpoint;
class Model;
class RacingShipGlobal;

/// For when having only 1 track in at a time, this is the manipulatable one (active map).
class SRTrack : public Map
{
public:
	SRTrack();
	/// Makes this track active. - rendered, physical, loaded
	void MakeActive();

	// Generates field.
	void Generate();
	// Generates playable mesh field.
	Mesh * GenerateMesh();
	Mesh * GenerateWalls();

	/// Spawns player into map. Gives it input focus too? <- should change later
	Entity * SpawnPlayer();

	// EntityProperty for controlling last created player ship.
	RacingShipGlobal * rsg;

	/// Default 5.f
	float trackWidth;
private:

	enum {
		FORWARD,
		TURN_LEFT, TURN_LEFT_BACK, // For when ending
		TURN_RIGHT, TURN_RIGHT_BACK, 
		TURN_RIGHT_TO_MIDDLE, TURN_LEFT_TO_MIDDLE,
		TURN_REACHED, // Reached target dir.
		TURN_TO_START,
		ARCH_TO_START, // Maybe same as turn to start.
		TURN_TO_CENTER,
		STOP,
		ENDED,
	};
	enum
	{
		RANDOM_GENERATION,
		TRY_TO_END,
		SWING_TOWARDS_CENTER_Z,
		GO_TO_CENTER_Z,
		TURN_TO_CENTER_XZ,
		FINAL_CURVE,
	};
	int mode; // Starts with RANDOM_GENERATION

	// Normalized.
	Vector3f CurrDir();
	// Saved into X and Y, Normalized.
	Vector2f CurrDirXZ();
	int Forward();
	// If back is true, is trying to go to (-1,0,0) dir.
	int TurnLeft(bool back = false, Vector2f clampDir = Vector2f(0,0));
	int TurnRight(bool back = false, Vector2f clampDir = Vector2f(0,0));
	int TurnToStart(); // Final turn-n-forwards.
	int TryEndIt();
	int ArchToStart();
	/// Returns true if still need iterative calls to this function.
	bool AddElevationToAvoidCollisions();
	int TurnToCenter();
	int GoToCenterZ();
	int FinalStraight();

	float itLength; // Iteration length. Default trackWidth * 2
	int left, right, forward, iterations;
	int attemptsToEnd;
	int archIterations;
	bool archLeft;
	int archSegments;
	Vector3f archStart, archEnd; // For longer arches, reset to 0,0,0 once arch completes or is canceled.

	List<Entity*> trackEntities;
	Entity * trackEntity, * wallEntity; // The actual road.
	Model * trackModel, * wallModel;
	Mesh * mesh, * wallMesh;
	List<Vector3f> points;
	List<Vector3f> leftSides, rightSides;
	List<Checkpoint> checkpoints;
};

extern SRTrack * track;


class Checkpoint 
{
public:
	String name; // str
	Vector3f pos; // vec3
};
