/// Emil Hedemalm
/// 2016-06-14
/// Property for blinking track-lights (and possibly attached real light sources)

#include "TrackLightProp.h"

#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Messages/GMLight.h"
#include "TextureManager.h"
#include "StateManager.h"

Color TrackLightProp::baseColor, TrackLightProp::brightColor;
int TrackLightProp::msPerDecayIteration = 200;
int TrackLightProp::msPerLight = 1000; // Default.
List<TrackLightProp*> TrackLightProp::trackLightsRight, TrackLightProp::trackLightsLeft;


TrackLightProp::TrackLightProp(Entity * owner, ConstVec3fr position, int type) 
	: EntityProperty("TrackLightProp", 3, owner), type(type)
{
	lit = false;
	if (type == TrackLight::RIGHT)
		lightList = &trackLightsRight;
	else
		lightList = &trackLightsLeft;
	index = lightList->Size();
	lightList->AddItem(this);
	light = 0;

	if (light == 0)
	{
		light = new Light("Light");
		light->diffuse = light->specular = brightColor;
		light->type = LightType::POINT;
		light->attenuation = Vector3f(1.f, 1.f, 0.01f);
		light->position = position;
		QueueGraphics(new GMAddLight(light));
	}
	Extinguish(); // Extinguish self!
}

TrackLightProp::~TrackLightProp()
{
	lightList->RemoveItem(this);
}

void TrackLightProp::Process(int timeInMs)
{
	if (type == TrackLight::RIGHT)
		return;
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
		TrackLightProp * next = (*lightList)[(index+1) % lightList->Size()];
		next->LightUp(lightTime);
		Extinguish();
	}
}

void TrackLightProp::LightUp(int startTime /*=0*/)
{
	lit = 255;
	lightTime = startTime;
	extinguishTime = msPerDecayIteration;
	UpdateTexture();
}

void TrackLightProp::UpdateTexture()
{
	Color color;
	float ratio = lit / 255.f;
	color = ratio * baseColor + ratio * brightColor;
	color.Clamp(0,1);
	color.w = 1.f;
	Texture * tex = TexMan.GetTextureByColor(color);
	QueueGraphics(new GMSetEntityTexture(owner, DIFFUSE_MAP | EMISSIVE_MAP, tex)); 	
	QueueGraphics(new GMSetEntityTexture(trackLightsRight[index]->owner, DIFFUSE_MAP | EMISSIVE_MAP, tex)); 	
//		QueueGraphics(new GMSetEntity(owner, 

	/// Update lighting at the same time.
	if (light)
	{
		float fRatio = 8 - 128.f * (1 - ratio);
		QueueGraphics(new GMSetLight(light, LT_CONSTANT_ATTENUATION, fRatio));
		QueueGraphics(new GMSetLight(trackLightsRight[index]->light, LT_CONSTANT_ATTENUATION, fRatio));
	}
}