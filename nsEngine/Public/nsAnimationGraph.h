#pragma once

#include "nsAnimationTypes.h"



class NS_ENGINE_API nsAnimationGraph
{
private:
	nsTMap<nsName, bool> ParameterBools;
	nsTMap<nsName, int> ParameterInts;
	nsTMap<nsName, float> ParameterFloats;
	nsTMap<nsName, nsVector3> ParameterVectors;

	
public:
	nsAnimationGraph();
	void TickUpdate(float deltaTime);

};
