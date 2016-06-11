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

#include "String/StringUtil.h"
#include "Physics/CC.h"
#include "StateManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "PhysicsLib/PhysicsShape.h"
#include "PhysicsLib/PhysicsType.h"
#include "StateManager.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Util/List/ListUtil.h"
#include "File/LogFile.h"
#include "File/FileUtil.h"

#include "Random/Random.h"
Random trackRand;

#define LogTrack(t,l) LogToFile("log/Track.txt", t, l, 0)

TrackPoint::TrackPoint()
{
	Nullify();
}
TrackPoint::TrackPoint(ConstVec3fr p)
{
	Nullify();
	pos = p;
}

TrackPoint::TrackPoint(float x, float y, float z)
{
	Nullify();
	pos = Vector3f(x,y,z);
}
void TrackPoint::Nullify()
{
	next = 0;
	isLoop = false;
}

bool TrackPoint::WriteTo(std::fstream & file)
{
	file.write((char*)&isLoop, sizeof(bool));
	pos.WriteTo(file);
	up.WriteTo(file);
	right.WriteTo(file);
	return true;
}
bool TrackPoint::ReadFrom(std::fstream & file)
{
	file.read((char*)&isLoop, sizeof(bool));
	pos.ReadFrom(file);
	up.ReadFrom(file);
	right.ReadFrom(file);
	return true;
}

/// Resets left/rightside to be pos.
void TrackPoint::CenterSides()
{
	leftSide = rightSide = pos;
}



SRTrack::SRTrack()
{
	forwardRate = 0.75f;
	turnRate = 0.85f;
	itLength = 10.f;
	trackWidth = 5.f;
	wallHeight = 3.5f;
	trackEntity = 0;
	trackModel = 0;
	mesh = 0;
	rsg = 0;
	loops = 0;
	minLoopRadius = 40.f;
	maxLoopRadius = 80.f;

	maxLoops = 2;
	loopChance = 0.05f;

	wallEntity = 0;
	wallModel = 0;
	wallMesh = 0;

	frameEntity = 0;
	frameModel = 0;
	frameMesh = 0;
	collisionDistance = 25.f;

	turnTiltRatio = 1.f;
	degreesTurnRight = 0;
	maxTurnPerSeg = 15;
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
		QueuePhysics(new PMSetEntity(trackEntity, PT_COLLISION_CATEGORY, CC_TRACK));
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
	if (!frameEntity)
	{
		frameEntity = EntityMan.CreateEntity("Frame", frameModel, TexMan.GetTexture("img/track/Concrete_Tile_diffuse.png"));
		QueuePhysics(new PMSetEntity(frameEntity, PT_PHYSICS_SHAPE, ShapeType::MESH));
		trackEntities.AddItem(frameEntity);
	}
	// Add all.
	MapMan.AddEntities(trackEntities);
	MapMan.AddEntities(audienceStructs);
	// Add other relevant entities to represent the track.

	// Add scenery

	// Create a dummy entity.
	static int attempts = 0;
	MapMan.CreateEntity("Lall", ModelMan.GetModel("obj/cube.obj"), TexMan.GetTexture("0xFFFFFFFF"), Vector3f(float(attempts++), 0,0));

	/// Update the physics thingy.
//	QueuePhysics(new PMSet(PT_AABB_SWEEPER_DIVISIONS

}

// Generates field.
void SRTrack::Generate()
{
	mode = RANDOM_GENERATION;
	points.Clear();
	loops = 0;
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
		points.Add(new TrackPoint(0,0,0), new TrackPoint(50, 0, 0), new TrackPoint(50,0,50), new TrackPoint(0,0,50));
	}
	else if (stereotype == CIRCLE)
	{
		int segs = 25;
		for (int i = 0; i < segs; ++i)
		{
			Vector2f pos = Angle(i * PI * 2/ (float)segs).ToVector2f() * 50.f;
			points.AddItem(new TrackPoint(pos.x, 0, pos.y));
		}
	}
	else if (stereotype == HIPPODROME)
	{
		/// Hmm.. far stretch.
		float flatDist = 50.f;
		float width = 25.f; float halfWidth = width / 2;
		points.AddItem(new TrackPoint(0,0,0));
//		points.AddItem(Vector3f(flatDist,0,0));
		int segs = 8;
		// First half-circle.
		for (int i = 0; i < segs + 1; ++i)
		{
			Vector2f pos = Angle(-PI / 2.f + i * PI / (float)segs).ToVector2f() * halfWidth;
			points.AddItem(new TrackPoint(flatDist + pos.x, 0, halfWidth + pos.y));
		}
//		points.AddItem(Vector3f(flatDist, 0, width));
//		points.AddItem(Vector3f(0,0,width));
		for (int i = 0; i < segs; ++i)
		{
			Vector2f pos = Angle(PI / 2.f + i * PI/ (float)segs).ToVector2f() * halfWidth;
			points.AddItem(new TrackPoint(pos.x, 0, halfWidth + pos.y));
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
		points.AddItem(new TrackPoint(0,0,0)); // Start
		points.AddItem(new TrackPoint(itLength, 0,0));
		int which = FORWARD;
		left = right = 0;
		int iterations = 0;
		attemptsToEnd = 0;
		archIterations = 0;
		degreesTurnRight = 0;
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
				case TURN_LEFT_BACK: which = TurnTo(Vector3f(-1,0,0)); break;
				case TURN_RIGHT_BACK: which = TurnTo(Vector3f(-1,0, 0)); break;
				case TURN_RIGHT_TO_MIDDLE: which = TurnTo(Vector3f(0,0, -1)); break;
				case TURN_LEFT_TO_MIDDLE: which = TurnTo(Vector3f(0,0, 1)); break;
				case TURN_TO_START: which = TurnToStart(); break;
				case ARCH_TO_START: which = ArchToStart(); break;
				case TURN_TO_CENTER: which = TurnToCenter(); break;
			}
			++iterations;
			if (iterations > 250)
				break;
		}
		bool collisions = true;
		// Add as many as needed, only?
//		while(AddElevationToAvoidCollisions())
	//		;
	}
}

int SRTrack::TryEndIt()
{
	++attemptsToEnd;
	TrackPoint * last = points.Last();
	Vector3f currDir = CurrDir();
	// Check which side of the field we're on.
	bool rightFace = false, leftFace = false, beyond = false;
	if (currDir.z > 0)
		rightFace = true;
	else // Left side
		leftFace = true;
	if (last->pos.x > 0)
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
		LogTrack("Swing towards center Z", INFO);
		mode = SWING_TOWARDS_CENTER_Z;
		return TURN_TO_CENTER;
	}
	// Not beyond? Can turn to middle and start going back.
	if (archIterations == 0)
	{
		LogTrack("Final arch start @ "+String((int)last), INFO);
		archStart = last;
		archEnd = points[0];
		if (last->pos.z > 0)
			archLeft = true;
		else 
			archLeft = false;
		
		archSegments = (archEnd->pos - archStart->pos).Length() / (itLength) * PI;
		++archIterations;
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
	if (AbsoluteValue(points.Last()->pos.z) < 50.f)
	{
		mode = TURN_TO_CENTER_XZ;
		return TURN_TO_START;
	}
	return FORWARD;
}

int SRTrack::FinalStraight()
{
	LogTrack("Final straight", INFO);
	Vector3f toStart = points[0]->pos - points.Last()->pos;
	Vector3f dir = toStart.NormalizedCopy();
	float dist = (toStart).Length();
	int segs = dist / itLength;
	float segLength = dist / (segs + 1);
	for (int i = 0; i < segs; ++i)
	{
		points.AddItem(new TrackPoint(points.Last()->pos + dir * segLength));
	}
	return STOP;
}

int SRTrack::ArchToStart()
{
	/// Hmm.. far stretch.
	float archRadius = (archEnd->pos - archStart->pos).Length() / 2;
	Vector3f middle = (archEnd->pos + archStart->pos) * 0.5f;
	int segs = archSegments;
	++archIterations;
	// Half-circle.
	Vector2f pos = Angle((archLeft? PI / 2.f : -PI / 2.f) + (archLeft ? 1.f : -1.f) * archIterations * PI / (float)segs).ToVector2f() * archRadius;
	Vector3f pointPos = middle + Vector3f(pos.x, 0, pos.y);
	LogTrack("ArchToStart "+VectorString(pointPos), INFO);
	points.AddItem(new TrackPoint(pointPos));
	if (archIterations + 1 >= segs)
		return STOP;
	return ARCH_TO_START;
}

int SRTrack::TurnToCenter()
{
	LogTrack("TurnToCenter", INFO);
	TrackPoint * last = points.Last();
	int turnResult = 0;
	if (last->pos.z > 0)
		turnResult = TurnTo(Vector3f(0,0,-1));
	else if (last->pos.z < 0)
		turnResult = TurnTo(Vector3f(0,0,1));
	if (turnResult == TURN_REACHED)
		mode = GO_TO_CENTER_Z;
	return TURN_TO_CENTER;
}


bool SRTrack::AddElevationToAvoidCollisions(float ratio)
{
	static List<int> lastCollisionPointIndices;
	// Fetch all points within collision distance.
	List<int> collisionPointIndices;
	for (int i = 0; i < points.Size(); ++i)
	{
		for (int j = i + 5; j < points.Size() - 5; ++j)
		{
			int index1 = i % points.Size(), index2 = j % points.Size();
			if (index1 == index2)
				continue;
			Vector3f p1 = points[index1]->pos, p2 = points[index2]->pos;
			float distXZ = (Vector2f(p2.x, p2.z) - Vector2f(p1.x, p1.z)).Length();
			float distY = AbsoluteValue(p2.y - p1.y);
			// Weighted, half distY, 1. of XZ
			float dist = Vector2f(distXZ, distY * 2.f).Length();
			if (dist < collisionDistance)
				collisionPointIndices.AddItem(index2);
		}
	}
	for (int i = 0; i < collisionPointIndices.Size(); ++i)
	{
		int pIndex = collisionPointIndices[i];
		int maxNeigh = 0;
		maxNeigh += lastCollisionPointIndices.Occurances(pIndex);
		ClampInt(maxNeigh, 0, 25);
		int neighboursToRaise = int(trackRand.Randf() * float(maxNeigh) + 4.f);
		for (int j = -neighboursToRaise; j <= neighboursToRaise; ++j)
		{
			float dist = AbsoluteValue(j);
			float multiplier = 1 - (dist / (float) (neighboursToRaise+1)); // Linear?
			int index = (collisionPointIndices[i] + j + points.Size()) % points.Size();
			points[index]->pos.y += ratio * multiplier; 
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

/// Mainly for the ending loops.
bool SRTrack::SmoothHardEdges(float threshold /*= 0.96f */)
{
	for (int i = 0; i < points.Size(); ++i)
	{
		// Compare with next one.
		TrackPoint * p = points[i], * p2 = points[(i+1) % points.Size()], * p3 = points[(i+2) % points.Size()];
		Vector3f dir = (p2->pos - p->pos).NormalizedCopy(), dir2 = (p3->pos - p2->pos).NormalizedCopy();
		float dot = dir.DotProduct(dir2);
		if (dot > threshold)
			continue; // Good enough.
		if (p2->isLoop)
			continue;
		/// Equalize point in middle to be somewhere in between the other 2?
		p2->pos = p2->pos * 0.5 + p->pos * 0.25f + p3->pos * 0.25f;
		p2->up = p2->up * 0.5 + p->up * 0.25f + p3->up * 0.25f;
		p2->right = p2->right * 0.25f + p->right * 0.25f + p3->right * 0.25f;

//		p2->rightSide
	}
	return true;
}

int SRTrack::PlaceNextPoint()
{
	// Check past 2 points.
	Vector3f dir = CurrDir();
	Vector2f xzDir = Vector2f(dir.x, dir.z).NormalizedCopy();
	Angle ang(xzDir.x, xzDir.y);
	ang -= Angle::FromDegrees(-degreesTurnRight);
	Vector2f newXZ = ang.ToVector2f();
	bool clamped = false;
//	Angle ang(degreesPitch);
	Vector3f newDir = Vector3f(newXZ.x, 0 /*dir.y*/, newXZ.y);
	points.AddItem(new TrackPoint(points.Last()->pos + newDir * itLength));
	return 0;
}

int SRTrack::MakeLoop()
{
	++loops;
	/// Place some forwards first to equalize previous curves. (the various 
	Forward(); Forward(); Forward();

	/// Place points in a default-loop.
	Vector3f forward = this->CurrDir();
	Vector3f up = Vector3f(0,1,0);
	Vector3f right = CurrRight();
	Vector3f offsetDir = right;
	if (trackRand.Randf() > 0.5f)
		offsetDir = -offsetDir; // Make a left-ending loop.

	float loopRadius = trackRand.Randf(maxLoopRadius - minLoopRadius) + minLoopRadius; // Radius?
	int segments = 50;
	float degreesPerSeg = 2 * PI / segments;
	float loopWidth = 30.f;
	Vector3f center = points.Last()->pos + up * loopRadius;
	for (int i = 1; i < segments + 1; ++i)
	{
		float forwardRatio = cos(degreesPerSeg * i - PI / 2);
		float upRatio = sin(degreesPerSeg * i - PI / 2);
		Vector3f rightOffset = offsetDir * loopWidth * i / (float) segments;
		TrackPoint * tp = new TrackPoint(center + forwardRatio * forward * loopRadius + upRatio * up * loopRadius + rightOffset);
		/// Set up-vector to be toward the center of the loop.
		tp->up = (center + rightOffset - tp->pos).NormalizedCopy();
		tp->right = right;
		tp->isLoop = true;
		points.AddItem(tp);
	}
	return 0;
}

int SRTrack::Forward()
{
	LogTrack("Forward", INFO);
	// Last 2 points, +
	if (degreesTurnRight > 0)
		--degreesTurnRight;
	if (degreesTurnRight < 0)
		++degreesTurnRight;

	PlaceNextPoint();
	if (degreesTurnRight == 0 && loops < maxLoops && trackRand.Randf() < loopChance)
		MakeLoop();
	if (trackRand.Randf() < forwardRate)
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
	return (points.Last()->pos - points[points.Size() - 2]->pos).NormalizedCopy();
}

Vector3f SRTrack::CurrRight()
{
	return CurrDir().CrossProduct(Vector3f(0,1,0)).NormalizedCopy();
}

// Saved into X and Y
Vector2f SRTrack::CurrDirXZ()
{
	Vector3f last = points.Last()->pos;
	Vector3f nextLast = points[points.Size() - 2]->pos;
	return Vector2f(last.x - nextLast.x, last.z - nextLast.z).NormalizedCopy();
}

int SRTrack::TurnLeft()
{
	LogTrack("TurnLeft", INFO);
	if (degreesTurnRight > 0)
		degreesTurnRight -= 1.f;
	degreesTurnRight -= 1.f + left;
	ClampFloat(degreesTurnRight, -maxTurnPerSeg, maxTurnPerSeg);
	PlaceNextPoint();
	right = 0;
	++left;
	if (trackRand.Randf() < turnRate)
		return TURN_LEFT;
	left = 0;
	return FORWARD;
}

int SRTrack::TurnToStart()
{
	LogTrack("TurnToStart", INFO);
	Vector3f last = points.Last()->pos;
	if (last.z > 0)
		TurnTo(Vector3f(1,0,0));
	else
		TurnTo(Vector3f(1,0,0));
	return TURN_TO_START;
}

int SRTrack::TurnRight()
{
	LogTrack("TurnRight", INFO);
	left = 0;
	if (degreesTurnRight < 0)
		degreesTurnRight += 1.f;
	degreesTurnRight += 1.f + right;
	ClampFloat(degreesTurnRight, -maxTurnPerSeg, maxTurnPerSeg);
	PlaceNextPoint();
	++right;
	if (trackRand.Randf() < turnRate)
		return TURN_RIGHT;
	right = 0;
	return FORWARD;
}

int SRTrack::TurnTo(ConstVec3fr dir)
{
	// Check current dir.
	Vector3f currDir = CurrDir();
	Angle ang(currDir.x, currDir.z), ang2(dir.x, dir.z);

	Angle diff = ang2 - ang;
	// compare with desired dir.
	if (diff.Radians() > 0)
		TurnRight();
	else
		TurnLeft();
	/// Check if turn reached manually now?
	if (CurrDir().DotProduct(dir) > 0.95f)
		return TURN_REACHED;
	return TURN_NOT_REACHED_YET;
}

/// Calculates Up- and Right-vectors. Skips those with already non-0 values, unless force is true.
void SRTrack::CalculateUpRightVectors(bool force)
{
	Vector3f lastForward(1,0,0);
	/// Reset all vars first.
	for (int i = 0; i < points.Size(); ++i)
	{
		TrackPoint * point = points[i];
		point->CenterSides();
		if (point->isLoop)
			continue;
		point->right = point->up = Vector3f();
	}
	/// Recalc all sides and up-vectors where needed.
	for (int i = 0; i < points.Size(); ++i)
	{
		int thisIndex = i, nextIndex = (i+1) % points.Size();
		TrackPoint * previous = points[(i - 1 + points.Size()) % points.Size()], * point = points[i], * next = points[nextIndex];
		if (point->isLoop)
			continue;
		// For each point, check where it's going.
		Vector3f forward = (next->pos - previous->pos).NormalizedCopy();
		if (point->right.MaxPart() == 0)
			point->right = forward.CrossProduct(Vector3f(0,1,0)).NormalizedCopy();
		if (point->up.MaxPart() == 0)
			point->up = forward.CrossProduct(-point->right).NormalizedCopy();
//		LogTrack("Up: "+VectorString(point->up), INFO);
		lastForward = forward;
	}
}

// Generates playable mesh field.
Mesh * SRTrack::GenerateMesh()
{
	// Pull off from rendering first?
	MapMan.RemoveEntities(trackEntities);
	Sleep(100);
	CalculateUpRightVectors();
	/// Set pointers for next/(previous) points?
	for (int i = 0; i < points.Size(); ++i)
	{
		points[i]->next = points[(i+1) % points.Size()];
	}


	// First copy-em over. They will later be adjusted.
	Vector3f lastForward(1,0,0);
	Vector3f lastPointPos = points.Last()->pos;

	/// Apply tilt to Up-vectors.
	for (int i = 0; i < points.Size(); ++i)
	{
		TrackPoint * previous = points[(i - 1 + points.Size()) % points.Size()], * point = points[i], * next = points[(i+1) % points.Size()];
		// Check tilt, based on turn.
		float distToLastPoint = (lastPointPos - point->pos).Length();
		if (distToLastPoint > 10.f)
			distToLastPoint = 0.f;
		float leftRatio = lastForward.DotProduct(-point->right) * distToLastPoint * 0.1f;
		float radiansToTilt = leftRatio * turnTiltRatio; 
		float rTilt = -radiansToTilt;
		float adjustedTrackWidth = trackWidth; // * (1 + AbsoluteValue(leftRatio) * 2.f);
		float w2 = adjustedTrackWidth * (1 / 6.f);
		
		/// If regular point, adjust tilt of up-vector for the walls (not for loop-segments, etc.)
		if (point->isLoop)
		{
			rTilt = 0;
		}
		Vector3f newRight = (cos(rTilt) * point->right + sin(rTilt) * point->up);
		Vector3f newUp = (-sin(rTilt) * point->right + cos(rTilt) * point->up);
		Vector3f offsetRight = newRight * w2;

		/// Save new directions? Or just keep temporarily?
		point->up = newUp;
		point->right = newRight;


		point->leftSide -= offsetRight;
		point->rightSide += offsetRight;
		next->leftSide -= offsetRight;
		next->rightSide += offsetRight;
		previous->leftSide -= offsetRight;
		previous->rightSide += offsetRight;

		Vector3f forward = (next->pos - previous->pos).NormalizedCopy();
		lastForward = forward;
		lastPointPos = point->pos;
	}

	// Create mesh.
	if (mesh)
		delete mesh;
	mesh = new Mesh();
	EMesh * em = new EMesh("Generated track");
	/// Adds quads for the main track.
	for (int i = 0; i < points.Size(); ++i)
	{
		int thisIndex = i, nextIndex = (i+1) % points.Size();
		TrackPoint * point = points[thisIndex],
			* nextPoint = points[nextIndex];
		Vector3f nextLeft = nextPoint->leftSide,
			left = point->leftSide,
			nextRight = nextPoint->rightSide,
			right = point->rightSide;
		Vector3f center = point->pos, nextCenter = nextPoint->pos;
		Vector3f up = point->up;
		float downDist = trackWidth * 0.15f; // 15% of trackWidth? so e.g. 3 for 20
		Vector3f centerDown = -up * downDist;
		Vector3f nextCenterDown = -nextPoint->up * downDist;

		/// Center-plane. Down a bit?
#ifdef WHY
		/// Initial simple version.
		em->AddPlane(nextLeft, 
			left, 
			right, 
			nextRight);
#endif
		/// First custom-shape version.
		float ratioCenter = 0.5f;
		float ratioEdge = 1 - ratioCenter;
		Vector3f midLeft = center * ratioCenter + left * ratioEdge,
			midRight = center * ratioCenter + right * ratioEdge,
			nextMidLeft = nextCenter * ratioCenter + nextLeft * ratioEdge,
			nextMidRight = nextCenter * ratioCenter + nextRight * ratioEdge;
		em->AddPlane(nextMidLeft + nextCenterDown,  // Middle
			midLeft + centerDown, 
			midRight + centerDown, 
			nextMidRight + nextCenterDown);
		em->AddPlane(nextLeft, 	// Left?
			left, 
			midLeft + centerDown, 
			nextMidLeft + nextCenterDown);
		em->AddPlane(nextMidRight + nextCenterDown,  // Right
			midRight + centerDown, 
			right, 
			nextRight);
		// Set UV-coordinates?
		// Side-planes.
//		em
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
	// For each point, generate walls.
	GenerateWalls();
	// Side- and under-walls.
	GenerateTrackFrame();
	// Add support-structures for the track.
	GenerateSupportStructures();
	// Add buildings.
	GenerateAudienceStructures();

	return mesh;
}

Mesh * SRTrack::GenerateWalls()
{
	// Based on previous left/right-sides. Create walls.
	for (int i = 0; i < points.Size(); ++i)
	{
		TrackPoint * tp = points[i], * next = points[(i+1)% points.Size()];
		tp->leftSideWall = tp->leftSide;
		tp->rightSideWall = tp->rightSide;

		Vector3f forward = (next->pos - tp->pos).NormalizedCopy();
		Vector3f right = tp->right;
		Vector3f up = tp->up;
		tp->leftSideWall += up * wallHeight; //std::cout<<"\nLsw: "<<lsw;
		tp->rightSideWall += up * wallHeight;
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
		TrackPoint * p = points[thisIndex], * p2 = points[nextIndex];
		// Left side.
		em->AddPlane(p2->leftSideWall, p->leftSideWall, p->leftSide, p2->leftSide);
		// Right side.
		em->AddPlane(p2->rightSide, p->rightSide, p->rightSideWall, p2->rightSideWall);
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

Mesh * SRTrack::GenerateTrackFrame()
{
	float wallWidth = 1.f;
	// Based on previous data, create outer walls.
	for (int i = 0; i < points.Size(); ++i)
	{
		TrackPoint * tp = points[i];
		Vector3f rightOffset = tp->right * wallWidth;
		tp->leftSideWallOuter = tp->leftSideWall - rightOffset;
		tp->rightSideWallOuter = tp->rightSideWall + rightOffset;
		tp->leftOuter = tp->leftSide - rightOffset;
		tp->rightOuter = tp->rightSide + rightOffset;
		tp->lowerLeft = tp->leftSide * 0.5f + tp->pos * 0.5f - tp->up * trackWidth * 0.3f;
		tp->lowerRight = tp->rightSide * 0.5f + tp->pos * 0.5f - tp->up * trackWidth * 0.3f;
	}
	// Create mesh.
	if (frameMesh)
		delete frameMesh;
	frameMesh = new Mesh();
	EMesh * em = new EMesh("Generated frame/outer walls");
	// Create the walls
	for (int i = 0; i < points.Size(); ++i)
	{
		int thisIndex = i, nextIndex = (i+1) % points.Size();
		TrackPoint * p = points[thisIndex], * p2 = points[nextIndex];
		/// Top-sides by the inner-walls.
/*		Vector3f outLeftUp = p->leftSideWall - p->right * wallWidth,
			inLeftUp = p->leftSideWall,
			farOutLeftUp = p2->leftSideWall - p->right * wallWidth,
			farInLeftUp = p2->leftSideWall;
			*/
		/// Top-sides.
//		em->AddPlane(farOutLeftUp, outLeftUp, inLeftUp, farInLeftUp);
		em->AddPlane(p2->leftSideWallOuter, p->leftSideWallOuter, p->leftSideWall, p2->leftSideWall);
		em->AddPlane(p2->rightSideWall, p->rightSideWall, p->rightSideWallOuter, p2->rightSideWallOuter);
		/// Side-sides
		em->AddPlane(p2->leftOuter, p->leftOuter, p->leftSideWallOuter, p2->leftSideWallOuter);
		em->AddPlane(p2->rightSideWallOuter, p->rightSideWallOuter, p->rightOuter, p2->rightOuter);
		// 3 bottom plates
		em->AddPlane(p2->lowerLeft, p->lowerLeft, p->leftOuter, p2->leftOuter);
		em->AddPlane(p2->rightOuter, p->rightOuter, p->lowerRight, p2->lowerRight);
		em->AddPlane(p2->lowerRight, p->lowerRight, p->lowerLeft, p2->lowerLeft);
		
	}
	frameMesh->LoadDataFrom(em);
	delete em;
	// Create model if needed
	if (!frameModel)
	{
		frameModel = ModelMan.NewDynamic();
	}
	frameModel->mesh = frameMesh;
	frameMesh->CalculateBounds(); 
	frameModel->RegenerateTriangulizedMesh();
	/// Buffer it if already in there.
	if (frameEntity)
	{
		QueueGraphics(new GMBufferMesh(frameMesh));
		QueuePhysics(new PMRecalculatePhysicsMesh(frameMesh));
	};
	return frameMesh;	
}


List<Entity*> SRTrack::GenerateSupportStructures()
{
	int supportStructureInterval = 5;
	// Do every X points?
	for (int i = 0; i < points.Size(); i += supportStructureInterval)
	{
		
	}
	return List<Entity*>();
}

List<Entity*> SRTrack::GenerateAudienceStructures()
{
	if (audienceStructs.Size())
		MapMan.DeleteEntities(audienceStructs);
	audienceStructs.Clear();
	int numBuildings = 15;
	String dir = "./obj/props/audienceStructs/";
	List<String> models;
	int ok = GetFilesInDirectory(dir, models);
	if (models.Size() == 0){
		LogMain("Unable to find any audience structure models", ERROR);
		return audienceStructs;
	}
	for (int i = 0; i < numBuildings; ++i)
	{
		/// Fetch model.
		Model * model = ModelMan.GetModel(dir + models[trackRand.Randi() % models.Size()]);
		float radius = model->Radius() + trackWidth;
		float radiusSq = radius * radius;
		// Find a good place close to some random point.
		for (int j = 0; j < 100; ++j)
		{
			TrackPoint * tp = points[trackRand.Randi() % points.Size()];
			Vector3f randomVec(trackRand.Randf() - 0.5f, 0, trackRand.Randf() - 0.5f);
			randomVec.Normalize();
			Vector3f position = tp->pos + randomVec * radius;
			position.y = 0;
			/// Check dist to all other points.
			bool skip = false;
			for (int k = 0; k < points.Size(); ++k)
			{
				TrackPoint * tp2 = points[k];
				float distSq = (Vector2f(position.x, position.z) - Vector2f(tp2->pos.x, tp2->pos.z)).LengthSquared();
				if (distSq < radiusSq)
				{
					skip = true;
					break;
				}
			}
			if (skip)
				continue;

			/// Place building.
			Entity * e = EntityMan.CreateEntity("AudienceBuilding"+String(i), model, TexMan.GetTexture("0xFFFF"));
			e->SetPosition(position);
			audienceStructs.AddItem(e);
			break;
		}
	}
	return audienceStructs;
}

Vector3f SRTrack::SpawnPosition()
{
	return points[0]->pos + Vector3f(0,2,0);
}
#include "ShipManager.h"
#include "Properties/RacingShipGlobal.h"

/// o-o
Entity * SRTrack::SpawnPlayer()
{
	// Get first best ship.
	// Add it.
	Ship * ship = new Ship(*ShipMan.GetShips()[0]);
	ship->modelSource;
	Entity * entity = MapMan.CreateEntity("Player", ModelMan.GetModel(ship->modelSource), TexMan.GetTexture(ship->diffuseSource), SpawnPosition());
	entity->SetScale(0.2f);
	// Spawn facing next point?
	Vector3f toNext = CurrDir();
	Angle angle(toNext);

	QueuePhysics(new PMSetEntity(entity, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));
	QueuePhysics(new PMSetEntity(entity, PT_USE_QUATERNIONS, true));
//	QueuePhysics(new PMSetEntity(entity, PT_REQ, PhysicsType::DYNAMIC));
	MapMan.AddEntity(entity);
	ResetPosition(entity); // Place position/rotation.
	// Give it input focus or whatever?
	rsg = new RacingShipGlobal(entity, ship);
	entity->properties.AddItem(rsg);
	rsg->OnEnter();
	rsg->inputFocus = true;

	return entity;
}

void SRTrack::DespawnPlayer(Entity * playerEntity)
{
	MapMan.DeleteEntity(playerEntity);
}

void SRTrack::ResetPosition(Entity * playerEntity)
{
	float radians = PI / 2;
	QueuePhysics(new PMSetEntity(playerEntity, PT_ROTATION_Y, radians));
	QueuePhysics(new PMSetEntity(playerEntity, PT_POSITION, track->SpawnPosition()));
	QueuePhysics(new PMSetEntity(playerEntity, PT_VELOCITY, Vector3f()));
}

/// Oh yeah.
bool SRTrack::Save(String fileName)
{
	fileName = "tracks/"+fileName+".track";
	std::fstream file;
	file.open(fileName.c_str(), std::ios_base::out | std::ios_base::binary);
	if (!file.is_open())
		return false;
	if (name.Length() == 0) name = fileName;
	name.WriteTo(file);
	/// Write other variables, such as tilt and track width?
	file.write((char*)&turnTiltRatio, sizeof(float));
	file.write((char*)&trackWidth, sizeof(float));
	file.write((char*)&wallHeight, sizeof(float));
	WriteListTo<TrackPoint>(points, file);
	return true;
}
/// Loads and makes active.
bool SRTrack::Load(String fileName)
{
	fileName = "tracks/"+fileName+".track";
	std::fstream file;
	file.open(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
		return false;
	name.ReadFrom(file);
	file.read((char*)&turnTiltRatio, sizeof(float));
	file.read((char*)&this->trackWidth, sizeof(float));
	file.read((char*)&this->wallHeight, sizeof(float));
	/// Read points
	ReadListFrom<TrackPoint>(points, file);
	// Generate mesh.
	track->GenerateMesh();
	// Redner it?
	track->MakeActive();
}

/// o-o
TrackPoint * SRTrack::NearestPoint(ConstVec3fr pos)
{
	TrackPoint * nearest = 0;
	float nearDist = 100000.f;
	for (int i = 0; i < points.Size(); ++i)
	{
		TrackPoint * tp = points[i];
		float dist = (tp->pos - pos).LengthSquared();
		if (dist < nearDist)
		{
			nearest = tp;
			nearDist = dist;
		}
	}
	return nearest;
}

