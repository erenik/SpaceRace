/// Emil Hedemalm
/// 2016-05-01
/// o-o

#include "Physics/Integrators/FirstPersonIntegrator.h"

class SRIntegrator : public FirstPersonIntegrator
{
public:
		/** All entities sent here should be fully dynamic! 
		Kinematic ones may or may not work (consider adding own integration function).
	*/
	virtual void IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds);
	/** All entities sent here should be fully kinematic! 
		If not subclassed, the standard IntegrateEntities is called.
	*/
	virtual void IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds);

};


