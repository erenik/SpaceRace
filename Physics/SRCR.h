/// Emil Hedemalm
/// 2016-05-01
/// o-o

#include "Physics/CollisionResolver.h"

class SRCR : public CollisionResolver 
{
public:
	/// Returns false if the colliding entities are no longer in contact after resolution.
	virtual bool ResolveCollision(Collision & c);

};
