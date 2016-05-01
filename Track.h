/// Emil Hedemalm
/// 2016-05-01
/// Race-track. Generatable.

#include "Maps/Map.h"

class Checkpoint;

/// For when having only 1 track in at a time, this is the manipulatable one (active map).
class Track : public Map
{
public:
	List<Checkpoint*> checkpoints;
};
extern Track * track;


class Checkpoint 
{
public:
	String name; // str
	Vector3f pos; // vec3
};
