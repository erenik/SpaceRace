/// Emil Hedemalm
/// 2016-05-18
/// o-o

#include "SRIntegrator.h"
#include "Properties/MagneticProperty.h"
#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "CC.h"

#include "SRTrack.h"


/** All entities sent here should be fully dynamic! 
	Kinematic ones may or may not work (consider adding own integration function).
*/
void SRIntegrator::IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds)
{
	FirstPersonIntegrator::IntegrateDynamicEntities(dynamicEntities, timeInSeconds);
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		// check for magnetics.
		Entity * entity = dynamicEntities[i];
		if (entity->physics->state & CollisionState::IN_REST)
			continue;
		MagneticProperty * magProp = entity->GetProperty<MagneticProperty>();
		if (magProp)
		{
			// Rotate a bit.
			// Raycast to floor on location, at forward + 1 and side + 1, then rotate so that the ship is flat to the level.
			Vector3f upVec = entity->rotationMatrix.GetColumn(1),
				forwardVec = -entity->rotationMatrix.GetColumn(2),
				rightVec = entity->rotationMatrix.GetColumn(0);
			/// New rolling, check nearest points of track.
			TrackPoint * tp = track->NearestPoint(entity->worldPosition);
			if (!tp || tp->next == 0)
				continue;
			TrackPoint * next = tp->next->next;
			if (!next)
				continue;
			
			/// Desired forward, and up-right vectors of the track.
			Vector3f desForward = (next->pos - tp->pos).NormalizedCopy(),
				desUp = tp->up,
				desRight = tp->right;

			float desForwardDotCurrForward = desForward.NormalizedCopy().DotProduct(forwardVec);
			Vector3f desForwardInCurrDir = desForwardDotCurrForward * desForward; // Less than 1			
			float forwardDotDesUp = forwardVec.DotProduct(desUp);

			// Pitch down or up?
			Vector3f forwardProjected = desForwardDotCurrForward * desForward;
			Vector3f diffForwardCurrForward = forwardVec - desForward;
			/// Pitch needed?
			float pitchNeeded = diffForwardCurrForward.DotProduct(upVec);
//			if (AbsoluteValue(pitchNeeded) > 0.1f)
	//			std::cout<<"\nPitch needed: "<<pitchNeeded;
			if (desForwardDotCurrForward > 0)
				pitchNeeded *= -1;
			// Rotate slightly around side-vector.
			entity->physics->angularVelocity  -= rightVec * pitchNeeded * timeInSeconds * 0.2f;
			
			/// Barrel-roll.
			Vector3f diffUpCurrUp = upVec - desUp;
			float barrelNeeded = diffUpCurrUp.DotProduct(rightVec);
	//		if (AbsoluteValue(barrelNeeded) > 0.1f)
	//			std::cout<<"\nBarrel needed: "<<barrelNeeded;
			entity->physics->angularVelocity += forwardVec * barrelNeeded * timeInSeconds * 0.1f;

			/// Add some velocity towards the next and previous node, acting as magnetism down towards the field?
			entity->physics->velocity -= desUp * timeInSeconds * 5.f + (entity->worldPosition - tp->pos).NormalizedCopy() * 5.f * timeInSeconds;

			/// Old rolling
			/*
			Ray ray(entity->worldPosition, -upVec); // 0 - side, 1 - up, 2 - forward
			ray.collisionFilter = CC_TRACK;
			List<Intersection> iSecs = Physics.Raycast(ray);
			float distBelow = iSecs.Size()? iSecs[0].distance : 0.f;
			// Ray again.
			ray.start += forwardVec;
			iSecs = PhysicsMan.Raycast(ray);
			float distInFront = iSecs.Size()? iSecs[0].distance : 0.f;
			ray.start = entity->worldPosition + rightVec;
			iSecs = PhysicsMan.Raycast(ray);
			float distSide = iSecs.Size()? iSecs[0].distance : 0.f;

			float distPitch = distInFront - distBelow;
			float distRoll = distSide - distBelow;

			// Rotate accordingly for pitch.
			if (distInFront && distBelow && AbsoluteValue(distPitch) < 2.f && distBelow < 5.f)
			{
				// Rotate slightly around side-vector.
				entity->physics->angularVelocity  += rightVec * distPitch * timeInSeconds * 0.2f;
			}
			// Rotate accordingly for roll.
			if (distSide && distBelow && AbsoluteValue(distRoll) < 2.f  && distBelow < 5.f)
			{
				// Rotate slightly around side-vector.
				entity->physics->angularVelocity  -= forwardVec * distRoll * timeInSeconds * 0.1f;
			}


			/// If not above anything, rotate to level-plane.
			if (distBelow == 0)
			{
				magProp->timeOutsideTrack += timeInSeconds;
				if (magProp->timeOutsideTrack > 2.f)
				{
					/// If nothing detected, rotate so that the ship will be level again, by rolling?
					if (forwardVec.y > 0.2f)
					{
						entity->physics->angularVelocity += forwardVec.y * rightVec * timeInSeconds * 0.1f;
					}
					/// Right-vec not aligned with XZ plane? Roll until it is.
					if (AbsoluteValue(rightVec.y) > 0.2f)
					{
						entity->physics->angularVelocity += rightVec.y * forwardVec * timeInSeconds * 0.1f;	
					}
					if (upVec.y < 0.8f)
					{
						// Barrel-roll up.
						entity->physics->angularVelocity += (-1.f + upVec.y) * forwardVec * timeInSeconds * 0.1f;	
					}
				}
			}
			else 
			{
				magProp->timeOutsideTrack = 0;
			}
			*/

			// Same for roll.

			// If not close to a surface... rotate to.. stuff?
//			entity->physics->angularVelocity.x = 0.00f;
		}
	}
}

/** All entities sent here should be fully kinematic! 
	If not subclassed, the standard IntegrateEntities is called.
*/
void SRIntegrator::IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds)
{
	FirstPersonIntegrator::IntegrateKinematicEntities(kinematicEntities, timeInSeconds);
}



