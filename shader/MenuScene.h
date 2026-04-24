#ifndef MENU_SCENE_H
#define MENU_SCENE_H

#include "CS3113/Scene.h"

class MenuScene : public Scene
{
private:
    Color mBackgroundColour;

public:
    MenuScene();
    MenuScene(Vector2 origin, const char *bgHexCode);
    ~MenuScene();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void renderUI() override;
    void shutdown() override;
};

#endif