/// Emil Hedemalm
/// 2016-06-14
/// Property for blinking track-lights (and possibly attached real light sources)

#include "Entity/EntityProperty.h"
#include "Color.h"

class Light;

namespace TrackLight{
enum 
{
	RIGHT,
	LEFT
};};

class TrackLightProp : public EntityProperty 
{
public:
	TrackLightProp(Entity * owner, ConstVec3fr pos, int type);
	virtual ~TrackLightProp();
	virtual void Process(int timeInMs);
	void LightUp(int startTime = 0);
	void Extinguish()
	{
		lit -= 10;
		UpdateTexture();
	}
	void UpdateTexture();
	int index;
	int lit;
	int extinguishTime;
	int lightTime;
	int type;
	List<TrackLightProp*> * lightList;
	static Color baseColor, brightColor;
	static int msPerDecayIteration;
	static int msPerLight;
	static List<TrackLightProp*> trackLightsRight, trackLightsLeft;
	Light * light;
};

