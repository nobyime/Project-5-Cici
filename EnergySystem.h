#ifndef ENERGY_SYSTEM_H
#define ENERGY_SYSTEM_H

extern float gSpaceEnergy;

void resetSpaceEnergy();
void drainSpaceEnergy(float deltaTime);
void addSpaceEnergy(float amount);
float getSpaceEnergy();

#endif