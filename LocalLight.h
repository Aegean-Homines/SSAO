#pragma once

#include "glsdk\glm\glm\glm.hpp"
#include "models.h"
#include <map>

class LocalLight
{
public:
	typedef std::map<unsigned, std::pair<float, float>> AttenuationMap;

	LocalLight(bool randomize = false, bool castingShadow = false, bool allWhite = false);
	~LocalLight();
	
	bool isCasting;

	glm::vec3 lightPos;
	glm::vec3 lightColor;
	glm::vec2 attenuationVector;

	Model* lightModel;

	float radius;
	float maxBrightness;

	static AttenuationMap attenuationLookUpMap;
};

