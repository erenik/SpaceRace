/// Emil Hedemalm
/// 2016-05-01
/// o-o

#include "Physics/CollisionResolvers/FirstPersonCR.h"

class SRCR : public FirstPersonCR
{
public:
	/// Returns false if the colliding entities are no longer in contact after resolution.
	virtual bool ResolveCollision(Collision & c);

};
