/// Emil Hedemalm
/// 2016-05-01
/// o-o

#include "SRCR.h"

// Implement it.
/// Returns false if the colliding entities are no longer in contact after resolution.
bool SRCR::ResolveCollision(Collision & c)
{
	return FirstPersonCR::ResolveCollision(c);
	// Sup.
	return false;
}

