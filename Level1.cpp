#include "Level1.h"
#include "EnergySystem.h"

constexpr char WALKABLE_COLOUR[]  = "#7A7A7A";
constexpr char BLOCKED_COLOUR[]   = "#000000";
constexpr char PROMPT_BG_COLOUR[] = "#D9CBBE";
constexpr char DOOR_PASSWORD[]    = "920421";
extern Sound gItemSound;
extern Sound gKorokSound;

Level1::Level1()
    : Scene{ {0.0f}, nullptr }, mGod(nullptr)
{
    mGodBasePosition = { 0.0f, 0.0f };
    mGodHoverAmplitude = 8.0f;
    mGodHoverSpeed = 2.0f;
    mBackgroundColour = BLACK;
}

Level1::Level1(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode }, mGod(nullptr)
{
    mGodBasePosition = { 0.0f, 0.0f };
    mGodHoverAmplitude = 8.0f;
    mGodHoverSpeed = 2.0f;
    mBackgroundColour = (bgHexCode != nullptr) ? ColorFromHex(bgHexCode) : BLACK;
}

Level1::~Level1() { }

void Level1::startDialogueStage1()
{
    mIsDialogueActive = true;
    mDialogueStarted = true;
    mDialogueStage = 1;
    mAwaitingYes = false;

    mDialogueLines.clear();
    mDialogueLines.push_back("You died after staying up too late for work.");
    mDialogueLines.push_back("You are now in the void after death.");
    mDialogueLines.push_back("Before your death");
    mDialogueLines.push_back("I detected that what you wanted most was a carefree life.");
    mDialogueLines.push_back("Based on your life experience, it was your childhood.");
    mDialogueLines.push_back("So I will send you back to your younger self.");
    mDialogueLines.push_back("Do you understand?");

    mVisibleDialogueLineCount = 1;
}

void Level1::startDialogueStage2()
{
    mDialogueStage = 2;
    mAwaitingYes = false;

    mDialogueLines.clear();
    mDialogueLines.push_back("You have forgotten many things about your childhood.");
    mDialogueLines.push_back("After you return, the space energy will keep fading with time.");
    mDialogueLines.push_back("Your memories are the anchors after the crossing.");
    mDialogueLines.push_back("To maintain the space energy,");
    mDialogueLines.push_back("You must search your past life for the memories you once left.");
    mDialogueLines.push_back("If you interact correctly with the people you met.");
    mDialogueLines.push_back("Then the space energy will increase.");
    mDialogueLines.push_back("If the energy disappears, the space will collapse.");

    mVisibleDialogueLineCount = 1;
}

void Level1::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.facingLeft = false;
    mGameState.map = nullptr;

    std::map<Direction, std::vector<int>> girlAnimationAtlas = {
        { DOWN,       {  0,  1,  2,  3,  4,  5,  6,  7 } },
        { LEFT_DOWN,  {  8,  9, 10, 11, 12, 13, 14, 15 } },
        { LEFT,       { 16, 17, 18, 19, 20, 21, 22, 23 } },
        { UP,         { 24, 25, 26, 27, 28, 29, 30, 31 } },
        { RIGHT,      { 32, 33, 34, 35, 36, 37, 38, 39 } },
        { RIGHT_DOWN, { 40, 41, 42, 43, 44, 45, 46, 47 } }
    };

    std::map<Direction, std::vector<int>> godAnimationAtlas = {
        { DOWN,  { 0, 1, 2 } },
        { LEFT,  { 0, 1, 2 } },
        { RIGHT, { 0, 1, 2 } },
        { UP,    { 0, 1, 2 } }
    };

    mGameState.player = new Entity(
        { mOrigin.x, mOrigin.y },
        { 88.0f, 88.0f },
        "images/player_walk.png",
        ATLAS,
        { 8, 6 },
        girlAnimationAtlas,
        PLAYER
    );

    mGameState.player->setSpeed(140);
    mGameState.player->setFrameSpeed(8);
    mGameState.player->setDirection(RIGHT);
    mGameState.player->setColliderDimensions({
        mGameState.player->getScale().x / 2.0f,
        mGameState.player->getScale().y * 0.8f
    });

    mGodBasePosition = { mOrigin.x + 180.0f, mOrigin.y - 60.0f };

    mGod = new Entity(
        mGodBasePosition,
        { 170.0f, 170.0f },
        "images/god.png",
        ATLAS,
        { 3, 1 },
        godAnimationAtlas,
        NPC
    );

    mGod->setSpeed(0);
    mGod->setFrameSpeed(4);
    mGod->setDirection(DOWN);

    mGodHoverAmplitude = 10.0f;
    mGodHoverSpeed = 2.0f;

    mInteractionPrompt = "";
    mPromptBackgroundColour = ColorFromHex(PROMPT_BG_COLOUR);
    mPromptTextColour = BLACK;

    mIsDialogueActive = false;
    mDialogueStarted = false;
    mIntroFinished = false;
    mIntroTimer = 0.0f;

    mDialogueLines.clear();
    mVisibleDialogueLineCount = 0;
    mAwaitingYes = false;
    mDialogueStage = 0;

    mIsAlbumViewActive = false;

    mPasswordEnergyGiven = false;

    mDialogueBoxTexture = LoadTexture("images/dialogue_box.png");
    mPortraitSheet      = LoadTexture("images/family_portraits.png");
    mItemBoxTexture     = LoadTexture("images/item_box.png");
    mAlbumTexture       = LoadTexture("images/album.png");
    mDoorTexture        = LoadTexture("images/level1_door.png");

    mAlbumPosition = { mOrigin.x - 180.0f, mOrigin.y + 20.0f };
    mAlbumScale    = { 90.0f, 90.0f };
    mAlbumTriggerArea = {
        mAlbumPosition.x - 45.0f,
        mAlbumPosition.y - 45.0f,
        90.0f,
        90.0f
    };

    mDoorPosition = { mOrigin.x + 320.0f, mOrigin.y + 10.0f };
    mDoorScale    = { 120.0f, 180.0f };
    mDoorTriggerArea = {
        mDoorPosition.x - 70.0f,
        mDoorPosition.y - 90.0f,
        140.0f,
        180.0f
    };
    mDoorPassageArea = {
        mDoorPosition.x - 25.0f,
        mDoorPosition.y + 20.0f,
        50.0f,
        90.0f
    };

    mDoorUnlocked = false;
    mDoorInput = "";
}

void Level1::update(float deltaTime)
{
    Vector2 debugPlayerPos = { 0.0f, 0.0f };
    if (mGameState.player != nullptr)
        debugPlayerPos = mGameState.player->getPosition();

    bool debugNearDoor = CheckCollisionPointRec(debugPlayerPos, mDoorTriggerArea);

    if (!debugNearDoor)
    {
        if (IsKeyPressed(KEY_ONE))   mGameState.nextSceneID = 1;
        if (IsKeyPressed(KEY_TWO))   mGameState.nextSceneID = 2;
        if (IsKeyPressed(KEY_THREE)) mGameState.nextSceneID = 3;
    }

    if (!mDialogueStarted)
    {
        mIntroTimer += deltaTime;
        if (mIntroTimer >= 3.0f)
            startDialogueStage1();
    }

    if (mIsDialogueActive)
    {
        mInteractionPrompt = "";

        if (!mAwaitingYes)
        {
            if (IsKeyPressed(KEY_SPACE))
            {
                if (mVisibleDialogueLineCount < (int)mDialogueLines.size())
                    mVisibleDialogueLineCount++;
                else
                    mAwaitingYes = true;
            }
        }
        else
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                if (mDialogueStage == 1)
                {
                    startDialogueStage2();
                }
                else
                {
                    mIsDialogueActive = false;
                    mIntroFinished = true;
                    mDialogueStage = 0;
                    mAwaitingYes = false;
                    mDialogueLines.clear();
                    mVisibleDialogueLineCount = 0;
                }
            }
        }

        return;
    }

    if (mIsAlbumViewActive)
    {
        mInteractionPrompt = "";

        if (IsKeyPressed(KEY_ENTER))
            mIsAlbumViewActive = false;

        return;
    }

    if (mGameState.player == nullptr) return;

    Vector2 movement = { 0.0f, 0.0f };

    bool up    = false;
    bool down  = false;
    bool left  = false;
    bool right = false;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    {
        movement.y = -1.0f;
        up = true;
    }

    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    {
        movement.y = 1.0f;
        down = true;
    }

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        movement.x = -1.0f;
        left = true;
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        movement.x = 1.0f;
        right = true;
    }

    if (GetLength(movement) > 1.0f)
        movement = Vector2Normalize(movement);

    if (up && left)
        mGameState.player->setDirection(UP);
    else if (up && right)
        mGameState.player->setDirection(UP);
    else if (down && left)
        mGameState.player->setDirection(LEFT_DOWN);
    else if (down && right)
        mGameState.player->setDirection(RIGHT_DOWN);
    else if (down)
        mGameState.player->setDirection(DOWN);
    else if (left)
        mGameState.player->setDirection(LEFT);
    else if (right)
        mGameState.player->setDirection(RIGHT);
    else if (up)
        mGameState.player->setDirection(UP);

    mGameState.player->setMovement(movement);

    Vector2 playerPosition = mGameState.player->getPosition();
    float playerSpeed = (float)mGameState.player->getSpeed();

    playerPosition.x += movement.x * playerSpeed * deltaTime;
    playerPosition.y += movement.y * playerSpeed * deltaTime;

    float leftBound   = mOrigin.x - 900.0f / 2.0f;
    float rightBound  = mOrigin.x + 900.0f / 2.0f;
    float topBound    = mOrigin.y - 540.0f / 2.0f;
    float bottomBound = mOrigin.y + 540.0f / 2.0f;

    float halfWidth  = mGameState.player->getScale().x / 2.0f;
    float halfHeight = mGameState.player->getScale().y / 2.0f;

    if (playerPosition.x - halfWidth < leftBound)
        playerPosition.x = leftBound + halfWidth;
    if (playerPosition.x + halfWidth > rightBound)
        playerPosition.x = rightBound - halfWidth;

    if (playerPosition.y - halfHeight < topBound)
        playerPosition.y = topBound + halfHeight;
    if (playerPosition.y + halfHeight > bottomBound)
        playerPosition.y = bottomBound - halfHeight;

    mGameState.player->setPosition(playerPosition);

    if (movement.x != 0.0f || movement.y != 0.0f)
        mGameState.player->updateAnimation(deltaTime);

    if (mGod != nullptr)
    {
        float hoverOffset = sinf((float)GetTime() * mGodHoverSpeed) * mGodHoverAmplitude;

        Vector2 godPosition = mGodBasePosition;
        godPosition.y += hoverOffset;

        mGod->setPosition(godPosition);
        mGod->updateAnimation(deltaTime);
    }

    mInteractionPrompt = "";

    if (mIntroFinished)
    {
        bool nearAlbum = CheckCollisionPointRec(playerPosition, mAlbumTriggerArea);
        bool nearDoor  = CheckCollisionPointRec(playerPosition, mDoorTriggerArea);

        if (nearAlbum)
            mInteractionPrompt = "Press 'V' to look closer";
        else if (nearDoor && !mDoorUnlocked)
            mInteractionPrompt = "Enter 6-digit Password to Open the Door";
        else if (nearDoor && mDoorUnlocked)
            mInteractionPrompt = "Door opened";

        if (nearAlbum && IsKeyPressed(KEY_V))
        {
            mIsAlbumViewActive = true;
            PlaySound(gItemSound);
        }

        if (nearDoor && !mDoorUnlocked)
        {
            if (IsKeyPressed(KEY_ZERO))  mDoorInput += "0";
            if (IsKeyPressed(KEY_ONE))   mDoorInput += "1";
            if (IsKeyPressed(KEY_TWO))   mDoorInput += "2";
            if (IsKeyPressed(KEY_THREE)) mDoorInput += "3";
            if (IsKeyPressed(KEY_FOUR))  mDoorInput += "4";
            if (IsKeyPressed(KEY_FIVE))  mDoorInput += "5";
            if (IsKeyPressed(KEY_SIX))   mDoorInput += "6";
            if (IsKeyPressed(KEY_SEVEN)) mDoorInput += "7";
            if (IsKeyPressed(KEY_EIGHT)) mDoorInput += "8";
            if (IsKeyPressed(KEY_NINE))  mDoorInput += "9";

            if (IsKeyPressed(KEY_BACKSPACE) && !mDoorInput.empty())
                mDoorInput.pop_back();

            if ((int)mDoorInput.size() == 6)
            {
                if (mDoorInput == DOOR_PASSWORD)
                {
                    mDoorUnlocked = true;
                    mDoorInput = "";
                    if (!mPasswordEnergyGiven)
                    {
                        addSpaceEnergy(20.0f);
                        PlaySound(gKorokSound);
                        mPasswordEnergyGiven = true;
                    }
                }
                else
                {
                    mDoorInput = "";
                }
            }
        }
        else if (!nearDoor && !mDoorUnlocked)
        {
            mDoorInput = "";
        }

        if (mDoorUnlocked && CheckCollisionPointRec(playerPosition, mDoorPassageArea))
        {
            mGameState.nextSceneID = 2;
        }
    }
}

void Level1::render()
{
    float walkWidth  = 900.0f;
    float walkHeight = 540.0f;

    float leftBound = mOrigin.x - walkWidth / 2.0f;
    float topBound  = mOrigin.y - walkHeight / 2.0f;

    DrawRectangle(
        (int)(mOrigin.x - 1000.0f / 2.0f),
        (int)(mOrigin.y - 600.0f / 2.0f),
        1000,
        600,
        ColorFromHex(BLOCKED_COLOUR)
    );

    DrawRectangle(
        (int)leftBound,
        (int)topBound,
        (int)walkWidth,
        (int)walkHeight,
        ColorFromHex(WALKABLE_COLOUR)
    );

    if (mIntroFinished)
    {
        Rectangle albumSource = {
            0.0f,
            0.0f,
            (float)mAlbumTexture.width,
            (float)mAlbumTexture.height
        };

        Rectangle albumDestination = {
            mAlbumPosition.x,
            mAlbumPosition.y,
            mAlbumScale.x,
            mAlbumScale.y
        };

        DrawTexturePro(
            mAlbumTexture,
            albumSource,
            albumDestination,
            { mAlbumScale.x / 2.0f, mAlbumScale.y / 2.0f },
            0.0f,
            WHITE
        );

        float frameWidth  = (float)mDoorTexture.width / 2.0f;
        float frameHeight = (float)mDoorTexture.height;

        Rectangle doorSource = {
            frameWidth * (mDoorUnlocked ? 1.0f : 0.0f),
            0.0f,
            frameWidth,
            frameHeight
        };

        Rectangle doorDestination = {
            mDoorPosition.x,
            mDoorPosition.y,
            mDoorScale.x,
            mDoorScale.y
        };

        DrawTexturePro(
            mDoorTexture,
            doorSource,
            doorDestination,
            { mDoorScale.x / 2.0f, mDoorScale.y / 2.0f },
            0.0f,
            WHITE
        );
    }

    if (mGod != nullptr)
        mGod->render();

    if (mGameState.player != nullptr)
        mGameState.player->render();
}

void Level1::renderUI()
{
    DrawText("LEVEL 1", 20, 20, 30, WHITE);

    if (!mInteractionPrompt.empty())
    {
        int fontSize = 18;
        int paddingX = 18;
        int paddingY = 10;

        int textWidth = MeasureText(mInteractionPrompt.c_str(), fontSize);
        int boxWidth = textWidth + paddingX * 2;
        int boxHeight = fontSize + paddingY * 2;
        int boxX = (1000 - boxWidth) / 2;
        int boxY = 600 - 50;

        DrawRectangleRounded(
            { (float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight },
            0.25f,
            8,
            ColorFromHex(PROMPT_BG_COLOUR)
        );

        DrawText(
            mInteractionPrompt.c_str(),
            boxX + paddingX,
            boxY + paddingY,
            fontSize,
            BLACK
        );
    }

    if (mIntroFinished && !mDoorUnlocked)
    {
        Vector2 playerPosition = { 0.0f, 0.0f };
        if (mGameState.player != nullptr)
            playerPosition = mGameState.player->getPosition();

        bool nearDoor = CheckCollisionPointRec(playerPosition, mDoorTriggerArea);

        if (nearDoor)
        {
            std::string passwordText = "Password: " + mDoorInput;
            DrawText(passwordText.c_str(), 360, 80, 28, WHITE);
        }
    }

    if (mIsDialogueActive)
    {
        float dialogWidth  = 900.0f;
        float dialogHeight = 540.0f;
        float dialogX = (1000.0f - dialogWidth) / 2.0f;
        float dialogY = (600.0f - dialogHeight) / 2.0f;

        Rectangle dialogSource = {
            0.0f,
            0.0f,
            (float)mDialogueBoxTexture.width,
            (float)mDialogueBoxTexture.height
        };

        Rectangle dialogDestination = {
            dialogX,
            dialogY,
            dialogWidth,
            dialogHeight
        };

        DrawTexturePro(
            mDialogueBoxTexture,
            dialogSource,
            dialogDestination,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        float portraitFrameWidth  = (float)mPortraitSheet.width / 4.0f;
        float portraitFrameHeight = (float)mPortraitSheet.height;

        Rectangle mePortraitSource = {
            0.0f,
            0.0f,
            portraitFrameWidth,
            portraitFrameHeight
        };

        Rectangle mePortraitDestination = {
            dialogX + dialogWidth - 150.0f,
            dialogY + 150.0f,
            90.0f,
            90.0f
        };

        DrawText("God", (int)(dialogX + 65.0f), (int)(dialogY + 97.0f), 24, BLACK);
        DrawText("Me", (int)(dialogX + dialogWidth - 120.0f), (int)(dialogY + 97.0f), 24, BLACK);

        DrawTexturePro(
            mPortraitSheet,
            mePortraitSource,
            mePortraitDestination,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        int textStartX = (int)(dialogX + 90.0f);
        int textStartY = (int)(dialogY + 180.0f);
        int lineSpacing = 28;

        for (int i = 0; i < mVisibleDialogueLineCount && i < (int)mDialogueLines.size(); i++)
        {
            DrawText(
                mDialogueLines[i].c_str(),
                textStartX,
                textStartY + i * lineSpacing,
                22,
                BLACK
            );
        }

        if (mAwaitingYes)
        {
            DrawText("> Yes", (int)(dialogX + 680.0f), (int)(dialogY + 420.0f), 24, BLACK);
        }
    }

    if (mIsAlbumViewActive)
    {
        float itemBoxWidth  = 900.0f;
        float itemBoxHeight = 540.0f;
        float itemBoxX = (1000.0f - itemBoxWidth) / 2.0f;
        float itemBoxY = (600.0f - itemBoxHeight) / 2.0f;

        Rectangle itemBoxSource = {
            0.0f,
            0.0f,
            (float)mItemBoxTexture.width,
            (float)mItemBoxTexture.height
        };

        Rectangle itemBoxDestination = {
            itemBoxX,
            itemBoxY,
            itemBoxWidth,
            itemBoxHeight
        };

        DrawTexturePro(
            mItemBoxTexture,
            itemBoxSource,
            itemBoxDestination,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        Rectangle albumSource = {
            0.0f,
            0.0f,
            (float)mAlbumTexture.width,
            (float)mAlbumTexture.height
        };

        Rectangle albumDestination = {
            itemBoxX + 80.0f,
            itemBoxY + 80.0f,
            300.0f,
            380.0f
        };

        DrawTexturePro(
            mAlbumTexture,
            albumSource,
            albumDestination,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        DrawText("1992/4/21", (int)(itemBoxX + 500.0f), (int)(itemBoxY + 120.0f), 26, BLACK);
        DrawText("Our daughter was born today.", (int)(itemBoxX + 450.0f), (int)(itemBoxY + 170.0f), 22, BLACK);
        DrawText("She has the same purple hair", (int)(itemBoxX + 450.0f), (int)(itemBoxY + 210.0f), 22, BLACK);
        DrawText("color as me.", (int)(itemBoxX + 450.0f), (int)(itemBoxY + 250.0f), 22, BLACK);
        DrawText("My husband and I decided to", (int)(itemBoxX + 450.0f), (int)(itemBoxY + 290.0f), 22, BLACK);
        DrawText("name her...", (int)(itemBoxX + 450.0f), (int)(itemBoxY + 330.0f), 22, BLACK);
    }

    std::string bottomHint = "";

    if (mIsDialogueActive)
    {
        if (mAwaitingYes)
            bottomHint = "Press ENTER to answer 'Yes'";
        else
            bottomHint = "Press SPACE to continue";
    }
    else if (mIsAlbumViewActive)
    {
        bottomHint = "Press ENTER to close";
    }

    if (!bottomHint.empty())
    {
        int fontSize = 18;
        int paddingX = 18;
        int paddingY = 10;

        int textWidth = MeasureText(bottomHint.c_str(), fontSize);

        int boxWidth = textWidth + paddingX * 2;
        int boxHeight = fontSize + paddingY * 2;

        int boxX = (1000 - boxWidth) / 2;
        int boxY = 600 - 50;

        DrawRectangleRounded(
            {
                (float)boxX,
                (float)boxY,
                (float)boxWidth,
                (float)boxHeight
            },
            0.25f,
            8,
            mPromptBackgroundColour
        );

        DrawText(
            bottomHint.c_str(),
            boxX + paddingX,
            boxY + paddingY,
            fontSize,
            mPromptTextColour
        );
    }
}

void Level1::shutdown()
{
    if (mGameState.player != nullptr)
    {
        delete mGameState.player;
        mGameState.player = nullptr;
    }

    if (mGod != nullptr)
    {
        delete mGod;
        mGod = nullptr;
    }

    if (mGameState.map != nullptr)
    {
        delete mGameState.map;
        mGameState.map = nullptr;
    }

    UnloadTexture(mDialogueBoxTexture);
    UnloadTexture(mPortraitSheet);
    UnloadTexture(mItemBoxTexture);
    UnloadTexture(mAlbumTexture);
    UnloadTexture(mDoorTexture);
}