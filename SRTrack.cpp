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
#include "Graphics/Messages/GMSetEntity.h"

SRTrack::SRTrack()
{
	trackWidth = 5.f;
	trackEntity = 0;
	trackModel = 0;
	mesh = 0;
	trackModel = 0;
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
		MapMan.AddEntity(trackEntity);
	}

	// Add other relevant entities to represent the track.

	// Add scenery
		// Create a dummy entity.
	static int attempts = 0;
	MapMan.CreateEntity("Lall", ModelMan.GetModel("obj/cube.obj"), TexMan.GetTexture("0xFFFFFFFF"), Vector3f(attempts++, 0,0));

}

// Generates field.
void SRTrack::Generate()
{
	points.Clear();
	// Quadratic
	enum {
		QUAD,
		CIRCLE,
		HIPPODROME,
	};
	int stereotype = HIPPODROME;
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
	// For each point, generate checkpoints?
}

// Generates playable mesh field.
Mesh * SRTrack::GenerateMesh()
{
	List<Vector3f> leftSides, rightSides;
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
		std::cout<<"\nRight sides ("<<i<<"): "<<rightSides[i];
		std::cout<<"\nRight sides ("<<i+1<<"): "<<rightSides[nextIndex];
	}

	// Create mesh.
	if (mesh)
		delete mesh;
	mesh = new Mesh();
	EMesh * em = new EMesh();

	for (int i = 0; i < points.Size(); ++i)
	{
		int thisIndex = i, nextIndex = (i+1) % points.Size();
		em->AddPlane(leftSides[nextIndex], leftSides[thisIndex], rightSides[thisIndex], rightSides[nextIndex]);
	}
	mesh->LoadDataFrom(em);
	delete em;
	// Create model if needed
	if (!trackEntity)
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
	};
	return mesh;
}
