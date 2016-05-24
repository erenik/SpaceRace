/// Emil Hedemalm
/// 2016-05-18
/// To turn ships and stuffs to align with the racing-track.

#include "MagneticProperty.h"

MagneticProperty::MagneticProperty(Entity * owner)
	: EntityProperty("MagneticProp", ID(), owner)
{
	timeOutsideTrack = 0;
}