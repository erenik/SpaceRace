/// Emil Hedemalm
/// 2016-05-01
/// Race-track. Generatable.

#include "Maps/Map.h"

class Mesh;
class Checkpoint;
class Model;
class RacingShipGlobal;

class TrackPoint
{
public:
	TrackPoint(ConstVec3fr);
	TrackPoint(float x, float y, float z);
	void Nullify();
	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);
	/// Resets left/rightside to be pos.
	void CenterSides();
	Vector3f pos, up, right;
	/// Points left and ride marking end of track, beginning of wall.
	Vector3f leftSide, rightSide, leftSideWall, rightSideWall; // Wall being upper ledge of wall.
	TrackPoint * next;
};

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
	Vector3f SpawnPosition();

	/// Spawns player into map. Gives it input focus too? <- should change later
	Entity * SpawnPlayer();

	/// Oh yeah.
	virtual bool Save(String toFile);
	/// Loads and makes active.
	virtual bool Load(String fromFile);

	/// o-o
	TrackPoint * NearestPoint(ConstVec3fr pos);

	// EntityProperty for controlling last created player ship.
	RacingShipGlobal * rsg;

	/// Default 5.f
	float trackWidth;
	/// Iteration length. o-o
	/// Default 10.f
	float itLength; 
	/// Default 1.0?
	float turnTiltRatio;
	/// Starts at 0, after long turns, decreases slowly to 0.
	float degreesTurnRight;
	/// Max degrees turn per segment. Default 15?
	float maxTurnPerSeg;
private:

	enum {
		FORWARD,
		TURN_LEFT, TURN_LEFT_BACK, // For when ending
		TURN_RIGHT, TURN_RIGHT_BACK, 
		TURN_RIGHT_TO_MIDDLE, TURN_LEFT_TO_MIDDLE,
		TURN_REACHED, // Reached target dir.
		TURN_NOT_REACHED_YET,
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
	/// Called after giving instructions (Forward, TurnLeft, etc.). Calculates new pieces to properly downscale heavy curves etc. so tilts are properly calculated later.
	int PlaceNextPoint();
	int Forward();
	int TurnLeft();
	int TurnRight();
	/// Returns TURN_REACHED upon success.
	int TurnTo(ConstVec3fr dir);
	int TurnToStart(); // Final turn-n-forwards.
	int TryEndIt();
	int ArchToStart();
	/// Returns true if still need iterative calls to this function.
	bool AddElevationToAvoidCollisions();
	int TurnToCenter();
	int GoToCenterZ();
	int FinalStraight();

	int left, right, forward, iterations;
	int attemptsToEnd;
	int archIterations;
	bool archLeft;
	int archSegments;
	TrackPoint * archStart, * archEnd; // For longer arches, reset to 0,0,0 once arch completes or is canceled.

	List<Entity*> trackEntities;
	Entity * trackEntity, * wallEntity; // The actual road.
	Model * trackModel, * wallModel;
	Mesh * mesh, * wallMesh;
	List<TrackPoint *> points;
	List<Checkpoint> checkpoints;
};

extern SRTrack * track;


class Checkpoint 
{
public:
	String name; // str
	Vector3f pos; // vec3
};
