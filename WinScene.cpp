#include "WinScene.h"

WinScene::WinScene()
    : Scene{ {0.0f}, nullptr }
{
    mBackgroundColour = BLACK;
}

WinScene::WinScene(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode }
{
    mBackgroundColour = (bgHexCode != nullptr) ? ColorFromHex(bgHexCode) : BLACK;
}

WinScene::~WinScene() { }

void WinScene::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.facingLeft = false;
    mGameState.map = nullptr;
    mGameState.player = nullptr;
}

void WinScene::update(float deltaTime)
{
    if (IsKeyPressed(KEY_ENTER))
        mGameState.nextSceneID = 0;
}

void WinScene::render()
{   
}

void WinScene::renderUI()
{
    DrawText("Congratulations!", 290, 220, 52, BLACK);
    DrawText("R.I.P.", 455, 300, 42, WHITE);
    DrawText("Press ENTER to return to Menu", 332, 360, 20, BLACK);
}

void WinScene::shutdown()
{
    if (mGameState.map != nullptr)
    {
        delete mGameState.map;
        mGameState.map = nullptr;
    }

    mGameState.player = nullptr;
}