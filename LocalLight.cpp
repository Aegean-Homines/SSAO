#include "LocalLight.h"

LocalLight::AttenuationMap LocalLight::attenuationLookUpMap =
{
	{ 7   , std::make_pair(0.7000f, 1.8f) },
	{ 13  , std::make_pair(0.3500f, 0.44f) },
	{ 20  , std::make_pair(0.2200f, 0.20f) },
	{ 32  , std::make_pair(0.1400f, 0.07f) },
	{ 50  , std::make_pair(0.0900f, 0.032f) },
	{ 65  , std::make_pair(0.0700f, 0.017f) },
	{ 100 , std::make_pair(0.0450f, 0.0075f) },
	{ 160 , std::make_pair(0.0270f, 0.0028f) },
	{ 200 , std::make_pair(0.0220f, 0.0019f) },
	{ 325 , std::make_pair(0.0140f, 0.0007f) },
	{ 600 , std::make_pair(0.0070f, 0.0002f) },
	{ 3250, std::make_pair(0.0014f, 0.00007f) },
};

float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

LocalLight::LocalLight(bool randomize, bool castingShadow, bool allWhite)
{
	if (randomize) {
		float randomX = RandomFloat(-50, 50);
		float randomY = RandomFloat(-50, 50);
		float randomZ = RandomFloat(1, 10);

		lightPos = glm::vec3(randomX, randomY, randomZ);
		
		float randomR;
		float randomG;
		float randomB;

		if(allWhite){
			randomR = 1.0f;
			randomG = 1.0f;
			randomB = 1.0f;
		}
		else {
			randomR = RandomFloat(0, 1);
			randomG = RandomFloat(0, 1);
			randomB = RandomFloat(0, 1);
		}

		lightColor = glm::vec3(randomR, randomG, randomB);

		radius = 13.0f;

		//attenuationVector = glm::vec2(linear, quadratic);

		//radius = RandomFloat(7, 50);

	}
	else {
		lightPos = glm::vec3(0, 0, 0);
		lightColor = glm::vec3(1, 1, 1);

		radius = 10.0f;
	}

	isCasting = castingShadow;
	lightModel = new Sphere(32);

	/*AttenuationMap::iterator iter = attenuationLookUpMap.begin();
	++iter;
	AttenuationMap::iterator iterBack = attenuationLookUpMap.begin();
	while (radius > iter->first && iter != attenuationLookUpMap.end()) {
		++iter;
		++iterBack;
	}

	// meaning range is higher than the max element in the map
	if (iter == attenuationLookUpMap.end())
		attenuationVector = glm::vec2(iter->second.first, iter->second.second);
	else // floor the range element
		attenuationVector = glm::vec2(iterBack->second.first, iterBack->second.second);*/


}


LocalLight::~LocalLight()
{
}
