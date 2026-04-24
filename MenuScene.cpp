#include "MenuScene.h"

MenuScene::MenuScene()
    : Scene{ {0.0f}, nullptr }
{
    mBackgroundColour = BLACK;
}

MenuScene::MenuScene(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode }
{
    mBackgroundColour = (bgHexCode != nullptr) ? ColorFromHex(bgHexCode) : BLACK;
}

MenuScene::~MenuScene() { }

void MenuScene::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.facingLeft = false;
    mGameState.map = nullptr;
    mGameState.player = nullptr;
}

void MenuScene::update(float deltaTime)
{
    (void)deltaTime;

    if (IsKeyPressed(KEY_ENTER))
        mGameState.nextSceneID = 1;

    if (IsKeyPressed(KEY_ONE))
        mGameState.nextSceneID = 1;
    if (IsKeyPressed(KEY_TWO))
        mGameState.nextSceneID = 2;
    if (IsKeyPressed(KEY_THREE))
        mGameState.nextSceneID = 3;
}

void MenuScene::render()
{
    
}

void MenuScene::renderUI()
{
    const char *titleText = "A Burnt-Out Worker";
    const char *startText = "Press ENTER to start";
    const char *quitText  = "Press Q to quit";
    const char *moveText  = "[WSAD: Up/Down/Left/Right]";
    const char *levelText = "[1/2/3: Level 1/2/3]";
    const char *chatText  = "[C: Chat with NPC]";
    const char *itemText  = "[V: Check Items]";
    const char *hintText  = "*Pay attention to the key prompts at the bottom of the screen.*";

    DrawText(titleText, (1000 - MeasureText(titleText, 48)) / 2, 70, 48, RED);
    DrawText(startText, (1000 - MeasureText(startText, 28)) / 2, 130, 28, WHITE);
    DrawText(quitText,  (1000 - MeasureText(quitText, 22)) / 2, 560, 22, WHITE);
    DrawText(moveText,  (1000 - MeasureText(moveText, 24)) / 2, 260, 24, WHITE);
    DrawText(levelText, (1000 - MeasureText(levelText, 24)) / 2, 230, 24, WHITE);
    DrawText(chatText,  (1000 - MeasureText(chatText, 24)) / 2, 200, 24, WHITE);
    DrawText(itemText,  (1000 - MeasureText(itemText, 24)) / 2, 170, 24, WHITE);
    DrawText(hintText,  (1000 - MeasureText(hintText, 24)) / 2, 290, 24, BLACK);
}

void MenuScene::shutdown()
{
    if (mGameState.map != nullptr)
    {
        delete mGameState.map;
        mGameState.map = nullptr;
    }

    mGameState.player = nullptr;
}