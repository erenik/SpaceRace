/// Emil Hedemalm
/// 2016-05-01
/// Race-track. Generatable.

#include "Maps/Map.h"

class Mesh;
class Checkpoint;
class Model;

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

	/// Default 5.f
	float trackWidth;
private:
	Entity * trackEntity; // The actual road.
	Model * trackModel;
	Mesh * mesh;
	List<Vector3f> points;
	List<Checkpoint> checkpoints;
};

extern SRTrack * track;


class Checkpoint 
{
public:
	String name; // str
	Vector3f pos; // vec3
};
