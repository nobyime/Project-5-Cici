#include "EnergySystem.h"

float gSpaceEnergy = 100.0f;

void resetSpaceEnergy()
{
    gSpaceEnergy = 100.0f;
}

void drainSpaceEnergy(float deltaTime)
{
    gSpaceEnergy -= (100.0f / 420.0f) * deltaTime;
    if (gSpaceEnergy < 0.0f) gSpaceEnergy = 0.0f;
}

void addSpaceEnergy(float amount)
{
    gSpaceEnergy += amount;
    if (gSpaceEnergy > 100.0f) gSpaceEnergy = 100.0f;
}

float getSpaceEnergy()
{
    return gSpaceEnergy;
}