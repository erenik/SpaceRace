/// Emil Hedemalm
/// 2016-05-18
/// To turn ships and stuffs to align with the racing-track.

#include "Entity/EntityProperty.h"

class MagneticProperty : public EntityProperty 
{
public:
	MagneticProperty(Entity * owner);
	static int ID() {return 1;};
	float timeOutsideTrack; // In seconds
};