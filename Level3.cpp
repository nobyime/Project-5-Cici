#include "Level3.h"
#include "EnergySystem.h"

constexpr char PROMPT_BG_COLOUR[] = "#D9CBBE";
constexpr char HIGHLIGHT_PURPLE[] = "#8900e1";
constexpr int ITEM_SHEET_COLUMNS = 9;
constexpr int ITEM_FRAME_SIZE = 320;
constexpr Rectangle BAG_DROP_AREA = { 110.0f, 115.0f, 26.0f, 18.0f };
extern Sound gItemSound;
extern Sound gDoorSound;
extern Sound gKorokSound;
extern Sound gFailureSound;

Level3::Level3()                                      : Scene { {0.0f}, nullptr   } {}
Level3::Level3(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}
Level3::~Level3() { }

void Level3::chooseNewTarget(Entity *npc, Vector2 &target, const std::vector<Vector2> &route)
{
    if (route.empty()) return;

    int index = GetRandomValue(0, (int)route.size() - 1);
    target = route[index];
}

void Level3::setNPCDirectionFromMovement(Entity *npc, Vector2 movement)
{
    if (fabs(movement.y) > fabs(movement.x))
    {
        if (movement.y < 0.0f) npc->setDirection(UP);
        else                   npc->setDirection(DOWN);
    }
    else
    {
        if (movement.x < 0.0f) npc->setDirection(LEFT);
        else                   npc->setDirection(RIGHT);
    }
}

void Level3::updateNPCMovement(Entity *npc, Vector2 &target, float &waitTimer, const std::vector<Vector2> &route, float deltaTime)
{
    if (npc == nullptr || route.empty()) return;

    if (waitTimer > 0.0f)
    {
        waitTimer -= deltaTime;
        return;
    }

    Vector2 position = npc->getPosition();

    Vector2 toTarget = {
        target.x - position.x,
        target.y - position.y
    };

    float distance = sqrtf(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

    if (distance < 6.0f)
    {
        waitTimer = (float)GetRandomValue(10, 25) / 10.0f;
        chooseNewTarget(npc, target, route);
        return;
    }

    Vector2 movement = {
        toTarget.x / distance,
        toTarget.y / distance
    };

    setNPCDirectionFromMovement(npc, movement);

    float npcSpeed = 35.0f;

    position.x += movement.x * npcSpeed * deltaTime;
    position.y += movement.y * npcSpeed * deltaTime;

    npc->setPosition(position);
    npc->updateAnimation(deltaTime);
}

Level3::Speaker Level3::getNearbySpeaker()
{
    Vector2 playerPos = mGameState.player->getPosition();

    if (mMom != nullptr && Vector2Distance(playerPos, mMom->getPosition()) < 55.0f)
        return MOMMY;

    if (mDad != nullptr && Vector2Distance(playerPos, mDad->getPosition()) < 55.0f)
        return DADDY;

    if (mSister != nullptr && Vector2Distance(playerPos, mSister->getPosition()) < 55.0f)
        return SISTER;

    return NO_SPEAKER;
}

int Level3::getNearbyItemIndex(Vector2 playerPixelPosition)
{
    for (int i = 0; i < (int)mItems.size(); i++)
    {
        if (!mItems[i].isVisible) continue;
        if (mItems[i].isCollected && mItems[i].itemName != "Bag") continue;

        if (isPointInsideRect(playerPixelPosition, mItems[i].triggerArea))
            return i;
    }

    return -1;
}

void Level3::collectItem(int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= (int)mItems.size()) return;
    if (mItems[itemIndex].isCollected) return;

    mItems[itemIndex].isCollected = true;

    if (mItems[itemIndex].followsPlayer)
        mCollectedItemIndices.push_back(itemIndex);

    if (mItems[itemIndex].itemName == "Key")
        mHasBedroomKey = true;
}

void Level3::drawColoredTextLine(int x, int y, int fontSize, const std::vector<TextSegment> &segments)
{
    int currentX = x;

    for (int i = 0; i < (int)segments.size(); i++)
    {
        DrawText(
            segments[i].text.c_str(),
            currentX,
            y,
            fontSize,
            segments[i].color
        );

        currentX += MeasureText(segments[i].text.c_str(), fontSize);
    }
}

void Level3::renderFollowingItems()
{
    for (int i = 0; i < (int)mCollectedItemIndices.size(); i++)
    {
        int itemIndex = mCollectedItemIndices[i];
        if (itemIndex < 0 || itemIndex >= (int)mItems.size()) continue;

        float frameWidth  = (float)mItemSheetTexture.width / ITEM_SHEET_COLUMNS;
        float frameHeight = (float)mItemSheetTexture.height;

        Rectangle sourceArea = {
            frameWidth * mItems[itemIndex].frameIndex,
            0.0f,
            frameWidth,
            frameHeight
        };

        Vector2 playerPos = mGameState.player->getPosition();
        Direction dir = mGameState.player->getDirection();

        Vector2 offset = { 0.0f, 0.0f };

        bool isKey = (mItems[itemIndex].itemName == "Key");

        if (!isKey)
        {
            if (dir == LEFT || dir == LEFT_DOWN)
                offset = { 18.0f, 10.0f };
            else if (dir == RIGHT || dir == RIGHT_DOWN)
                offset = { -18.0f, 10.0f };
            else if (dir == UP)
                offset = { 0.0f, 18.0f };
            else
                offset = { 0.0f, -18.0f };
        }
        else
        {
            if (dir == LEFT || dir == LEFT_DOWN)
                offset = { -18.0f, -10.0f };
            else if (dir == RIGHT || dir == RIGHT_DOWN)
                offset = { 18.0f, -10.0f };
            else if (dir == UP)
                offset = { 0.0f, -18.0f };
            else
                offset = { 0.0f, 18.0f };
        }

        Rectangle destinationArea = {
            playerPos.x + offset.x,
            playerPos.y + offset.y,
            34.0f,
            34.0f
        };

        DrawTexturePro(
            mItemSheetTexture,
            sourceArea,
            destinationArea,
            { 17.0f, 17.0f },
            0.0f,
            WHITE
        );
    }
}

void Level3::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.facingLeft = false;

    mGameState.map = nullptr;

    std::map<Direction, std::vector<int>> playerAnimationAtlas = {
        { DOWN,       {  0,  1,  2,  3,  4,  5,  6,  7 } },
        { LEFT_DOWN,  {  8,  9, 10, 11, 12, 13, 14, 15 } },
        { LEFT,       { 16, 17, 18, 19, 20, 21, 22, 23 } },
        { UP,         { 24, 25, 26, 27, 28, 29, 30, 31 } },
        { RIGHT,      { 32, 33, 34, 35, 36, 37, 38, 39 } },
        { RIGHT_DOWN, { 40, 41, 42, 43, 44, 45, 46, 47 } }
    };

    std::map<Direction, std::vector<int>> familyAnimationAtlas = {
        { DOWN,       {  0,  1,  2,  3,  4,  5 } },
        { LEFT_DOWN,  {  6,  7,  8,  9, 10, 11 } },
        { LEFT,       {  6,  7,  8,  9, 10, 11 } },
        { UP,         { 12, 13, 14, 15, 16, 17 } },
        { RIGHT,      { 18, 19, 20, 21, 22, 23 } },
        { RIGHT_DOWN, { 18, 19, 20, 21, 22, 23 } }
    };

    mHouseBackground = LoadTexture("images/level3_house.png");

    mHouseBackgroundPosition = {
        mOrigin.x,
        mOrigin.y
    };

    mHouseBackgroundScale = {
        858.0f,
        780.0f
    };

    mHouseScaleFactor = 3.0f;

    mHouseTopLeft = {
        mHouseBackgroundPosition.x - mHouseBackgroundScale.x / 2.0f,
        mHouseBackgroundPosition.y - mHouseBackgroundScale.y / 2.0f
    };

    mRooms.clear();
    mDoors.clear();

    HouseRoom room1(1);
    room1.addWalkableArea({ 29, 51, 26, 122 });
    mRooms.push_back(room1);

    HouseRoom room2(2);
    room2.addWalkableArea({ 61, 36, 37, 55 });
    room2.addWalkableArea({ 98, 63, 58, 28 });
    mRooms.push_back(room2);

    HouseRoom room3(3);
    room3.addWalkableArea({ 61, 97, 95, 46 });
    mRooms.push_back(room3);

    HouseRoom room4(4);
    room4.addWalkableArea({ 61, 149, 43, 52 });
    room4.addWalkableArea({ 82, 201, 22, 21 });
    mRooms.push_back(room4);

    HouseRoom room5(5);
    room5.addWalkableArea({ 110, 149, 72, 92 });
    room5.addWalkableArea({ 182, 188, 17, 53 });
    mRooms.push_back(room5);

    HouseRoom room6(6);
    room6.addWalkableArea({ 162, 36, 25, 107 });
    room6.addWalkableArea({ 187, 36, 40, 16 });
    mRooms.push_back(room6);

    HouseRoom room7(7);
    room7.addWalkableArea({ 193, 58, 76, 85 });
    mRooms.push_back(room7);

    HouseRoom room8(8);
    room8.addWalkableArea({ 188, 162, 17, 20 });
    room8.addWalkableArea({ 205, 162, 33, 79 });
    mRooms.push_back(room8);

    HouseRoom room9(9);
    room9.addWalkableArea({ 61, 10, 95, 20 });
    room9.addWalkableArea({ 104, 30, 52, 27 });
    mRooms.push_back(room9);

    HouseDoor door1(
        { 36, 135, 44, 55 },
        { 44, 147, 28, 31 },
        1,
        4
    );
    door1.texture = LoadTexture("images/level3_door.png");
    door1.frameSize = { 320.0f, 320.0f };
    door1.currentFrame = 0;
    door1.hingePosition = { 60.0f, 165.0f };
    mDoors.push_back(door1);

    HouseDoor door2(
        { 36, 56, 44, 55 },
        { 44, 68, 28, 31 },
        1,
        2
    );
    door2.texture = LoadTexture("images/level3_door.png");
    door2.frameSize = { 320.0f, 320.0f };
    door2.currentFrame = 0;
    door2.hingePosition = { 60.0f, 86.0f };
    mDoors.push_back(door2);

    HouseDoor door3(
        { 85, 159, 44, 55 },
        { 93, 171, 28, 31 },
        4,
        5
    );
    door3.texture = LoadTexture("images/level3_door.png");
    door3.frameSize = { 320.0f, 320.0f };
    door3.currentFrame = 0;
    door3.hingePosition = { 109.0f, 189.0f };
    mDoors.push_back(door3);

    HouseDoor door4(
        { 137, 90, 44, 55 },
        { 145, 102, 28, 31 },
        3,
        6
    );
    door4.texture = LoadTexture("images/level3_door.png");
    door4.frameSize = { 320.0f, 320.0f };
    door4.currentFrame = 0;
    door4.hingePosition = { 161.0f, 120.0f };
    mDoors.push_back(door4);

    HouseDoor door5(
        { 145, 73, 28, 30 },
        { 145, 55, 28, 31 },
        2,
        6
    );
    door5.texture = LoadTexture("images/level3_door.png");
    door5.frameSize = { 320.0f, 320.0f };
    door5.currentFrame = 0;
    door5.hingePosition = { 161.0f, 73.0f };
    mDoors.push_back(door5);

    HouseDoor door6(
        { 145, 52, 28, 21 },
        { 145, 34, 28, 31 },
        6,
        9
    );
    door6.texture = LoadTexture("images/level3_door.png");
    door6.frameSize = { 320.0f, 320.0f };
    door6.currentFrame = 0;
    door6.hingePosition = { 161.0f, 52.0f };
    mDoors.push_back(door6);

    HouseDoor door7(
        { 168, 49, 44, 55 },
        { 176, 61, 28, 31 },
        6,
        7
    );
    door7.texture = LoadTexture("images/level3_door.png");
    door7.frameSize = { 320.0f, 320.0f };
    door7.currentFrame = 0;
    door7.hingePosition = { 192.0f, 79.0f };
    mDoors.push_back(door7);

    HouseDoor door8(
        { 163, 142, 44, 55 },
        { 171, 154, 28, 31 },
        5,
        8
    );
    door8.texture = LoadTexture("images/level3_door.png");
    door8.frameSize = { 320.0f, 320.0f };
    door8.currentFrame = 0;
    door8.hingePosition = { 187.0f, 172.0f };
    mDoors.push_back(door8);

    mGameState.player = new Entity(
        { mOrigin.x - 150.0f, mOrigin.y + 235.0f },
        { 62.0f, 62.0f },
        "images/player_walk.png",
        ATLAS,
        { 8, 6 },
        playerAnimationAtlas,
        PLAYER
    );

    mGameState.player->setSpeed(180);
    mGameState.player->setFrameSpeed(8);
    mGameState.player->setColliderDimensions({
        mGameState.player->getScale().x / 2.0f,
        mGameState.player->getScale().y * 0.8f
    });

    Vector2 sisterWorldPosition = pixelToWorld(
        { 130.0f, 40.0f },
        mHouseTopLeft,
        mHouseScaleFactor
    );

    Vector2 momWorldPosition = pixelToWorld(
        { 146.0f, 205.0f },
        mHouseTopLeft,
        mHouseScaleFactor
    );

    Vector2 dadWorldPosition = pixelToWorld(
        { 231.0f, 100.0f },
        mHouseTopLeft,
        mHouseScaleFactor
    );

    mSister = new Entity(
        sisterWorldPosition,
        { 65.0f, 65.0f },
        "images/sister_walk.png",
        ATLAS,
        { 6, 4 },
        familyAnimationAtlas,
        NPC
    );

    mMom = new Entity(
        momWorldPosition,
        { 67.0f, 67.0f },
        "images/mommy_walk.png",
        ATLAS,
        { 6, 4 },
        familyAnimationAtlas,
        NPC
    );

    mDad = new Entity(
        dadWorldPosition,
        { 70.0f, 70.0f },
        "images/dad_walk.png",
        ATLAS,
        { 6, 4 },
        familyAnimationAtlas,
        NPC
    );

    mSister->setDirection(DOWN);
    mMom->setDirection(DOWN);
    mDad->setDirection(DOWN);

    mSister->setFrameSpeed(12);
    mMom->setFrameSpeed(12);
    mDad->setFrameSpeed(12);

    mMomRoute.clear();
    mDadRoute.clear();
    mSisterRoute.clear();

    mMomRoute.push_back(pixelToWorld({ 190.0f, 225.0f }, mHouseTopLeft, mHouseScaleFactor));
    mMomRoute.push_back(pixelToWorld({ 130.0f, 220.0f }, mHouseTopLeft, mHouseScaleFactor));
    mMomRoute.push_back(pixelToWorld({ 145.0f, 185.0f }, mHouseTopLeft, mHouseScaleFactor));
    mMomRoute.push_back(pixelToWorld({ 160.0f, 230.0f }, mHouseTopLeft, mHouseScaleFactor));
    mMomRoute.push_back(pixelToWorld({ 175.0f, 190.0f }, mHouseTopLeft, mHouseScaleFactor));

    mDadRoute.push_back(pixelToWorld({ 215.0f, 80.0f }, mHouseTopLeft, mHouseScaleFactor));
    mDadRoute.push_back(pixelToWorld({ 230.0f, 100.0f }, mHouseTopLeft, mHouseScaleFactor));
    mDadRoute.push_back(pixelToWorld({ 248.0f, 90.0f }, mHouseTopLeft, mHouseScaleFactor));
    mDadRoute.push_back(pixelToWorld({ 255.0f, 120.0f }, mHouseTopLeft, mHouseScaleFactor));
    mDadRoute.push_back(pixelToWorld({ 210.0f, 130.0f }, mHouseTopLeft, mHouseScaleFactor));
    mDadRoute.push_back(pixelToWorld({ 260.0f, 129.0f }, mHouseTopLeft, mHouseScaleFactor));

    mSisterRoute.push_back(pixelToWorld({ 115.0f, 28.0f }, mHouseTopLeft, mHouseScaleFactor));
    mSisterRoute.push_back(pixelToWorld({ 130.0f, 35.0f }, mHouseTopLeft, mHouseScaleFactor));
    mSisterRoute.push_back(pixelToWorld({ 145.0f, 42.0f }, mHouseTopLeft, mHouseScaleFactor));
    mSisterRoute.push_back(pixelToWorld({ 110.0f, 39.0f }, mHouseTopLeft, mHouseScaleFactor));

    chooseNewTarget(mMom, mMomTarget, mMomRoute);
    chooseNewTarget(mDad, mDadTarget, mDadRoute);
    chooseNewTarget(mSister, mSisterTarget, mSisterRoute);

    mMomWaitTimer = 0.0f;
    mDadWaitTimer = 0.0f;
    mSisterWaitTimer = 0.0f;

    // mSisterWaitTimer = 0.8f;
    // mMomWaitTimer = 1.0f;
    // mDadWaitTimer = 1.5f;

    mCurrentRoomID = 4;

    mInteractionPrompt = "";
    mPromptBackgroundColour = ColorFromHex(PROMPT_BG_COLOUR);
    mPromptTextColour = BLACK;

    mIsDialogueActive = false;
    mCurrentSpeaker = NO_SPEAKER;

    mTalkedToMom = false;
    mMomDialogueFinished = false;
    mMomDialogueStage = 0;
    mMomAskedLunchBox = false;
    mMomNeedsDrink = false;

    mDadDrinkDelivered = false;
    mMomSentToBed = false;
    mCanSleep = false;

    mSisterAskedKey = false;
    mSisterDialogueFinished = false;
    mSisterKnowsKeyLocation = false;

    mDialogueText = "";
    mDialogueLines.clear();
    mCurrentDialogueLineIndex = 0;
    mDialogueAwaitingClose = false;

    mHasDialogueOptions = false;
    mSelectedDialogueOption = 0;
    mDialogueOptions.clear();
    mDialogueCanClose = false;

    mDialogueBoxTexture = LoadTexture("images/dialogue_box.png");
    mPortraitSheet = LoadTexture("images/family_portraits.png");

    mIsItemViewActive = false;
    mCurrentItemIndex = -1;
    mItemViewJustOpened = false;

    mBagPlaced = false;
    mHasBedroomKey = false;
    mRoom3Unlocked = false;
    mHasUsedKey = false;

    mMomEnergyGiven = false;
    mSisterEnergyGiven = false;
    mDadEnergyGiven = false;

    mIsCoffeeChoiceActive = false;
    mSelectedCoffeeIndex = -1;

    mItemBoxTexture = LoadTexture("images/item_box.png");
    mItemSheetTexture = LoadTexture("images/items_sheet.png");

    mItems.clear();
    mCollectedItemIndices.clear();

    Item painting;
    painting.triggerArea = { 72.0f, 165.0f, 24.0f, 14.0f };
    painting.frameIndex = 0;
    painting.promptText = "Press 'V' to take a closer look";
    painting.itemName = "Painting";
    painting.type = ITEM_INSPECT_ONLY;
    painting.isVisible = true;
    painting.isCollected = false;
    painting.canInspect = true;
    painting.canPickup = false;
    painting.followsPlayer = false;
    painting.worldPixelPosition = { 84.0f, 202.0f };
    painting.worldScale = { 60.0f, 60.0f };
    mItems.push_back(painting);

    Item key;
    key.triggerArea = { 80.0f, 50.0f, 16.0f, 10.0f };
    key.frameIndex = 1;
    key.promptText = "Press 'V' to take a closer look";
    key.itemName = "Key";
    key.type = ITEM_PICKUP;
    key.isVisible = true;
    key.isCollected = false;
    key.canInspect = true;
    key.canPickup = true;
    key.followsPlayer = true;
    key.worldPixelPosition = { 128.0f, 26.0f };
    key.worldScale = { 28.0f, 28.0f };
    mItems.push_back(key);

    Item bag;
    bag.triggerArea = BAG_DROP_AREA;
    bag.frameIndex = 2;
    bag.promptText = "Press 'P' to put down the bag";
    bag.itemName = "Bag";
    bag.type = ITEM_BAG;
    bag.isVisible = true;
    bag.isCollected = true;
    bag.canInspect = false;
    bag.canPickup = false;
    bag.followsPlayer = true;
    bag.worldPixelPosition = {
        BAG_DROP_AREA.x + BAG_DROP_AREA.width / 2.0f,
        BAG_DROP_AREA.y + BAG_DROP_AREA.height / 2.0f
    };
    bag.worldScale = { 42.0f, 42.0f };
    mItems.push_back(bag);
    mCollectedItemIndices.push_back((int)mItems.size() - 1);

    Item diary;
    diary.triggerArea = { 135.0f, 165.0f, 24.0f, 14.0f };
    diary.frameIndex = 5;
    diary.promptText = "Press 'V' to take a closer look";
    diary.itemName = "Diary";
    diary.type = ITEM_DIARY;
    diary.isVisible = true;
    diary.isCollected = false;
    diary.canInspect = true;
    diary.canPickup = false;
    diary.followsPlayer = false;
    diary.worldPixelPosition = { 150.0f, 235.0f };
    diary.worldScale = { 46.0f, 46.0f };
    mItems.push_back(diary);

    Item coffeeMachine;
    coffeeMachine.triggerArea = { 115.0f, 223.0f, 18.0f, 18.0f };
    coffeeMachine.frameIndex = 6;
    coffeeMachine.promptText = "Press 'V' to choose coffee";
    coffeeMachine.itemName = "CoffeeMachine";
    coffeeMachine.type = ITEM_COFFEE_MACHINE;
    coffeeMachine.isVisible = true;
    coffeeMachine.isCollected = false;
    coffeeMachine.canInspect = true;
    coffeeMachine.canPickup = false;
    coffeeMachine.followsPlayer = false;
    coffeeMachine.worldPixelPosition = { 193.0f, 165.0f };
    coffeeMachine.worldScale = { 0.0f, 0.0f };
    mItems.push_back(coffeeMachine);

    Item bed;
    bed.triggerArea = { 70.0f, 105.0f, 40.0f, 30.0f };
    bed.frameIndex = 0;
    bed.promptText = "";
    bed.itemName = "Bed";
    bed.type = ITEM_INSPECT_ONLY;
    bed.isVisible = true;
    bed.isCollected = false;
    bed.canInspect = false;
    bed.canPickup = false;
    bed.followsPlayer = false;
    bed.worldPixelPosition = { 0.0f, 0.0f };
    bed.worldScale = { 0.0f, 0.0f };
    mItems.push_back(bed);
}

void Level3::update(float deltaTime)
{
    if (!mIsCoffeeChoiceActive)
    {
        if (IsKeyPressed(KEY_ONE))   mGameState.nextSceneID = 1;
        if (IsKeyPressed(KEY_TWO))   mGameState.nextSceneID = 2;
        if (IsKeyPressed(KEY_THREE)) mGameState.nextSceneID = 3;
    }

    if (mIsDialogueActive)
    {
        mInteractionPrompt = "";

        if (mHasDialogueOptions)
        {
            if (IsKeyPressed(KEY_DOWN))
            {
                mSelectedDialogueOption++;
                if (mSelectedDialogueOption >= (int)mDialogueOptions.size())
                    mSelectedDialogueOption = 0;
            }

            if (IsKeyPressed(KEY_UP))
            {
                mSelectedDialogueOption--;
                if (mSelectedDialogueOption < 0)
                    mSelectedDialogueOption = (int)mDialogueOptions.size() - 1;
            }

            if (IsKeyPressed(KEY_ENTER))
            {
                if (mCurrentSpeaker == MOMMY)
                {
                    if (mSelectedDialogueOption == 2)
                    {
                        mDialogueText = "Alright. Then go make your dad his favorite drink.";
                        mDialogueOptions.clear();
                        mHasDialogueOptions = false;
                        mDialogueCanClose = true;

                        mMomAskedLunchBox = true;
                        mMomNeedsDrink = true;
                        mMomDialogueFinished = true;
                        mTalkedToMom = true;
                        if (!mMomEnergyGiven)
                        {
                            addSpaceEnergy(20.0f);
                            PlaySound(gKorokSound);
                            mMomEnergyGiven = true;
                        }
                    }
                    else
                    {
                        mDialogueText = "That is not where you usually keep it.\nAre you sure?";
                        PlaySound(gFailureSound);
                        mDialogueOptions.clear();
                        mHasDialogueOptions = false;
                        mDialogueCanClose = true;
                    }
                }
                else if (mCurrentSpeaker == SISTER)
                {
                    if (mDialogueText == "Sis, you're back. What do you want?")
                    {
                        if (mSelectedDialogueOption == 0)
                        {
                            mDialogueText = "Answer one question first, then maybe I'll tell you.";
                            mDialogueOptions.clear();
                            mHasDialogueOptions = true;
                            mSelectedDialogueOption = 0;
                            mDialogueOptions.push_back("1. Okay.");
                            mDialogueOptions.push_back("2. Never mind.");
                            mDialogueCanClose = false;
                        }
                        else
                        {
                            mDialogueText = "Okay then.";
                            mDialogueOptions.clear();
                            mHasDialogueOptions = false;
                            mDialogueCanClose = true;
                        }
                    }
                    else if (mDialogueText == "Answer one question first, then maybe I'll tell you.")
                    {
                        if (mSelectedDialogueOption == 0)
                        {
                            mDialogueText = "Do you still remember what my favorite toy is?";
                            mDialogueOptions.clear();
                            mHasDialogueOptions = true;
                            mSelectedDialogueOption = 0;
                            mDialogueOptions.push_back("1. Dragon");
                            mDialogueOptions.push_back("2. Duck");
                            mDialogueOptions.push_back("3. Frog");
                            mDialogueOptions.push_back("4. Doll");
                            mDialogueCanClose = false;
                        }
                        else
                        {
                            mDialogueText = "Then I'm not telling you.";
                            mDialogueOptions.clear();
                            mHasDialogueOptions = false;
                            mDialogueCanClose = true;
                        }
                    }
                    else if (mDialogueText == "Do you still remember what my favorite toy is?")
                    {
                        if (mSelectedDialogueOption == 2)
                        {
                            mDialogueText = "That's right. I saw you near the bookshelf this morning...\nI don't know what you were doing there though.";
                            mDialogueOptions.clear();
                            mHasDialogueOptions = false;
                            mDialogueCanClose = true;

                            mSisterAskedKey = true;
                            mSisterDialogueFinished = true;
                            mSisterKnowsKeyLocation = true;

                            if (!mSisterEnergyGiven)
                            {
                                addSpaceEnergy(20.0f);
                                PlaySound(gKorokSound);
                                mSisterEnergyGiven = true;
                            }
                        }
                        else
                        {
                            mDialogueText = "Nope, it's wrong. I'm not gonna tell you now.";
                            PlaySound(gFailureSound);
                            mDialogueOptions.clear();
                            mHasDialogueOptions = false;
                            mDialogueCanClose = true;

                            mSisterAskedKey = true;
                            mSisterDialogueFinished = true;
                        }
                    }
                }
            }

            return;
        }

        if (IsKeyPressed(KEY_SPACE))
        {
            if (mCurrentDialogueLineIndex < (int)mDialogueLines.size() - 1)
            {
                mCurrentDialogueLineIndex++;
                mDialogueText = mDialogueLines[mCurrentDialogueLineIndex];
            }
            else
            {
                mDialogueAwaitingClose = true;
            }
        }

        if (mDialogueAwaitingClose && IsKeyPressed(KEY_ENTER))
        {
            mIsDialogueActive = false;
            mCurrentSpeaker = NO_SPEAKER;
            mDialogueText = "";
            mDialogueLines.clear();
            mCurrentDialogueLineIndex = 0;
            mDialogueAwaitingClose = false;
            mHasDialogueOptions = false;
            mDialogueOptions.clear();
            mSelectedDialogueOption = 0;
            mDialogueCanClose = false;
        }

        if (mDialogueCanClose && IsKeyPressed(KEY_ENTER))
        {
            mIsDialogueActive = false;
            mCurrentSpeaker = NO_SPEAKER;
            mDialogueText = "";
            mDialogueOptions.clear();
            mHasDialogueOptions = false;
            mSelectedDialogueOption = 0;
            mDialogueCanClose = false;
        }

        return;
    }
    
    if (mIsItemViewActive)
    {
        mInteractionPrompt = "";

        if (mItemViewJustOpened)
        {
            mItemViewJustOpened = false;
            return;
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            if (mCurrentItemIndex >= 0 &&
                mCurrentItemIndex < (int)mItems.size() &&
                mItems[mCurrentItemIndex].type == ITEM_PICKUP &&
                !mItems[mCurrentItemIndex].isCollected)
            {
                collectItem(mCurrentItemIndex);
            }

            mIsItemViewActive = false;
            mCurrentItemIndex = -1;
            mItemViewJustOpened = false;
        }

        return;
    }

    if (mIsCoffeeChoiceActive)
    {
        mInteractionPrompt = "";

        if (IsKeyPressed(KEY_ONE))   mSelectedCoffeeIndex = 7;
        if (IsKeyPressed(KEY_TWO))   mSelectedCoffeeIndex = 6;
        if (IsKeyPressed(KEY_THREE)) mSelectedCoffeeIndex = 8;

        if (IsKeyPressed(KEY_ENTER))
        {
            mIsCoffeeChoiceActive = false;
            mSelectedCoffeeIndex = -1;
            return;
        }

        if (mSelectedCoffeeIndex >= 0)
        {
            int existingCoffeeIndex = -1;

            for (int i = 0; i < (int)mItems.size(); i++)
            {
                if (mItems[i].itemName == "Coffee" && mItems[i].followsPlayer)
                {
                    existingCoffeeIndex = i;
                    break;
                }
            }

            if (existingCoffeeIndex >= 0)
            {
                mItems[existingCoffeeIndex].frameIndex = mSelectedCoffeeIndex;
                mItems[existingCoffeeIndex].isVisible = true;
                mItems[existingCoffeeIndex].isCollected = true;
                mItems[existingCoffeeIndex].followsPlayer = true;
            }
            else
            {
                Item coffee;
                coffee.triggerArea = { 0.0f, 0.0f, 0.0f, 0.0f };
                coffee.frameIndex = mSelectedCoffeeIndex;
                coffee.promptText = "";
                coffee.itemName = "Coffee";
                coffee.type = ITEM_PICKUP;
                coffee.isVisible = true;
                coffee.isCollected = true;
                coffee.canInspect = false;
                coffee.canPickup = false;
                coffee.followsPlayer = true;
                coffee.worldPixelPosition = { 0.0f, 0.0f };
                coffee.worldScale = { 34.0f, 34.0f };

                mItems.push_back(coffee);
                mCollectedItemIndices.push_back((int)mItems.size() - 1);
            }

            mIsCoffeeChoiceActive = false;
        }

        return;
    }

    Vector2 currentWorldPosition = mGameState.player->getPosition();
    Vector2 movement = mGameState.player->getMovement();
    float speed = (float) mGameState.player->getSpeed();
    mInteractionPrompt = "";

    Vector2 colliderInPixels = {
        mGameState.player->getColliderDimensions().x / mHouseScaleFactor,
        mGameState.player->getColliderDimensions().y / mHouseScaleFactor
    };

    Vector2 nextWorldPositionX = currentWorldPosition;
    nextWorldPositionX.x += movement.x * speed * deltaTime;

    Vector2 nextWorldPositionY = currentWorldPosition;
    nextWorldPositionY.y += movement.y * speed * deltaTime;

    HouseRoom *currentRoom = nullptr;

    for (int i = 0; i < (int) mRooms.size(); i++)
    {
        if (mRooms[i].getRoomID() == mCurrentRoomID)
        {
            currentRoom = &mRooms[i];
            break;
        }
    }

    if (currentRoom == nullptr) return;

    Vector2 nextPixelX = worldToPixel(
        nextWorldPositionX,
        mHouseTopLeft,
        mHouseScaleFactor
    );

    nextPixelX.y += colliderInPixels.y / 2.0f;

    Vector2 playerPixelPosition = worldToPixel(
        mGameState.player->getPosition(),
        mHouseTopLeft,
        mHouseScaleFactor
    );

    playerPixelPosition.y += colliderInPixels.y / 2.0f;

    if (mMom != nullptr && Vector2Distance(mGameState.player->getPosition(), mMom->getPosition()) < 55.0f)
            mInteractionPrompt = "Press 'C' to talk";
    else if (mDad != nullptr && Vector2Distance(mGameState.player->getPosition(), mDad->getPosition()) < 55.0f)
        mInteractionPrompt = "Press 'C' to talk";
    else if (mSister != nullptr && Vector2Distance(mGameState.player->getPosition(), mSister->getPosition()) < 55.0f)
        mInteractionPrompt = "Press 'C' to talk";

    if (mInteractionPrompt.empty())
    {
        int nearbyItemIndex = getNearbyItemIndex(playerPixelPosition);

        if (nearbyItemIndex >= 0)
            mInteractionPrompt = mItems[nearbyItemIndex].promptText;
    }

    if (mInteractionPrompt.empty() && mCanSleep)
    {
        for (int i = 0; i < (int)mItems.size(); i++)
        {
            if (mItems[i].itemName == "Bed" &&
                isPointInsideRect(playerPixelPosition, mItems[i].triggerArea))
            {
                mInteractionPrompt = "Press 'Z' to sleep";
                break;
            }
        }
    }

    if (mInteractionPrompt.empty())
    {
        for (int i = 0; i < (int)mDoors.size(); i++)
        {
            if (!mDoors[i].belongsToRoom(mCurrentRoomID)) continue;
            if (!isPointInsideRect(playerPixelPosition, mDoors[i].triggerArea)) continue;

            bool isRoom3LockedDoor =
                ((mDoors[i].roomA == 3 && mDoors[i].roomB == 6) ||
                (mDoors[i].roomA == 6 && mDoors[i].roomB == 3));

            if (isRoom3LockedDoor && !mRoom3Unlocked)
            {
                if (mHasBedroomKey)
                    mInteractionPrompt = "Press 'F' to unlock the door";
                else
                    mInteractionPrompt = "Door locked";
            }
            else
            {
                mInteractionPrompt = "Press 'F' to open the door";
            }

            break;
        }
    }

    if (mInteractionPrompt.empty() && !mBagPlaced && mCurrentRoomID == 3)
    {
        for (int i = 0; i < (int)mItems.size(); i++)
        {
            if (mItems[i].itemName == "Bag" &&
                mItems[i].followsPlayer &&
                isPointInsideRect(playerPixelPosition, mItems[i].triggerArea))
            {
                mInteractionPrompt = "Press 'P' to put down the bag";
                break;
            }
        }
    }

    if (currentRoom->canMoveTo(nextPixelX, colliderInPixels, mDoors))
        currentWorldPosition.x = nextWorldPositionX.x;

    Vector2 mixedWorldPosition = {
        currentWorldPosition.x,
        nextWorldPositionY.y
    };

    Vector2 nextPixelY = worldToPixel(
        mixedWorldPosition,
        mHouseTopLeft,
        mHouseScaleFactor
    );

    nextPixelY.y += colliderInPixels.y / 2.0f;

    if (currentRoom->canMoveTo(nextPixelY, colliderInPixels, mDoors))
        currentWorldPosition.y = nextWorldPositionY.y;

    mGameState.player->setPosition(currentWorldPosition);
    if (movement.x != 0.0f || movement.y != 0.0f)
        mGameState.player->updateAnimation(deltaTime);

    if (IsKeyPressed(KEY_F))
    {
        playerPixelPosition = worldToPixel(
            mGameState.player->getPosition(),
            mHouseTopLeft,
            mHouseScaleFactor
        );

        playerPixelPosition.y += colliderInPixels.y / 2.0f;

        for (int i = 0; i < mDoors.size(); i++)
        {
            if (!mDoors[i].belongsToRoom(mCurrentRoomID)) continue;
            if (!isPointInsideRect(playerPixelPosition, mDoors[i].triggerArea)) continue;

            bool isRoom3LockedDoor =
                ((mDoors[i].roomA == 3 && mDoors[i].roomB == 6) ||
                (mDoors[i].roomA == 6 && mDoors[i].roomB == 3));

            if (isRoom3LockedDoor && !mRoom3Unlocked)
            {
                if (!mHasBedroomKey)
                    break;

                mRoom3Unlocked = true;
                mHasUsedKey = true;
                mHasBedroomKey = false;

                for (int j = 0; j < (int)mItems.size(); j++)
                {
                    if (mItems[j].itemName == "Key")
                    {
                        mItems[j].isVisible = false;
                        mItems[j].isCollected = true;
                        mItems[j].followsPlayer = false;
                        break;
                    }
                }

                for (int j = 0; j < (int)mCollectedItemIndices.size(); j++)
                {
                    int itemIndex = mCollectedItemIndices[j];
                    if (itemIndex >= 0 &&
                        itemIndex < (int)mItems.size() &&
                        mItems[itemIndex].itemName == "Key")
                    {
                        mCollectedItemIndices.erase(mCollectedItemIndices.begin() + j);
                        break;
                    }
                }
            }

            mDoors[i].isOpen = true;
            mDoors[i].currentFrame = 1;
            PlaySound(gDoorSound);
            break;
        }
    }

    if (IsKeyPressed(KEY_P) && !mBagPlaced && mCurrentRoomID == 3)
    {
        for (int i = 0; i < (int)mItems.size(); i++)
        {
            if (mItems[i].itemName == "Bag" &&
                mItems[i].followsPlayer &&
                isPointInsideRect(playerPixelPosition, mItems[i].triggerArea))
            {
                mBagPlaced = true;
                mItems[i].followsPlayer = false;
                mItems[i].canInspect = true;
                mItems[i].promptText = "Press 'V' to take a closer look";
                mItems[i].worldPixelPosition = {
                    BAG_DROP_AREA.x + 10.0f,
                    BAG_DROP_AREA.y + 8.0f
                };

                for (int j = 0; j < (int)mCollectedItemIndices.size(); j++)
                {
                    if (mCollectedItemIndices[j] == i)
                    {
                        mCollectedItemIndices.erase(mCollectedItemIndices.begin() + j);
                        break;
                    }
                }

                break;
            }
        }
    }

    if (IsKeyPressed(KEY_V))
    {
        int nearbyItemIndex = getNearbyItemIndex(playerPixelPosition);

        if (nearbyItemIndex >= 0)
        {
            Item &item = mItems[nearbyItemIndex];

            if (item.type == ITEM_INSPECT_ONLY || item.type == ITEM_DIARY)
            {
                mIsItemViewActive = true;
                mCurrentItemIndex = nearbyItemIndex;
                mItemViewJustOpened = true;
                PlaySound(gItemSound);
            }
            else if (item.type == ITEM_PICKUP)
            {
                mIsItemViewActive = true;
                mCurrentItemIndex = nearbyItemIndex;
                mItemViewJustOpened = true;
                PlaySound(gItemSound);
            }
            else if (item.type == ITEM_BAG)
            {
                if (mBagPlaced)
                {
                    mIsItemViewActive = true;
                    mCurrentItemIndex = nearbyItemIndex;
                    mItemViewJustOpened = true;
                    PlaySound(gItemSound);
                }
            }
            else if (item.type == ITEM_COFFEE_MACHINE)
            {
                mIsCoffeeChoiceActive = true;
                mSelectedCoffeeIndex = -1;
            }
        }
    }

    playerPixelPosition = worldToPixel(
        mGameState.player->getPosition(),
        mHouseTopLeft,
        mHouseScaleFactor
    );

    playerPixelPosition.y += colliderInPixels.y / 2.0f;

    for (int i = 0; i < mDoors.size(); i++)
    {
        if (!mDoors[i].isOpen) continue;
        if (!mDoors[i].belongsToRoom(mCurrentRoomID)) continue;

        if (isPointInsideRect(playerPixelPosition, mDoors[i].passageArea))
        {
            mCurrentRoomID = mDoors[i].getOtherRoom(mCurrentRoomID);
            break;
        }
    }

    for (int i = 0; i < mDoors.size(); i++)
    {
        if (!mDoors[i].isOpen) continue;

        if (!isPointInsideRect(playerPixelPosition, mDoors[i].passageArea))
        {
            mDoors[i].isOpen = false;
            mDoors[i].currentFrame = 0;
        }
    }

    updateNPCMovement(mMom, mMomTarget, mMomWaitTimer, mMomRoute, deltaTime);
    updateNPCMovement(mDad, mDadTarget, mDadWaitTimer, mDadRoute, deltaTime);
    updateNPCMovement(mSister, mSisterTarget, mSisterWaitTimer, mSisterRoute, deltaTime);

    Speaker nearbySpeaker = getNearbySpeaker();

    if (IsKeyPressed(KEY_C) && nearbySpeaker != NO_SPEAKER)
    {
        mIsDialogueActive = true;
        mCurrentSpeaker = nearbySpeaker;

        mDialogueText = "";
        mDialogueLines.clear();
        mCurrentDialogueLineIndex = 0;
        mDialogueAwaitingClose = false;

        mDialogueOptions.clear();
        mHasDialogueOptions = false;
        mSelectedDialogueOption = 0;
        mDialogueCanClose = false;

        if (nearbySpeaker == MOMMY)
        {
            if (!mBagPlaced)
            {
                mDialogueText = "You are back, honey.\nGo put your bag back in your bedroom first.";
                mDialogueCanClose = true;
            }
            else if (!mMomAskedLunchBox)
            {
                mDialogueText = "Where is your lunch box?";
                mHasDialogueOptions = true;
                mSelectedDialogueOption = 0;
                mDialogueOptions.push_back("1. I did not bring a lunch box today.");
                mDialogueOptions.push_back("2. It is at home.");
                mDialogueOptions.push_back("3. It is still in my bag.");
                mDialogueOptions.push_back("4. I do not know.");
                mDialogueCanClose = false;
            }
            else if (mDadDrinkDelivered && !mMomSentToBed)
            {
                mDialogueLines.push_back("Mom, Dad told you to go back to the bedroom and sleep early.");
                mDialogueLines.push_back("Okay, sweetheart. I'll go back to the bedroom soon.");
                mDialogueLines.push_back("You should go to bed now too. You still have class tomorrow.");
                mDialogueLines.push_back("Okay, Mom. Good night.");
                mDialogueLines.push_back("Good night.");

                mDialogueText = mDialogueLines[0];
                mCurrentDialogueLineIndex = 0;
                mMomSentToBed = true;
                mCanSleep = true;
            }
            else if (mMomNeedsDrink && !mDadDrinkDelivered)
            {
                mDialogueText = "Go make your dad his favorite drink.";
                mDialogueCanClose = true;
            }
            else
            {
                mDialogueText = "Go get some rest.";
                mDialogueCanClose = true;
            }
        }
        else if (nearbySpeaker == DADDY)
        {
            if (mDadDrinkDelivered && mMomSentToBed)
            {
                mDialogueText = "Go back to your bedroom and sleep.";
                mDialogueCanClose = true;
            }
            else
            {
                int coffeeIndex = -1;

                for (int i = 0; i < (int)mItems.size(); i++)
                {
                    if (mItems[i].itemName == "Coffee" && mItems[i].followsPlayer)
                    {
                        coffeeIndex = i;
                        break;
                    }
                }

                if (coffeeIndex < 0)
                {
                    mDialogueText = "Do you have something for me, sweetheart?";
                    mDialogueCanClose = true;
                }
                else
                {
                    int coffeeFrame = mItems[coffeeIndex].frameIndex;

                    if (coffeeFrame == 7 || coffeeFrame == 8)
                    {
                        mDialogueText = "My dear daughter,\ndon't you remember which is my favourite coffee?\nAnd I'm lactose intolerant.";
                        PlaySound(gFailureSound);
                        mDialogueCanClose = true;
                    }
                    else if (coffeeFrame == 6)
                    {
                        mDialogueLines.push_back("Thank you, my dear daughter.");
                        mDialogueLines.push_back("I still need to work for a little while.");
                        mDialogueLines.push_back("Tell your mom to go back to the bedroom and sleep early.");
                        mDialogueLines.push_back("And you too. You still have class tomorrow.");
                        mDialogueLines.push_back("Okay, Dad.");

                        mDialogueText = mDialogueLines[0];
                        mCurrentDialogueLineIndex = 0;
                        mDadDrinkDelivered = true;
                        mMomNeedsDrink = false;

                            if (!mDadEnergyGiven)
                            {
                                addSpaceEnergy(20.0f);
                                PlaySound(gKorokSound);
                                mDadEnergyGiven = true;
                            }

                        mItems[coffeeIndex].followsPlayer = false;
                        mItems[coffeeIndex].isVisible = false;
                        mItems[coffeeIndex].isCollected = true;

                        for (int j = 0; j < (int)mCollectedItemIndices.size(); j++)
                        {
                            if (mCollectedItemIndices[j] == coffeeIndex)
                            {
                                mCollectedItemIndices.erase(mCollectedItemIndices.begin() + j);
                                break;
                            }
                        }
                    }
                }
            }
            
        }
        else if (nearbySpeaker == SISTER)
        {
            mDialogueText = "Sis, you're back. What do you want?";
            mHasDialogueOptions = true;
            mSelectedDialogueOption = 0;
            mDialogueOptions.push_back("1. Do you know where my key is?");
            mDialogueOptions.push_back("2. Nothing. Never mind.");
            mDialogueCanClose = false;
        }
    }

    if (mCanSleep && IsKeyPressed(KEY_Z))
    {
        for (int i = 0; i < (int)mItems.size(); i++)
        {
            if (mItems[i].itemName == "Bed" &&
                isPointInsideRect(playerPixelPosition, mItems[i].triggerArea))
            {
                mGameState.nextSceneID = 4;
                break;
            }
        }
    }
}

void Level3::render()
{
    Rectangle sourceArea = {
        0.0f,
        0.0f,
        (float)mHouseBackground.width,
        (float)mHouseBackground.height
    };

    Rectangle destinationArea = {
        mHouseBackgroundPosition.x,
        mHouseBackgroundPosition.y,
        mHouseBackgroundScale.x,
        mHouseBackgroundScale.y
    };

    DrawTexturePro(
        mHouseBackground,
        sourceArea,
        destinationArea,
        { mHouseBackgroundScale.x / 2.0f, mHouseBackgroundScale.y / 2.0f },
        0.0f,
        WHITE
    );

    for (int i = 0; i < (int)mDoors.size(); i++)
    {
        if (mDoors[i].texture.id == 0) continue;

        float frameWidth  = mDoors[i].frameSize.x;
        float frameHeight = mDoors[i].frameSize.y;

        Rectangle sourceArea = {
            frameWidth * mDoors[i].currentFrame,
            0.0f,
            frameWidth,
            frameHeight
        };

        Vector2 hingeWorld = pixelToWorld(
            mDoors[i].hingePosition,
            mHouseTopLeft,
            mHouseScaleFactor
        );

        float doorWorldWidth  = 96.0f;
        float doorWorldHeight = 96.0f;

        Rectangle destinationArea = {
            hingeWorld.x,
            hingeWorld.y,
            doorWorldWidth,
            doorWorldHeight
        };

        Vector2 originOffset = {
            doorWorldWidth  * (240.0f / 320.0f),
            doorWorldHeight * (190.0f / 320.0f)
        };

        DrawTexturePro(
            mDoors[i].texture,
            sourceArea,
            destinationArea,
            originOffset,
            0.0f,
            WHITE
        );
    }

    // **
    // for (int i = 0; i < (int)mItems.size(); i++)
    // {
    //     if (!mItems[i].isVisible) continue;
    //     mItems[i].drawTriggerArea(mHouseTopLeft, mHouseScaleFactor);
    // }

    for (int i = 0; i < (int)mItems.size(); i++)
    {
        if (mItems[i].itemName != "Bag") continue;
        if (mItems[i].followsPlayer) continue;

        float frameWidth  = (float)mItemSheetTexture.width / ITEM_SHEET_COLUMNS;
        float frameHeight = (float)mItemSheetTexture.height;

        Rectangle sourceArea = {
            frameWidth * mItems[i].frameIndex,
            0.0f,
            frameWidth,
            frameHeight
        };

        Vector2 bagWorld = pixelToWorld(
            mItems[i].worldPixelPosition,
            mHouseTopLeft,
            mHouseScaleFactor
        );

        Rectangle destinationArea = {
            bagWorld.x,
            bagWorld.y,
            mItems[i].worldScale.x,
            mItems[i].worldScale.y
        };

        DrawTexturePro(
            mItemSheetTexture,
            sourceArea,
            destinationArea,
            { mItems[i].worldScale.x / 2.0f, mItems[i].worldScale.y / 2.0f },
            0.0f,
            WHITE
        );
    }

    if (mMom != nullptr)    mMom->render();
    if (mDad != nullptr)    mDad->render();
    if (mSister != nullptr) mSister->render();
    renderFollowingItems();
    mGameState.player->render();
}

void Level3::renderUI()
{
    DrawText("LEVEL 3", 20, 20, 30, WHITE);

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
            mInteractionPrompt.c_str(),
            boxX + paddingX,
            boxY + paddingY,
            fontSize,
            mPromptTextColour
        );
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

        int npcPortraitIndex = 0;
        const char *npcName = "";

        if (mCurrentSpeaker == SISTER)
        {
            npcPortraitIndex = 1;
            npcName = "Sister";
        }
        else if (mCurrentSpeaker == DADDY)
        {
            npcPortraitIndex = 2;
            npcName = "Daddy";
        }
        else if (mCurrentSpeaker == MOMMY)
        {
            npcPortraitIndex = 3;
            npcName = "Mommy";
        }

        Rectangle npcPortraitSource = {
            portraitFrameWidth * npcPortraitIndex,
            0.0f,
            portraitFrameWidth,
            portraitFrameHeight
        };

        Rectangle mePortraitSource = {
            0.0f,
            0.0f,
            portraitFrameWidth,
            portraitFrameHeight
        };

        Rectangle npcPortraitDestination = {
            dialogX + 60.0f,
            dialogY + 150.0f,
            90.0f,
            90.0f
        };

        Rectangle mePortraitDestination = {
            dialogX + dialogWidth - 150.0f,
            dialogY + 150.0f,
            90.0f,
            90.0f
        };

        DrawText(
            npcName,
            (int)(dialogX + 65.0f),
            (int)(dialogY + 97.0f),
            24,
            BLACK
        );

        DrawText(
            "Me",
            (int)(dialogX + dialogWidth - 120.0f),
            (int)(dialogY + 97.0f),
            24,
            BLACK
        );

        DrawTexturePro(
            mPortraitSheet,
            npcPortraitSource,
            npcPortraitDestination,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        int dialogueX = (int)(dialogX + 90.0f);
        int dialogueY = (int)(dialogY + 250.0f);
        int dialogueFontSize = 24;

        if (mDialogueText == "You are back, honey.\nGo put your bag back in your bedroom first.")
        {
            DrawText(
                "You are back, honey.",
                dialogueX,
                dialogueY,
                dialogueFontSize,
                BLACK
            );

            drawColoredTextLine(
                dialogueX,
                dialogueY + 32,
                dialogueFontSize,
                {
                    { "Go put your bag back in your ", BLACK },
                    { "bedroom", ColorFromHex(HIGHLIGHT_PURPLE) },
                    { " first.", BLACK }
                }
            );
        }
        else if (mDialogueText == "That's right. I saw you near the bookshelf this morning...\nI don't know what you were doing there though.")
        {
            drawColoredTextLine(
                dialogueX,
                dialogueY,
                dialogueFontSize,
                {
                    { "That's right. I saw you near the ", BLACK },
                    { "bookshelf", ColorFromHex(HIGHLIGHT_PURPLE) },
                    { " this morning...", BLACK }
                }
            );

            DrawText(
                "I don't know what you were doing there though.",
                dialogueX,
                dialogueY + 32,
                dialogueFontSize,
                BLACK
            );
        }
        else if (mDialogueText == "Go back to your bedroom and sleep.")
        {
            drawColoredTextLine(
                dialogueX,
                dialogueY,
                dialogueFontSize,
                {
                    { "Go back to your ", BLACK },
                    { "bedroom", ColorFromHex(HIGHLIGHT_PURPLE) },
                    { " and sleep.", BLACK }
                }
            );
}
        else
        {
            DrawText(
                mDialogueText.c_str(),
                dialogueX,
                dialogueY,
                dialogueFontSize,
                BLACK
            );
        }

        if (mHasDialogueOptions)
        {
            for (int i = 0; i < (int)mDialogueOptions.size(); i++)
            {
                std::string optionText = mDialogueOptions[i];

                if (i == mSelectedDialogueOption)
                    optionText = "> " + optionText;

                DrawText(
                    optionText.c_str(),
                    (int)(dialogX + 110.0f),
                    (int)(dialogY + 300.0f + i * 32.0f),
                    22,
                    BLACK
                );
            }
        }

        DrawTexturePro(
            mPortraitSheet,
            mePortraitSource,
            mePortraitDestination,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );
    }

    if (mIsItemViewActive && mCurrentItemIndex >= 0 && mCurrentItemIndex < (int)mItems.size())
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

        float frameWidth  = (float)mItemSheetTexture.width / ITEM_SHEET_COLUMNS;
        float frameHeight = (float)mItemSheetTexture.height;

        if (mItems[mCurrentItemIndex].itemName == "Bag")
        {
            Rectangle bookSource = {
                frameWidth * 3.0f,
                0.0f,
                frameWidth,
                frameHeight
            };

            Rectangle lunchBoxSource = {
                frameWidth * 4.0f,
                0.0f,
                frameWidth,
                frameHeight
            };

            Rectangle bookDestination = {
                itemBoxX + 210.0f,
                itemBoxY + 120.0f,
                180.0f,
                180.0f
            };

            Rectangle lunchBoxDestination = {
                itemBoxX + 510.0f,
                itemBoxY + 120.0f,
                180.0f,
                180.0f
            };

            DrawTexturePro(
                mItemSheetTexture,
                bookSource,
                bookDestination,
                { 0.0f, 0.0f },
                0.0f,
                WHITE
            );

            DrawTexturePro(
                mItemSheetTexture,
                lunchBoxSource,
                lunchBoxDestination,
                { 0.0f, 0.0f },
                0.0f,
                WHITE
            );

            DrawText("Book", (int)(itemBoxX + 265.0f), (int)(itemBoxY + 330.0f), 22, BLACK);
            DrawText("Lunch Box", (int)(itemBoxX + 535.0f), (int)(itemBoxY + 330.0f), 22, BLACK);
        }
        else
        {
            Rectangle itemSource = {
                frameWidth * mItems[mCurrentItemIndex].frameIndex,
                0.0f,
                frameWidth,
                frameHeight
            };

            Rectangle itemDestination = {
                itemBoxX + 270.0f,
                itemBoxY + 90.0f,
                360.0f,
                360.0f
            };

            DrawTexturePro(
                mItemSheetTexture,
                itemSource,
                itemDestination,
                { 0.0f, 0.0f },
                0.0f,
                WHITE
            );
        }

        if (mItems[mCurrentItemIndex].itemName == "Diary")
        {
            DrawText("Dad likes very bitter iced coffee with no milk.", (int)(itemBoxX + 185.0f), (int)(itemBoxY + 440.0f), 22, BLACK);
        }
    }

    if (mIsCoffeeChoiceActive)
    {
        DrawRectangleRounded(
            { 250.0f, 180.0f, 500.0f, 220.0f },
            0.2f,
            8,
            mPromptBackgroundColour
        );

        DrawText("Choose a coffee", 380, 210, 30, BLACK);
        DrawText("1 : Latte", 330, 260, 24, BLACK);
        DrawText("2 : Americano", 330, 300, 24, BLACK);
        DrawText("3 : Cappuccino", 330, 340, 24, BLACK);
    }

    std::string bottomHint = "";

    if (mIsDialogueActive)
    {
        if (mHasDialogueOptions)
            bottomHint = "Use UP/DOWN to choose, Press ENTER to answer";
        else if (mDialogueAwaitingClose || mDialogueCanClose)
            bottomHint = "Press ENTER to close";
        else
            bottomHint = "Press SPACE to continue";
    }
    else if (mIsItemViewActive)
    {
        if (mCurrentItemIndex >= 0 &&
            mCurrentItemIndex < (int)mItems.size() &&
            mItems[mCurrentItemIndex].type == ITEM_PICKUP &&
            !mItems[mCurrentItemIndex].isCollected)
        {
            bottomHint = "Press ENTER to take it";
        }
        else
        {
            bottomHint = "Press ENTER to close";
        }
    }
    else if (mIsCoffeeChoiceActive)
    {
        bottomHint = "Press 1, 2, or 3 to choose | Press ENTER to close";
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

void Level3::shutdown()
{
    if (mGameState.player != nullptr)
    {
        delete mGameState.player;
        mGameState.player = nullptr;
    }

    if (mMom != nullptr)
    {
        delete mMom;
        mMom = nullptr;
    }

    if (mDad != nullptr)
    {
        delete mDad;
        mDad = nullptr;
    }

    if (mSister != nullptr)
    {
        delete mSister;
        mSister = nullptr;
    }

    if (mGameState.map != nullptr)
    {
        delete mGameState.map;
        mGameState.map = nullptr;
    }

    for (int i = 0; i < (int)mDoors.size(); i++)
    {
        if (mDoors[i].texture.id != 0)
            UnloadTexture(mDoors[i].texture);
    }

    UnloadTexture(mHouseBackground);
    UnloadTexture(mDialogueBoxTexture);
    UnloadTexture(mPortraitSheet);
    UnloadTexture(mItemBoxTexture);
    UnloadTexture(mItemSheetTexture);
}