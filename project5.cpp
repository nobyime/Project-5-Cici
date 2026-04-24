/**
* Author: Xiling Wang
* Assignment: A Burnt-Out Worker
* Date due: , 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"
#include "CS3113/ShaderProgram.h"
#include "MenuScene.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "WinScene.h"
#include "LoseScene.h"
#include "EnergySystem.h"
#include "CS3113/Effects.h"

constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char LEVEL_BG_COLOUR[] = "#B09C8A";
constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Camera2D gCamera = {0};

Scene *gCurrentScene = nullptr;
Scene *gScenes[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
ShaderProgram gShader;
Vector2 gLightPosition = { 0.0f, 0.0f };
Vector2 gGodLightPosition = { 0.0f, 0.0f };
Effects *gEffects = nullptr;
bool gIsTransitioning = false;
int gPendingSceneID = -1;

//music
Music gBackgroundMusic = {0};
Sound gWalkSound = {0};
bool gIsWalkSoundPlaying = false;
Sound gItemSound = {0};
Sound gDoorSound = {0};
Sound gKorokSound = {0};
Sound gFailureSound = {0};
Sound gWinSound = {0};
Sound gLoseSound = {0};

void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Project 5");

    //music
    InitAudioDevice();
    gBackgroundMusic = LoadMusicStream("bgm/Eerie.wav");
    gBackgroundMusic.looping = true;
    SetMusicVolume(gBackgroundMusic, 0.35f);
    PlayMusicStream(gBackgroundMusic);
    gWalkSound = LoadSound("bgm/walk.wav");
    SetSoundPitch(gWalkSound, 1.35f);
    SetSoundVolume(gWalkSound, 0.40f);
    gItemSound = LoadSound("bgm/item.wav");
    SetSoundVolume(gItemSound, 0.60f);
    gDoorSound = LoadSound("bgm/door.wav");
    SetSoundPitch(gDoorSound, 1.50f);
    SetSoundVolume(gDoorSound, 0.70f);
    gKorokSound = LoadSound("bgm/korok.wav");
    SetSoundVolume(gKorokSound, 0.7f);
    gFailureSound = LoadSound("bgm/failure.wav");
    SetSoundVolume(gFailureSound, 0.7f);
    gWinSound = LoadSound("bgm/win.wav");
    gLoseSound = LoadSound("bgm/lose.wav");
    SetSoundVolume(gWinSound, 0.8f);
    SetSoundVolume(gLoseSound, 0.8f);

    gShader.load("shaders/vertex.glsl", "shaders/fragment.glsl");

    gScenes[0] = new MenuScene(ORIGIN, "#ffffffff");
    gScenes[1] = new Level1(ORIGIN, "#ffffffff");
    gScenes[2] = new Level2(ORIGIN, "#ffffffff");
    gScenes[3] = new Level3(ORIGIN, "#ffffffff");
    gScenes[4] = new WinScene(ORIGIN, "#ffffffff");
    gScenes[5] = new LoseScene(ORIGIN, "#ffffffff");

    gCurrentScene = gScenes[0];
    gCurrentScene->initialise();
    resetSpaceEnergy();

    gEffects = new Effects(ORIGIN, (float)SCREEN_WIDTH * 3.0f, (float)SCREEN_HEIGHT * 3.0f);
    gEffects->start(FADEIN);

    gCamera.offset   = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    gCamera.rotation = 0.0f;
    gCamera.zoom     = 1.0f;

    if (gCurrentScene->getState().player != nullptr)
    {
        gCamera.target = gCurrentScene->getState().player->getPosition();
        gLightPosition = gCurrentScene->getState().player->getPosition();
    }

    SetTargetFPS(FPS);
}

void processInput()
{
    if (gCurrentScene->getState().player != nullptr)
    {
        Entity *player = gCurrentScene->getState().player;
        player->resetMovement();

        bool up    = false;
        bool down  = false;
        bool left  = false;
        bool right = false;

        if (IsKeyDown(KEY_W))
        {
            player->moveUp();
            up = true;
        }

        if (IsKeyDown(KEY_S))
        {
            player->moveDown();
            down = true;
        }

        if (IsKeyDown(KEY_A))
        {
            player->moveLeft();
            left = true;
        }

        if (IsKeyDown(KEY_D))
        {
            player->moveRight();
            right = true;
        }

        if (GetLength(player->getMovement()) > 1.0f)
            player->normaliseMovement();
        
        if (dynamic_cast<Level1*>(gCurrentScene) != nullptr ||
            dynamic_cast<Level2*>(gCurrentScene) != nullptr ||
            dynamic_cast<Level3*>(gCurrentScene) != nullptr)
        {
            bool isMoving = (player->getMovement().x != 0.0f || player->getMovement().y != 0.0f);

            if (isMoving)
            {
                if (!gIsWalkSoundPlaying)
                {
                    PlaySound(gWalkSound);
                    gIsWalkSoundPlaying = true;
                }
            }
            else
            {
                if (gIsWalkSoundPlaying)
                {
                    StopSound(gWalkSound);
                    gIsWalkSoundPlaying = false;
                }
            }
        }
        else
        {
            if (gIsWalkSoundPlaying)
            {
                StopSound(gWalkSound);
                gIsWalkSoundPlaying = false;
            }
        }

        if (up && left)
            player->setDirection(UP);
        else if (up && right)
            player->setDirection(UP);
        else if (down && left)
            player->setDirection(LEFT_DOWN);
        else if (down && right)
            player->setDirection(RIGHT_DOWN);
        else if (down)
            player->setDirection(DOWN);
        else if (left)
            player->setDirection(LEFT);
        else if (right)
            player->setDirection(RIGHT);
        else if (up)
            player->setDirection(UP);
    }

    if (IsKeyPressed(KEY_SEVEN))
        gCurrentScene->getState().nextSceneID = 4;

    if (IsKeyPressed(KEY_EIGHT))
        gCurrentScene->getState().nextSceneID = 5;
    if (IsKeyPressed(KEY_Q) || WindowShouldClose())
        gAppStatus = TERMINATED;
}

void update()
{
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    //music
    UpdateMusicStream(gBackgroundMusic);
    if (gIsWalkSoundPlaying && !IsSoundPlaying(gWalkSound))
        PlaySound(gWalkSound);

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;

        if (dynamic_cast<Level1*>(gCurrentScene) != nullptr ||
            dynamic_cast<Level2*>(gCurrentScene) != nullptr ||
            dynamic_cast<Level3*>(gCurrentScene) != nullptr)
        {
            drainSpaceEnergy(FIXED_TIMESTEP);

            if (getSpaceEnergy() <= 0.0f)
            {
                gCurrentScene->shutdown();
                gCurrentScene = gScenes[5];
                gCurrentScene->initialise();
                PlaySound(gLoseSound);
                resetSpaceEnergy();
                gCurrentScene->getState().nextSceneID = -1;
                return;
            }
        }

        if (gCurrentScene->getState().player != nullptr)
        {
            gCamera.target = gCurrentScene->getState().player->getPosition();
            gLightPosition = gCurrentScene->getState().player->getPosition();
        }

        if (dynamic_cast<Level1*>(gCurrentScene) != nullptr)
        {
            Level1 *level1 = (Level1*)gCurrentScene;
            gGodLightPosition = level1->getGodPosition();
        }

        if (gEffects != nullptr)
            gEffects->update(FIXED_TIMESTEP, &gCamera.target);

        if (gIsTransitioning && gEffects != nullptr && gEffects->isFinished())
        {
            gCurrentScene->shutdown();
            gCurrentScene = gScenes[gPendingSceneID];
            gCurrentScene->initialise();

            if (gPendingSceneID == 4)
                PlaySound(gWinSound);
            else if (gPendingSceneID == 5)
                PlaySound(gLoseSound);

            if (gPendingSceneID == 4 || gPendingSceneID == 5)
                resetSpaceEnergy();

            if (gCurrentScene->getState().player != nullptr)
            {
                gCamera.target = gCurrentScene->getState().player->getPosition();
                gLightPosition = gCurrentScene->getState().player->getPosition();
            }

            gEffects->start(FADEIN);
            gPendingSceneID = -1;
            gIsTransitioning = false;
        }

        if (gCurrentScene->getState().nextSceneID >= 0)
        {
            int nextSceneID = gCurrentScene->getState().nextSceneID;

            if ((dynamic_cast<Level1*>(gCurrentScene) != nullptr && nextSceneID == 2) ||
                (dynamic_cast<Level2*>(gCurrentScene) != nullptr && nextSceneID == 3) ||
                (dynamic_cast<Level3*>(gCurrentScene) != nullptr && (nextSceneID == 4 || nextSceneID == 5)))
            {
                gPendingSceneID = nextSceneID;
                gIsTransitioning = true;
                gEffects->start(FADEOUT);
                gCurrentScene->getState().nextSceneID = -1;
            }
            else
            {
                gCurrentScene->shutdown();
                gCurrentScene = gScenes[nextSceneID];
                gCurrentScene->initialise();

                if (nextSceneID == 4 || nextSceneID == 5)
                    resetSpaceEnergy();

                if (gCurrentScene->getState().player != nullptr)
                {
                    gCamera.target = gCurrentScene->getState().player->getPosition();
                    gLightPosition = gCurrentScene->getState().player->getPosition();
                }
            }
        }
    }

    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();

    if (dynamic_cast<Level1*>(gCurrentScene) != nullptr)
        ClearBackground(BLACK);
    else
        ClearBackground(ColorFromHex(LEVEL_BG_COLOUR));

    BeginMode2D(gCamera);

    if (dynamic_cast<Level1*>(gCurrentScene) != nullptr)
    {
        gShader.begin();
        gShader.setVector2("lightPosition", gLightPosition);
        gShader.setVector2("godLightPosition", gGodLightPosition);
        gCurrentScene->render();
        gShader.end();
    }
    else
    {
        gCurrentScene->render();
    }

    EndMode2D();

    gCurrentScene->renderUI();

    if (dynamic_cast<Level1*>(gCurrentScene) != nullptr ||
        dynamic_cast<Level2*>(gCurrentScene) != nullptr ||
        dynamic_cast<Level3*>(gCurrentScene) != nullptr)
    {
        float energy = getSpaceEnergy();

        int barWidth = 220;
        int barHeight = 12;
        int barX = (SCREEN_WIDTH - barWidth) / 2 - 10;
        int barY = 30;

        DrawText("Space Energy", barX + 58, barY - 25, 18, WHITE);

        DrawRectangle(barX, barY, barWidth, barHeight, Fade(WHITE, 0.25f));
        DrawRectangle(barX, barY, (int)(barWidth * (energy / 100.0f)), barHeight, RED);
        DrawRectangleLines(barX, barY, barWidth, barHeight, WHITE);

        char energyText[32];
        snprintf(energyText, sizeof(energyText), "%.1f%%", energy);
        DrawText(energyText, barX + barWidth + 10, barY - 3, 18, WHITE);
    }

    if (gEffects != nullptr)
        gEffects->render();

    EndDrawing();
}

void shutdown()
{
    if (gCurrentScene != nullptr)
        gCurrentScene->shutdown();

    for (int i = 0; i < 6; i++)
    {
        if (gScenes[i] != nullptr)
        {
            delete gScenes[i];
            gScenes[i] = nullptr;
        }
    }

    gCurrentScene = nullptr;

    if (gEffects != nullptr)
    {
        delete gEffects;
        gEffects = nullptr;
    }
    gShader.unload();

    //music
    StopMusicStream(gBackgroundMusic);
    UnloadMusicStream(gBackgroundMusic);
    CloseAudioDevice();
    StopSound(gWalkSound);
    UnloadSound(gWalkSound);
    UnloadSound(gItemSound);
    UnloadSound(gDoorSound);
    UnloadSound(gKorokSound);
    UnloadSound(gFailureSound);
    UnloadSound(gWinSound);
    UnloadSound(gLoseSound);

    CloseWindow();
}

int main()
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();
    return 0;
}