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

//	Physics.collisionResolver = CollisionResolver::CUSTOM_SPACE_RACE_PUSHBACK;
//	Physics.integrator = Integrator::SPACE_RACE_CUSTOM_INTEGRATOR;

#include "SRIntegrator.h"

/** All entities sent here should be fully dynamic! 
	Kinematic ones may or may not work (consider adding own integration function).
*/
void SRIntegrator::IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds)
{
	FirstPersonIntegrator::IntegrateDynamicEntities(dynamicEntities, timeInSeconds);
}
/** All entities sent here should be fully kinematic! 
	If not subclassed, the standard IntegrateEntities is called.
*/
void SRIntegrator::IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds)
{
	FirstPersonIntegrator::IntegrateKinematicEntities(kinematicEntities, timeInSeconds);
}
