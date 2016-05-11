/// Emil Hedemalm
/// 2016-05-01
/// Race-track. Generatable.

#include "SRTrack.h"
#include "Mesh/EMesh.h"
#include "Entity/EntityManager.h"
#include "Model/ModelManager.h"
#include "Model/Model.h"
#include "Mesh/Mesh.h"
#include "TextureManager.h"
#include "Maps/MapManager.h"

#include "StateManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "PhysicsLib/PhysicsShape.h"
#include "PhysicsLib/PhysicsType.h"
#include "StateManager.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Random/Random.h"
Random trackRand;

SRTrack::SRTrack()
{
	trackWidth = 5.f;
	trackEntity = 0;
	trackModel = 0;
	mesh = 0;
	rsg = 0;

	wallEntity = 0;
	wallModel = 0;
	wallMesh = 0;
}
/// Makes this track active. - rendered, physical, loaded
void SRTrack::MakeActive()
{
	/// Make it active.
	MapMan.MakeActive(this);
	// Make track entity. 
	if (!trackEntity)
	{
		trackEntity = EntityMan.CreateEntity("Track", trackModel, TexMan.GetTexture("img/track/part_1_diffuse.png"));
		QueuePhysics(new PMSetEntity(trackEntity, PT_PHYSICS_SHAPE, ShapeType::MESH));
		trackEntities.AddItem(trackEntity);
	}
	else 
	{
		// Re-register
		QueuePhysics(new PMUnregisterEntity(trackEntity));
		QueuePhysics(new PMRegisterEntity(trackEntity));
	}
	if (!wallEntity)
	{
		wallEntity = EntityMan.CreateEntity("Walls", wallModel, TexMan.GetTexture("img/track/part_2_diffuse.png"));
		QueuePhysics(new PMSetEntity(wallEntity, PT_PHYSICS_SHAPE, ShapeType::MESH));
		trackEntities.AddItem(wallEntity);
	}
	// Add all.
	MapMan.AddEntities(trackEntities);
	// Add other relevant entities to represent the track.

	// Add scenery

	// Create a dummy entity.
	static int attempts = 0;
	MapMan.CreateEntity("Lall", ModelMan.GetModel("obj/cube.obj"), TexMan.GetTexture("0xFFFFFFFF"), Vector3f(float(attempts++), 0,0));

}

// Generates field.
void SRTrack::Generate()
{
	mode = RANDOM_GENERATION;
	points.Clear();
	// Quadratic
	enum {
		QUAD,
		CIRCLE,
		HIPPODROME,
		GENERATED_TRACK,
	};
	int stereotype = GENERATED_TRACK;
	if (stereotype == QUAD)
	{
		points.Add(Vector3f(0,0,0), Vector3f(50, 0, 0), Vector3f(50,0,50), Vector3f(0,0,50));
	}
	else if (stereotype == CIRCLE)
	{
		int segs = 25;
		for (int i = 0; i < segs; ++i)
		{
			Vector2f pos = Angle(i * PI * 2/ (float)segs).ToVector2f() * 50.f;
			points.AddItem(Vector3f(pos.x, 0, pos.y));
		}
	}
	else if (stereotype == HIPPODROME)
	{
		/// Hmm.. far stretch.
		float flatDist = 50.f;
		float width = 25.f; float halfWidth = width / 2;
		points.AddItem(Vector3f(0,0,0));
//		points.AddItem(Vector3f(flatDist,0,0));
		int segs = 8;
		// First half-circle.
		for (int i = 0; i < segs + 1; ++i)
		{
			Vector2f pos = Angle(-PI / 2.f + i * PI / (float)segs).ToVector2f() * halfWidth;
			points.AddItem(Vector3f(flatDist, 0, halfWidth) + Vector3f(pos.x, 0, pos.y));
		}
//		points.AddItem(Vector3f(flatDist, 0, width));
//		points.AddItem(Vector3f(0,0,width));
		for (int i = 0; i < segs; ++i)
		{
			Vector2f pos = Angle(PI / 2.f + i * PI/ (float)segs).ToVector2f() * halfWidth;
			points.AddItem(Vector3f(0, 0, halfWidth) + Vector3f(pos.x, 0, pos.y));
		}
	}
	else if (stereotype == GENERATED_TRACK)
	{
		/*
		Track generation 
		Add point in move direction, 10 meters in front 

		for turns, add slight inclination of 5 to 45 degrees depending on neighboring points
		more if continuous turn 

		for elevation (pitch inclination),  randomize similar to sideways algorithm, but use a 1 degree incline per iteration 
		up to max incline +/-

		if point is within trackwidth of other point, iterate until good point 
		if 5 iterations fail, use temporarily greater max limits
		if 5 further iterations fail, remove last point and redo it 
		if this fails, open manual editor and announce the problem 

		place checkpoint every 3 points or 5, can be customized */
		points.AddItem(Vector3f(0,0,0)); // Start
		itLength = trackWidth * 2;
		points.AddItem(Vector3f(itLength, 0,0));
		int which = FORWARD;
		left = right = 0;
		int iterations = 0;
		attemptsToEnd = 0;
		archIterations = 0;
		while(which != STOP)
		{
			switch(mode)
			{
				case RANDOM_GENERATION: 
					if (iterations > 100)
						mode = TRY_TO_END;
					break;
				case TRY_TO_END:
					which = TryEndIt();
					break;
				case GO_TO_CENTER_Z: which = GoToCenterZ(); break;
				case TURN_TO_CENTER_XZ: 
				{
					Vector3f currDir = CurrDir();
					if (currDir.x > 0.9f)
					{
						FinalStraight();
						which = STOP;
						break;
					}
					which = TURN_TO_START;
					break;
				}
				case FINAL_CURVE:	break;
			}
			switch(which)
			{
				case FORWARD: which = Forward(); break;
				case TURN_LEFT: which = TurnLeft(); break;
				case TURN_RIGHT: which = TurnRight(); break;
				case TURN_LEFT_BACK: which = TurnLeft(true, Vector2f(-1,0)); break;
				case TURN_RIGHT_BACK: which = TurnRight(true, Vector2f(-1,0)); break;
				case TURN_RIGHT_TO_MIDDLE: which = TurnLeft(true, Vector2f(0, -1)); break;
				case TURN_LEFT_TO_MIDDLE: which = TurnRight(true, Vector2f(0, 1)); break;
				case TURN_TO_START: which = TurnToStart(); break;
				case ARCH_TO_START: which = ArchToStart(); break;
				case TURN_TO_CENTER: which = TurnToCenter(); break;
			}
			++iterations;
			if (iterations > 250)
				break;
		}
		bool collisions = true;
		while(AddElevationToAvoidCollisions())
			;
	}
}

int SRTrack::TryEndIt()
{
	++attemptsToEnd;
	Vector3f last = points.Last();
	Vector3f currDir = CurrDir();
	// Check which side of the field we're on.
	bool rightFace = false, leftFace = false, beyond = false;
	if (currDir.z > 0)
		rightFace = true;
	else // Left side
		leftFace = true;
	if (last.x > 0)
		beyond = true;

	// Assume beyond in the beginning.
	if (beyond)
	{
		// Is aligned enough.
		if (currDir.DotProduct(Vector3f(-1,0,0)) > 0.95f)
			return FORWARD;
		if (rightFace)
			return TURN_RIGHT_BACK;
		if (leftFace)
			return TURN_LEFT_BACK;
	}
	if (attemptsToEnd <= 1)
	{
		mode = SWING_TOWARDS_CENTER_Z;
		return TURN_TO_CENTER;
	}
	// Not beyond? Can turn to middle and start going back.
	if (archIterations == 0)
	{
		std::cout<<"\nFinal arch start @ "<<last;
		archStart = last;
		archEnd = points[0];
		if (last.z > 0)
			archLeft = true;
		else 
			archLeft = false;
		
		archSegments = (archEnd - archStart).Length() / (trackWidth);

		mode = FINAL_CURVE;
	}
	return ARCH_TO_START;
	/*
	bool rightSide = false, leftSide = false;
	if (last.z > 0 && currDir.z > -0.9f)
		return TURN_RIGHT_TO_MIDDLE;
	else if (last.z < 0 && currDir.z < 0.9f)
		return TURN_LEFT_TO_MIDDLE;

	// Final stretch?
	if (AbsoluteValue(last.z) < 50.f)
	{
		return TURN_TO_START;
	}
	*/
}

int SRTrack::GoToCenterZ()
{
	/// Going to center Z.
	if (AbsoluteValue(points.Last().z) < 50.f)
	{
		mode = TURN_TO_CENTER_XZ;
		return TURN_TO_START;
	}
	return FORWARD;
}

int SRTrack::FinalStraight()
{
	Vector3f toStart = points[0] - points.Last();
	Vector3f dir = toStart.NormalizedCopy();
	float dist = (toStart).Length();
	int segs = dist / 20.f;
	float segLength = dist / (segs + 1);
	for (int i = 0; i < segs; ++i)
	{
		points.AddItem(points.Last() + dir * segLength);
	}
	return STOP;
}

int SRTrack::ArchToStart()
{
	/// Hmm.. far stretch.
	float archRadius = (archEnd - archStart).Length() / 2;
	Vector3f middle = (archEnd + archStart) * 0.5f;
	int segs = archSegments;
	++archIterations;
	// Half-circle.
	Vector2f pos = Angle((archLeft? PI / 2.f : -PI / 2.f) + (archLeft ? 1.f : -1.f) * archIterations * PI / (float)segs).ToVector2f() * archRadius;
	points.AddItem(middle + Vector3f(pos.x, 0, pos.y));
	if (archIterations + 1 >= segs)
		return STOP;
	return ARCH_TO_START;
}

int SRTrack::TurnToCenter()
{
	Vector3f last = points.Last();
	int turnResult = 0;
	if (last.z > 0)
		turnResult = TurnLeft(true, Vector2f(0,-1));
	else if (last.z < 0)
		turnResult = TurnRight(true, Vector2f(0,1));
	if (turnResult == TURN_REACHED)
		mode = GO_TO_CENTER_Z;
	return TURN_TO_CENTER;
}


bool SRTrack::AddElevationToAvoidCollisions()
{
	static List<int> lastCollisionPointIndices;
	// Fetch all points within collision distance.
	List<int> collisionPointIndices;
	for (int i = 0; i < points.Size(); ++i)
	{
		for (int j = i + 3; j < points.Size() - 3; ++j)
		{
			int index1 = i % points.Size(), index2 = j % points.Size();
			if (index1 == index2)
				continue;
			Vector3f p1 = points[index1], p2 = points[index2];
			float distXZ = (Vector2f(p2.x, p2.z) - Vector2f(p1.x, p1.z)).Length();
			float distY = AbsoluteValue(p2.y - p1.y);
			// Weighted, half distY, 1. of XZ
			float dist = Vector2f(distXZ, distY * 0.5f).Length();
			if (dist < trackWidth * 1.2f)
				collisionPointIndices.AddItem(index2);
		}
	}
	for (int i = 0; i < collisionPointIndices.Size(); ++i)
	{
		int pIndex = collisionPointIndices[i];
		int maxNeigh = 0;
		maxNeigh += lastCollisionPointIndices.Occurances(pIndex);
		ClampInt(maxNeigh, 0, 25);
		int neighboursToRaise = trackRand.Randf() * maxNeigh + 4.f;
		for (int j = -neighboursToRaise; j <= neighboursToRaise; ++j)
		{
			float dist = AbsoluteValue(j);
			float multiplier = 1 - (dist / (float) (neighboursToRaise+1)); // Linear?
			int index = (collisionPointIndices[i] + j + points.Size()) % points.Size();
			points[index].y += trackWidth * multiplier; 
		}
	}
	lastCollisionPointIndices.Add(collisionPointIndices);
	if (collisionPointIndices.Size())
	{
		std::cout<<"\nStill "<<collisionPointIndices.Size()<<" collision points.";
		return true;
	}
	return false;
}

int SRTrack::Forward()
{
	// Last 2 points, +
	points.AddItem(points.Last() + CurrDir() * itLength);
	if (trackRand.Randf() > 0.65f)
		return FORWARD;
	else // Turn!
	{
		if (trackRand.Randf() > 0.5f)
			return TURN_LEFT;
		else 
			return TURN_RIGHT;
	}
	/* 	
		when turning, move left or right by 5 degrees + 1 for each turn in same direction 
		keep same direction at 75% chance, otherwise straight 

		70% chance to keep going straight 
		after minimum distance is reached, try to connect with start position */
	return STOP;
}

Vector3f SRTrack::CurrDir()
{
	return (points.Last() - points[points.Size() - 2]).NormalizedCopy();
}

// Saved into X and Y
Vector2f SRTrack::CurrDirXZ()
{
	Vector3f last = points.Last();
	Vector3f nextLast = points[points.Size() - 2];
	return Vector2f(last.x - nextLast.x, last.z - nextLast.z).NormalizedCopy();
}

int SRTrack::TurnLeft(bool back, Vector2f clampDir)
{
	// Check past 2 points.
	Vector3f dir = CurrDir();
	Vector2f xzDir = Vector2f(dir.x, dir.z).NormalizedCopy();
	Angle ang(xzDir.x, xzDir.y);
	ang -= Angle::FromDegrees(MinimumFloat(3.f+left * 2.f, 25.f));
	Vector2f newXZ = ang.ToVector2f();
	bool clamped = false;
	if (back)
	{
		if (newXZ.DotProduct(clampDir) > 0.95f)
		{
			newXZ = clampDir;
			clamped = true;
		}
	}
	Vector3f newDir = Vector3f(newXZ.x, dir.y, newXZ.y);
	points.AddItem(points.Last() + newDir * itLength);
	if (clamped)
		return TURN_REACHED;
	++left;
	if (trackRand.Randf() < 0.85f)
		return TURN_LEFT;
	left = 0;
	return FORWARD;
}

int SRTrack::TurnToStart()
{
	Vector3f last = points.Last();
	if (last.z > 0)
		TurnRight(true, Vector2f(1,0));
	else
		TurnLeft(true, Vector2f(1,0));
	return TURN_TO_START;
}

int SRTrack::TurnRight(bool back, Vector2f clampDir)
{
	// Check past 2 points.
	Vector3f dir = CurrDir();
	Vector2f xzDir = Vector2f(dir.x, dir.z).NormalizedCopy();
	Angle ang(xzDir.x, xzDir.y);
	ang += Angle::FromDegrees(MinimumFloat(3.f + right * 2.f, 25.f));
	Vector2f newXZ = ang.ToVector2f();
	bool clamped = false;
	if (back)
	{
		if (newXZ.DotProduct(clampDir) > 0.95f)
		{
			newXZ = clampDir;
			clamped = true;
		}
	}
	Vector3f newDir = Vector3f(newXZ.x, dir.y, newXZ.y);
	points.AddItem(points.Last() + newDir * itLength);
	if (clamped)
		return TURN_REACHED;
	++right;
	if (trackRand.Randf() < 0.85f)
		return TURN_RIGHT;
	right = 0;
	return FORWARD;
}

// Generates playable mesh field.
Mesh * SRTrack::GenerateMesh()
{
	// Pull off from rendering first?
	MapMan.RemoveEntities(trackEntities);
	Sleep(100);

	leftSides.Clear();
	rightSides.Clear();
	// First copy-em over. They will later be adjusted.
	leftSides = rightSides = points;
	for (int i = 0; i < points.Size(); ++i)
	{
		int thisIndex = i, nextIndex = (i+1) % points.Size();
		Vector3f point = points[i], next = points[nextIndex];
		// For each point, check where it's going.
		Vector3f forward = (next - point).NormalizedCopy();
		Vector3f left = forward.CrossProduct(Vector3f(0,-1,0));
		leftSides[thisIndex] += (left * trackWidth * 0.5f);
		rightSides[thisIndex] -= (left * trackWidth * 0.5f);
		leftSides[nextIndex] += (left * trackWidth * 0.5f);
		rightSides[nextIndex] -= (left * trackWidth * 0.5f);
//		std::cout<<"\nRight sides ("<<i<<"): "<<rightSides[i];
//		std::cout<<"\nRight sides ("<<i+1<<"): "<<rightSides[nextIndex];
	}

	// Create mesh.
	if (mesh)
		delete mesh;
	mesh = new Mesh();
	EMesh * em = new EMesh("Generated track");

	for (int i = 0; i < points.Size(); ++i)
	{
		int thisIndex = i, nextIndex = (i+1) % points.Size();
		em->AddPlane(leftSides[nextIndex], leftSides[thisIndex], rightSides[thisIndex], rightSides[nextIndex]);
	}
	mesh->LoadDataFrom(em);
	delete em;
	// Create model if needed
	if (!trackModel)
	{
		trackModel = ModelMan.NewDynamic();
	}
	trackModel->mesh = mesh;
	mesh->CalculateBounds(); 
	trackModel->RegenerateTriangulizedMesh();
	/// Buffer it if already in there.
	if (trackEntity)
	{
		QueueGraphics(new GMBufferMesh(mesh));
		QueuePhysics(new PMRecalculatePhysicsMesh(mesh));
	};
	// For each point, generate checkpoints?
	GenerateWalls();
	return mesh;
}

Mesh * SRTrack::GenerateWalls()
{
	// Based on previous left/right-sides. Create walls.
	List<Vector3f> leftSideWall, rightSideWall;
	leftSideWall = leftSides;
	rightSideWall = rightSides;
	for (int i = 0; i < points.Size(); ++i)
	{
		Vector3f & lsw = leftSideWall[i];
		lsw.y += 2.f; //std::cout<<"\nLsw: "<<lsw;
		rightSideWall[i].y += 2.f;
	}

	// Create mesh.
	if (wallMesh)
		delete wallMesh;
	wallMesh = new Mesh();
	EMesh * em = new EMesh("Generated walls");
	// Create the walls
	for (int i = 0; i < points.Size(); ++i)
	{
		int thisIndex = i, nextIndex = (i+1) % points.Size();
		// Left side.
		em->AddPlane(leftSideWall[nextIndex], leftSideWall[thisIndex], leftSides[thisIndex], leftSides[nextIndex]);
		// Right side.
		em->AddPlane(rightSides[nextIndex], rightSides[thisIndex], rightSideWall[thisIndex], rightSideWall[nextIndex]);
	}
	wallMesh->LoadDataFrom(em);
	delete em;
	// Create model if needed
	if (!wallModel)
	{
		wallModel = ModelMan.NewDynamic();
	}
	wallModel->mesh = wallMesh;
	wallMesh->CalculateBounds(); 
	wallModel->RegenerateTriangulizedMesh();
	/// Buffer it if already in there.
	if (wallEntity)
	{
		QueueGraphics(new GMBufferMesh(wallMesh));
		QueuePhysics(new PMRecalculatePhysicsMesh(wallMesh));
	};
	return wallMesh;	
}

#include "ShipManager.h"
#include "EntityStates/RacingShipGlobal.h"

/// o-o
Entity * SRTrack::SpawnPlayer()
{
	// Get first best ship.
	// Add it.
	Ship * ship = new Ship(*ShipMan.GetShips()[0]);
	ship->modelSource;
	Entity * entity = MapMan.CreateEntity("Player", ModelMan.GetModel(ship->modelSource), TexMan.GetTexture(ship->diffuseSource), points[0] + Vector3f(0,2,0));
	entity->SetScale(0.2f);
	// Spawn facing next point?
	Vector3f toNext = points[1] - points[0];
	toNext.Normalize();
	Angle angle(toNext);
	float radians = angle.Radians() - PI / 2;
	entity->Rotate(Vector3f(0, radians, 0));

	QueuePhysics(new PMSetEntity(entity, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));
	QueuePhysics(new PMSetEntity(entity, PT_USE_QUATERNIONS, true));
//	QueuePhysics(new PMSetEntity(entity, PT_REQ, PhysicsType::DYNAMIC));
	MapMan.AddEntity(entity);
	// Give it input focus or whatever?
	rsg = new RacingShipGlobal(entity, ship);
	entity->properties.AddItem(rsg);
	rsg->OnEnter();
	rsg->inputFocus = true;

	return entity;
}
