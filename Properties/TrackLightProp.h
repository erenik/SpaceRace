/// Emil Hedemalm
/// 2016-06-14
/// Property for blinking track-lights (and possibly attached real light sources)

#include "Entity/EntityProperty.h"

class TrackLightProp : public EntityProperty 
{
public:
	TrackLightProp(Entity * owner)
		: EntityProperty("TrackLightProp", 3, owner)
	{
		lit = false;
		index = trackLights.Size();
		trackLights.AddItem(this);
		Extinguish(); // Extinguish self!
	}
	virtual ~TrackLightProp()
	{
		trackLights.RemoveItem(this);
	}
	virtual void Process(int timeInMs)
	{
		// Set first texture, swap?
		if (lit <= 0)
			return;
		if (lit < 254)
		{
			extinguishTime -= timeInMs;
			if (extinguishTime < 0)
			{
				Extinguish();
				extinguishTime += msPerDecayIteration;
			}
			return;
		}
		// Skip 1st frame after switch, due to bugging.
		if (lit == 255)
		{
			--lit;
			return;
		}
		lightTime = (lightTime + timeInMs) % 10000;
		if (lightTime > msPerLight)
		{
			lightTime -= msPerLight;
			TrackLightProp * next = trackLights[(index+1) % trackLights.Size()];
			next->Light(lightTime);
			Extinguish();
		}
	}
	void Light(int startTime = 0)
	{
		lit = 255;
		lightTime = startTime;
		extinguishTime = msPerDecayIteration;
		UpdateTexture();
	}
	void Extinguish()
	{
		lit -= 10;
		UpdateTexture();
	}
	void UpdateTexture()
	{
		Color color;
		float ratio = lit / 255.f;
		color = ratio * baseColor + ratio * brightColor;
		color.Clamp(0,1);
		Texture * tex = TexMan.GetTextureByColor(color);
		QueueGraphics(new GMSetEntityTexture(owners, DIFFUSE_MAP, tex)); 	
//		QueueGraphics(new GMSetEntity(owner, 
	}
	int index;
	int lit;
	int extinguishTime;
	int lightTime;
	static Color baseColor, brightColor;
	static int msPerDecayIteration;
	static int msPerLight;
	static List<TrackLightProp*> trackLights;
};

Color TrackLightProp::baseColor, TrackLightProp::brightColor;
int TrackLightProp::msPerDecayIteration = 200;
int TrackLightProp::msPerLight = 1000; // Default.
List<TrackLightProp*> TrackLightProp::trackLights;

