#include "LoseScene.h"

LoseScene::LoseScene()
    : Scene{ {0.0f}, nullptr }
{
    mBackgroundColour = BLACK;
}

LoseScene::LoseScene(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode }
{
    mBackgroundColour = (bgHexCode != nullptr) ? ColorFromHex(bgHexCode) : BLACK;
}

LoseScene::~LoseScene() { }

void LoseScene::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.facingLeft = false;
    mGameState.map = nullptr;
    mGameState.player = nullptr;
}

void LoseScene::update(float deltaTime)
{
    if (IsKeyPressed(KEY_ENTER))
        mGameState.nextSceneID = 0;
}

void LoseScene::render()
{
}

void LoseScene::renderUI()
{
    DrawText("You Failed...", 380, 235, 48, RED);
    DrawText("May your next life be a little kinder.", 217, 305, 34, RED);
    DrawText("Press ENTER to return to Menu", 340, 365, 20, BLACK);
}

void LoseScene::shutdown()
{
    if (mGameState.map != nullptr)
    {
        delete mGameState.map;
        mGameState.map = nullptr;
    }

    mGameState.player = nullptr;
}