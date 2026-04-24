#include "Level2.h"
#include "EnergySystem.h"

constexpr char PROMPT_BG_COLOUR[] = "#D9CBBE";
constexpr int ITEM_SHEET_COLUMNS = 2;
extern Sound gItemSound;
extern Sound gKorokSound;
extern Sound gFailureSound;

Level2::Level2()                                      : Scene{ {0.0f}, nullptr } {}
Level2::Level2(Vector2 origin, const char *bgHexCode) : Scene{ origin, bgHexCode } {}
Level2::~Level2() { }

Level2::Speaker Level2::getNearbySpeaker()
{
    Vector2 playerPos = mGameState.player->getPosition();

    if (mTeacher != nullptr && Vector2Distance(playerPos, mTeacher->getPosition()) < 55.0f)
        return TEACHER;

    if (mBestie != nullptr && Vector2Distance(playerPos, mBestie->getPosition()) < 55.0f)
        return BESTIE;

    return NO_SPEAKER;
}

void Level2::initialise()
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

    std::map<Direction, std::vector<int>> npcAnimationAtlas = {
        { DOWN,  { 0, 1, 2 } },
        { LEFT,  { 0, 1, 2 } },
        { RIGHT, { 0, 1, 2 } },
        { UP,    { 0, 1, 2 } }
    };

    mSchoolBackground = LoadTexture("images/level2_classroom.png");

    mSchoolBackgroundPosition = {
        mOrigin.x,
        mOrigin.y
    };

    mSchoolBackgroundScale = {
        150.0f * 3.0f,
        200.0f * 3.0f
    };

    mSchoolScaleFactor = 3.0f;

    mSchoolTopLeft = {
        mSchoolBackgroundPosition.x - mSchoolBackgroundScale.x / 2.0f,
        mSchoolBackgroundPosition.y - mSchoolBackgroundScale.y / 2.0f
    };

    mRooms.clear();
    mDoors.clear();

    HouseRoom room1(1);
    room1.addWalkableArea({ 49,  63, 81, 100 });
    room1.addWalkableArea({ 21,  16, 23, 165 });
    room1.addWalkableArea({ 49,  16, 81,  37 });
    room1.addWalkableArea({ 44, 142,  5,  26 });
    room1.addWalkableArea({ 44,  75,  5,  26 });
    room1.addWalkableArea({ 44,  32,  5,  26 });
    mRooms.push_back(room1);

    mCurrentRoomID = 1;

    Vector2 playerWorldPosition = pixelToWorld(
        { 90.0f, 90.0f },
        mSchoolTopLeft,
        mSchoolScaleFactor
    );

    mGameState.player = new Entity(
        playerWorldPosition,
        { 62.0f, 62.0f },
        "images/player_walk.png",
        ATLAS,
        { 8, 6 },
        playerAnimationAtlas,
        PLAYER
    );

    mGameState.player->setSpeed(180);
    mGameState.player->setFrameSpeed(8);
    mGameState.player->setDirection(DOWN);
    mGameState.player->setColliderDimensions({
        mGameState.player->getScale().x / 2.0f,
        mGameState.player->getScale().y * 0.8f
    });

    Vector2 teacherWorldPosition = pixelToWorld(
        { 80.0f, 40.0f },
        mSchoolTopLeft,
        mSchoolScaleFactor
    );

    Vector2 bestieWorldPosition = pixelToWorld(
        { 110.0f, 90.0f },
        mSchoolTopLeft,
        mSchoolScaleFactor
    );

    mTeacher = new Entity(
        teacherWorldPosition,
        { 68.0f, 68.0f },
        "images/teacher.png",
        ATLAS,
        { 3, 1 },
        npcAnimationAtlas,
        NPC
    );

    mBestie = new Entity(
        bestieWorldPosition,
        { 68.0f, 68.0f },
        "images/bestie.png",
        ATLAS,
        { 3, 1 },
        npcAnimationAtlas,
        NPC
    );

    mTeacher->setDirection(DOWN);
    mBestie->setDirection(DOWN);
    mTeacher->setFrameSpeed(6);
    mBestie->setFrameSpeed(6);

    mDialogueBoxTexture = LoadTexture("images/dialogue_box.png");
    mPortraitSheet = LoadTexture("images/school_portraits.png");
    mItemBoxTexture = LoadTexture("images/item_box.png");
    mItemSheetTexture = LoadTexture("images/level2_items.png");
    mBagSheetTexture = LoadTexture("images/items_sheet.png");

    mInteractionPrompt = "";
    mPromptBackgroundColour = ColorFromHex(PROMPT_BG_COLOUR);
    mPromptTextColour = BLACK;

    mIsDialogueActive = false;
    mCurrentSpeaker = NO_SPEAKER;
    mDialogueText = "";
    mDialogueLines.clear();
    mCurrentDialogueLineIndex = 0;
    mDialogueAwaitingClose = false;
    mHasDialogueOptions = false;
    mSelectedDialogueOption = 0;
    mDialogueOptions.clear();
    mDialogueCanClose = false;

    mTalkedToBestie = false;
    mHasStudentCard = false;
    mPendingStudentCardItemView = false;

    mAskedBestieAboutLocker = false;
    mLockerInteractionUnlocked = false;
    mHasBag = false;
    mPendingGoHomeChoice = false;

    mIsLockerViewActive = false;
    mLockerStep = 0;
    mSelectedLockerColumn = 1;
    mSelectedLockerLayer = 0;
    mSelectedLockerHasCard = 0;
    mLockerResultText = "";
    mLockerResultCanClose = false;
    mLockerEnergyGiven = false;

    mLockerTriggerArea = { 49.0f, 83.0f, 81.0f, 20.0f };

    mIsItemViewActive = false;
    mItemViewJustOpened = false;
    mCurrentItemIndex = -1;

    mItems.clear();

    Item book;
    book.triggerArea = { 103.0f - 20.0f, 107.0f - 20.0f, 40.0f, 40.0f };
    book.frameIndex = 0;
    book.promptText = "Press 'V' to look closer";
    book.itemName = "Book";
    book.type = ITEM_INSPECT_ONLY;
    book.isVisible = true;
    book.isCollected = false;
    book.canInspect = true;
    book.canPickup = false;
    book.followsPlayer = false;
    book.worldPixelPosition = { 103.0f, 107.0f };
    book.worldScale = { 0.0f, 0.0f };
    mItems.push_back(book);

    Item studentCard;
    studentCard.triggerArea = { 0.0f, 0.0f, 0.0f, 0.0f };
    studentCard.frameIndex = 1;
    studentCard.promptText = "";
    studentCard.itemName = "StudentCard";
    studentCard.type = ITEM_PICKUP;
    studentCard.isVisible = false;
    studentCard.isCollected = false;
    studentCard.canInspect = true;
    studentCard.canPickup = true;
    studentCard.followsPlayer = false;
    studentCard.worldPixelPosition = { 0.0f, 0.0f };
    studentCard.worldScale = { 0.0f, 0.0f };
    mItems.push_back(studentCard);

    Item bag;
    bag.triggerArea = { 0.0f, 0.0f, 0.0f, 0.0f };
    bag.frameIndex = 2;
    bag.promptText = "";
    bag.itemName = "Bag";
    bag.type = ITEM_PICKUP;
    bag.isVisible = false;
    bag.isCollected = false;
    bag.canInspect = false;
    bag.canPickup = false;
    bag.followsPlayer = false;
    bag.worldPixelPosition = { 0.0f, 0.0f };
    bag.worldScale = { 34.0f, 34.0f };
    mItems.push_back(bag);
}

void Level2::update(float deltaTime)
{
    if (!mIsDialogueActive && !mIsItemViewActive && !mIsLockerViewActive)
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
                if (mCurrentSpeaker == TEACHER)
                {
                    if (mHasStudentCard)
                    {
                        mDialogueText = "No, thank you.";
                        mDialogueOptions.clear();
                        mHasDialogueOptions = false;
                        mDialogueCanClose = true;
                    }
                    else if (mSelectedDialogueOption == 0)
                    {
                        mDialogueText = "Nick? That can't be right.\nYou're a girl, aren't you?";
                        PlaySound(gFailureSound);
                        mDialogueOptions.clear();
                        mHasDialogueOptions = false;
                        mDialogueCanClose = true;
                    }
                    else if (mSelectedDialogueOption == 1)
                    {
                        mDialogueText = "Chelsea is one of my students.\nThat's not you.";
                        PlaySound(gFailureSound);
                        mDialogueOptions.clear();
                        mHasDialogueOptions = false;
                        mDialogueCanClose = true;
                    }
                    else
                    {
                        mDialogueText = "Oh, so you're Rebekah.\nChelsea talks about you all the time.";
                        mDialogueOptions.clear();
                        mHasDialogueOptions = false;
                        mDialogueCanClose = true;
                        mPendingStudentCardItemView = true;

                        if (!mHasStudentCard)
                            addSpaceEnergy(20.0f);
                            PlaySound(gKorokSound);
                    }
                }
                else if (mCurrentSpeaker == BESTIE)
                {
                    if (mHasBag)
                    {
                        if (mSelectedDialogueOption == 0)
                        {
                            mGameState.nextSceneID = 3;
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
                            mPendingGoHomeChoice = false;
                        }
                        else
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
                            mPendingGoHomeChoice = false;
                        }
                    }
                }
            }

            return;
        }

        if (!mDialogueCanClose && !mDialogueAwaitingClose && !mHasDialogueOptions)
        {
            if (IsKeyPressed(KEY_SPACE))
            {
                if (mCurrentDialogueLineIndex < (int)mDialogueLines.size() - 1)
                {
                    mCurrentDialogueLineIndex++;
                    mDialogueText = mDialogueLines[mCurrentDialogueLineIndex];
                }
                else
                {
                    if (mCurrentSpeaker == TEACHER &&
                        !mHasStudentCard &&
                        mTalkedToBestie &&
                        mDialogueLines.size() == 3 &&
                        mDialogueLines[0] == "Teacher, I can't find my student ID.")
                    {
                        mDialogueLines.clear();
                        mHasDialogueOptions = true;
                        mSelectedDialogueOption = 0;
                        mDialogueOptions.clear();
                        mDialogueOptions.push_back("1. Nick");
                        mDialogueOptions.push_back("2. Chelsea");
                        mDialogueOptions.push_back("3. Rebekah");
                    }
                    else if (mCurrentSpeaker == BESTIE &&
                             mHasStudentCard &&
                             !mAskedBestieAboutLocker &&
                             mDialogueLines.size() == 7 &&
                             mDialogueLines[0] == "I got my student ID card back.")
                    {
                        mDialogueAwaitingClose = true;
                        mAskedBestieAboutLocker = true;
                        mLockerInteractionUnlocked = true;
                    }
                    else if (mCurrentSpeaker == BESTIE &&
                             mHasBag &&
                             mPendingGoHomeChoice &&
                             mDialogueLines.size() == 2 &&
                             mDialogueLines[0] == "I got my school bag back.")
                    {
                        mDialogueLines.clear();
                        mHasDialogueOptions = true;
                        mSelectedDialogueOption = 0;
                        mDialogueOptions.clear();
                        mDialogueOptions.push_back("1. Yes");
                        mDialogueOptions.push_back("2. No");
                    }
                    else
                    {
                        mDialogueAwaitingClose = true;
                    }
                }
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
            return;
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

            if (mPendingStudentCardItemView)
            {
                mPendingStudentCardItemView = false;
                mIsItemViewActive = true;
                mCurrentItemIndex = 1;
                mItemViewJustOpened = true;
                PlaySound(gItemSound);
            }

            return;
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
            if (mCurrentItemIndex == 1)
            {
                mHasStudentCard = true;
                mItems[1].isCollected = true;
            }

            mIsItemViewActive = false;
            mCurrentItemIndex = -1;
            mItemViewJustOpened = false;
        }

        return;
    }

    if (mIsLockerViewActive)
    {
        mInteractionPrompt = "";

        if (mLockerResultCanClose)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                if (mLockerResultText == "Locker opened.\nYou got your school bag back.")
                {
                    mHasBag = true;
                    mItems[2].isVisible = true;
                    mItems[2].isCollected = true;
                    mItems[2].followsPlayer = true;

                    if (mBestie != nullptr && mGameState.player != nullptr)
                    {
                        Vector2 playerPos = mGameState.player->getPosition();
                        mBestie->setPosition({ playerPos.x + 70.0f, playerPos.y + 10.0f });
                    }
                }

                mIsLockerViewActive = false;
                mLockerResultCanClose = false;
                mLockerResultText = "";
                mLockerStep = 0;
                mSelectedLockerColumn = 1;
                mSelectedLockerLayer = 0;
                mSelectedLockerHasCard = 0;
            }

            return;
        }

        if (mLockerStep == 0)
        {
            if (IsKeyPressed(KEY_ONE))   mSelectedLockerColumn = 1;
            if (IsKeyPressed(KEY_TWO))   mSelectedLockerColumn = 2;
            if (IsKeyPressed(KEY_THREE)) mSelectedLockerColumn = 3;
            if (IsKeyPressed(KEY_FOUR))  mSelectedLockerColumn = 4;
            if (IsKeyPressed(KEY_FIVE))  mSelectedLockerColumn = 5;
            if (IsKeyPressed(KEY_SIX))   mSelectedLockerColumn = 6;

            if (IsKeyPressed(KEY_ENTER))
                mLockerStep = 1;

            return;
        }
        else if (mLockerStep == 1)
        {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN))
                mSelectedLockerLayer = 1 - mSelectedLockerLayer;

            if (IsKeyPressed(KEY_ENTER))
                mLockerStep = 2;

            return;
        }
        else if (mLockerStep == 2)
        {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN))
                mSelectedLockerHasCard = 1 - mSelectedLockerHasCard;

            if (IsKeyPressed(KEY_ENTER))
            {
                if (!mHasStudentCard || mSelectedLockerHasCard == 1)
                {
                    mLockerResultText = "You need your student ID card to open it.";
                    PlaySound(gFailureSound);
                    mLockerResultCanClose = true;
                }
                else
                {
                    if (mSelectedLockerColumn == 5 && mSelectedLockerLayer == 0)
                    {
                        mLockerResultText = "Locker opened.\nYou got your school bag back.";
                        if (!mLockerEnergyGiven)
                        {
                            addSpaceEnergy(20.0f);
                            PlaySound(gKorokSound);
                            mLockerEnergyGiven = true;
                        }
                    }
                    else
                    {
                        mLockerResultText = "That's not your locker.";
                        PlaySound(gFailureSound);
                    }

                    mLockerResultCanClose = true;
                }
            }

            return;
        }
    }

    if (mGameState.player == nullptr) return;

    Vector2 currentWorldPosition = mGameState.player->getPosition();
    Vector2 movement = mGameState.player->getMovement();
    float speed = (float)mGameState.player->getSpeed();

    Vector2 colliderInPixels = {
        mGameState.player->getColliderDimensions().x / mSchoolScaleFactor,
        mGameState.player->getColliderDimensions().y / mSchoolScaleFactor
    };

    Vector2 nextWorldPositionX = currentWorldPosition;
    nextWorldPositionX.x += movement.x * speed * deltaTime;

    Vector2 nextWorldPositionY = currentWorldPosition;
    nextWorldPositionY.y += movement.y * speed * deltaTime;

    HouseRoom *currentRoom = nullptr;

    for (int i = 0; i < (int)mRooms.size(); i++)
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
        mSchoolTopLeft,
        mSchoolScaleFactor
    );

    nextPixelX.y += colliderInPixels.y / 2.0f;

    if (currentRoom->canMoveTo(nextPixelX, colliderInPixels, mDoors))
        currentWorldPosition.x = nextWorldPositionX.x;

    Vector2 mixedWorldPosition = {
        currentWorldPosition.x,
        nextWorldPositionY.y
    };

    Vector2 nextPixelY = worldToPixel(
        mixedWorldPosition,
        mSchoolTopLeft,
        mSchoolScaleFactor
    );

    nextPixelY.y += colliderInPixels.y / 2.0f;

    if (currentRoom->canMoveTo(nextPixelY, colliderInPixels, mDoors))
        currentWorldPosition.y = nextWorldPositionY.y;

    mGameState.player->setPosition(currentWorldPosition);
    if (movement.x != 0.0f || movement.y != 0.0f)
        mGameState.player->updateAnimation(deltaTime);
    if (mTeacher != nullptr) mTeacher->updateAnimation(deltaTime);
    if (mBestie != nullptr)  mBestie->updateAnimation(deltaTime);

    mInteractionPrompt = "";

    Vector2 playerPixelPosition = worldToPixel(
        mGameState.player->getPosition(),
        mSchoolTopLeft,
        mSchoolScaleFactor
    );

    playerPixelPosition.y += colliderInPixels.y / 2.0f;

    Speaker nearbySpeaker = getNearbySpeaker();

    if (nearbySpeaker != NO_SPEAKER)
    {
        mInteractionPrompt = "Press 'C' to talk";

        if (IsKeyPressed(KEY_C))
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

            if (nearbySpeaker == BESTIE)
            {
                if (!mTalkedToBestie)
                {
                    mDialogueLines.push_back("School's over. Let's go home.");
                    mDialogueLines.push_back("Wait. I can't find my student ID card.");
                    mDialogueLines.push_back("I remember I put my bag in the locker.");
                    mDialogueLines.push_back("Maybe check with the teacher in the office.");
                    mDialogueLines.push_back("The teacher usually keeps lost ID cards.");

                    mDialogueText = mDialogueLines[0];
                    mCurrentDialogueLineIndex = 0;
                    mDialogueAwaitingClose = false;
                    mTalkedToBestie = true;
                }
                else if (mHasBag)
                {
                    mDialogueLines.push_back("I got my school bag back.");
                    mDialogueLines.push_back("That's great. Let's go home.");

                    mDialogueText = mDialogueLines[0];
                    mCurrentDialogueLineIndex = 0;
                    mDialogueAwaitingClose = false;
                    mPendingGoHomeChoice = true;
                }
                else if (mHasStudentCard && !mAskedBestieAboutLocker)
                {
                    mDialogueLines.push_back("I got my student ID card back.");
                    mDialogueLines.push_back("That's great.\nGo get your school bag from your locker.");
                    mDialogueLines.push_back("Do you remember which locker is mine?");
                    mDialogueLines.push_back("Isn't your locker 127? How did you forget that?");
                    mDialogueLines.push_back("Then which one is yours?");
                    mDialogueLines.push_back("Mine is 121. The green one.");
                    mDialogueLines.push_back("Okay. I just suddenly forgot.");

                    mDialogueText = mDialogueLines[0];
                    mCurrentDialogueLineIndex = 0;
                    mDialogueAwaitingClose = false;
                }
                else if (mHasStudentCard && mAskedBestieAboutLocker)
                {
                    mDialogueText = "Go get your bag from your locker.";
                    mDialogueCanClose = true;
                }
                else
                {
                    mDialogueText = "Did you ask the teacher yet?";
                    mDialogueCanClose = true;
                }
            }
            else if (nearbySpeaker == TEACHER)
            {
                if (!mTalkedToBestie)
                {
                    mDialogueText = "Kid, if you don't need anything, go back to your classroom.";
                    mDialogueCanClose = true;
                }
                else if (!mHasStudentCard)
                {
                    mDialogueLines.push_back("Teacher, I can't find my student ID.");
                    mDialogueLines.push_back("I have a few student cards that were left behind.");
                    mDialogueLines.push_back("What's your name?");

                    mDialogueText = mDialogueLines[0];
                    mCurrentDialogueLineIndex = 0;
                }
                else
                {
                    mDialogueText = "You just came to see me.\nIs there anything else I can help you with?";
                    mHasDialogueOptions = true;
                    mSelectedDialogueOption = 0;
                    mDialogueOptions.clear();
                    mDialogueOptions.push_back("1. No, thanks.");
                    mDialogueCanClose = false;
                }
            }
        }

        return;
    }

    if (mLockerInteractionUnlocked &&
        playerPixelPosition.x >= mLockerTriggerArea.x &&
        playerPixelPosition.x <= mLockerTriggerArea.x + mLockerTriggerArea.width &&
        playerPixelPosition.y >= mLockerTriggerArea.y &&
        playerPixelPosition.y <= mLockerTriggerArea.y + mLockerTriggerArea.height)
    {
        mInteractionPrompt = "Press 'F' to open locker";

        if (IsKeyPressed(KEY_F))
        {
            mIsLockerViewActive = true;
            mLockerStep = 0;
            mSelectedLockerColumn = 1;
            mSelectedLockerLayer = 0;
            mSelectedLockerHasCard = 0;
            mLockerResultText = "";
            mLockerResultCanClose = false;
            PlaySound(gItemSound);
        }

        return;
    }

    for (int i = 0; i < (int)mItems.size(); i++)
    {
        if (!mItems[i].isVisible) continue;
        if (mItems[i].itemName != "Book") continue;

        if (isPointInsideRect(playerPixelPosition, mItems[i].triggerArea))
        {
            mInteractionPrompt = mItems[i].promptText;

            if (IsKeyPressed(KEY_V))
            {
                mIsItemViewActive = true;
                mCurrentItemIndex = i;
                mItemViewJustOpened = true;
                PlaySound(gItemSound);
            }

            break;
        }
    }
}

void Level2::render()
{
    Rectangle sourceArea = {
        0.0f,
        0.0f,
        (float)mSchoolBackground.width,
        (float)mSchoolBackground.height
    };

    Rectangle destinationArea = {
        mSchoolBackgroundPosition.x,
        mSchoolBackgroundPosition.y,
        mSchoolBackgroundScale.x,
        mSchoolBackgroundScale.y
    };

    DrawTexturePro(
        mSchoolBackground,
        sourceArea,
        destinationArea,
        { mSchoolBackgroundScale.x / 2.0f, mSchoolBackgroundScale.y / 2.0f },
        0.0f,
        WHITE
    );

    if (mTeacher != nullptr) mTeacher->render();
    if (mBestie != nullptr)  mBestie->render();

    if (mHasBag && mGameState.player != nullptr)
    {
        float frameWidth  = (float)mBagSheetTexture.width / 9.0f;
        float frameHeight = (float)mBagSheetTexture.height;

        Rectangle bagSource = {
            frameWidth * 2.0f,
            0.0f,
            frameWidth,
            frameHeight
        };

        Vector2 playerPos = mGameState.player->getPosition();
        Direction dir = mGameState.player->getDirection();

        Vector2 offset = { 0.0f, 0.0f };

        if (dir == LEFT || dir == LEFT_DOWN)
            offset = { 18.0f, 10.0f };
        else if (dir == RIGHT || dir == RIGHT_DOWN)
            offset = { -18.0f, 10.0f };
        else if (dir == UP)
            offset = { 0.0f, 18.0f };
        else
            offset = { 0.0f, -18.0f };

        Rectangle bagDestination = {
            playerPos.x + offset.x,
            playerPos.y + offset.y,
            34.0f,
            34.0f
        };

        DrawTexturePro(
            mBagSheetTexture,
            bagSource,
            bagDestination,
            { 17.0f, 17.0f },
            0.0f,
            WHITE
        );
    }

    if (mGameState.player != nullptr) mGameState.player->render();

    // for (int i = 0; i < (int)mRooms.size(); i++)
    //     mRooms[i].drawWalkableAreas(mSchoolTopLeft, mSchoolScaleFactor);

    // if (mGameState.player != nullptr)
    // {
    //     Vector2 playerPosition = mGameState.player->getPosition();
    //     Vector2 colliderSize = mGameState.player->getColliderDimensions();

    //     DrawRectangleLines(
    //         (int)(playerPosition.x - colliderSize.x / 2.0f),
    //         (int)(playerPosition.y - colliderSize.y / 2.0f),
    //         (int)colliderSize.x,
    //         (int)colliderSize.y,
    //         RED
    //     );
    // }

    // Rectangle lockerWorldRect = pixelRectToWorld(
    //     mLockerTriggerArea,
    //     mSchoolTopLeft,
    //     mSchoolScaleFactor
    // );

    // DrawRectangleLines(
    //     (int)lockerWorldRect.x,
    //     (int)lockerWorldRect.y,
    //     (int)lockerWorldRect.width,
    //     (int)lockerWorldRect.height,
    //     BLUE
    // );

    // Vector2 colliderInPixels = {
    //     mGameState.player->getColliderDimensions().x / mSchoolScaleFactor,
    //     mGameState.player->getColliderDimensions().y / mSchoolScaleFactor
    // };

    // Vector2 playerPixelPosition = worldToPixel(
    //     mGameState.player->getPosition(),
    //     mSchoolTopLeft,
    //     mSchoolScaleFactor
    // );

    // playerPixelPosition.y += colliderInPixels.y / 2.0f;

    // Vector2 footWorld = pixelToWorld(
    //     playerPixelPosition,
    //     mSchoolTopLeft,
    //     mSchoolScaleFactor
    // );

    // DrawCircle((int)footWorld.x, (int)footWorld.y, 5.0f, YELLOW);
}

void Level2::renderUI()
{
    DrawText("LEVEL 2", 20, 20, 30, WHITE);

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

        float portraitFrameWidth  = (float)mPortraitSheet.width / 3.0f;
        float portraitFrameHeight = (float)mPortraitSheet.height;

        Rectangle mePortraitSource = {
            0.0f,
            0.0f,
            portraitFrameWidth,
            portraitFrameHeight
        };

        int npcPortraitIndex = 0;
        const char *npcName = "";

        if (mCurrentSpeaker == BESTIE)
        {
            npcPortraitIndex = 1;
            npcName = "Bestie";
        }
        else if (mCurrentSpeaker == TEACHER)
        {
            npcPortraitIndex = 2;
            npcName = "Teacher";
        }

        Rectangle npcPortraitSource = {
            portraitFrameWidth * npcPortraitIndex,
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

        DrawTexturePro(
            mPortraitSheet,
            mePortraitSource,
            mePortraitDestination,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        DrawText(
            mDialogueText.c_str(),
            (int)(dialogX + 150.0f),
            (int)(dialogY + 200.0f),
            24,
            BLACK
        );

        if (mHasDialogueOptions)
        {
            for (int i = 0; i < (int)mDialogueOptions.size(); i++)
            {
                std::string optionText = mDialogueOptions[i];

                if (i == mSelectedDialogueOption)
                    optionText = "> " + optionText;

                DrawText(
                    optionText.c_str(),
                    (int)(dialogX + 150.0f),
                    (int)(dialogY + 260.0f + i * 32.0f),
                    22,
                    BLACK
                );
            }
        }
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

        Rectangle itemSource = {
            frameWidth * mItems[mCurrentItemIndex].frameIndex,
            0.0f,
            frameWidth,
            frameHeight
        };

        Rectangle itemDestination = {
            itemBoxX + 120.0f,
            itemBoxY + 110.0f,
            260.0f,
            260.0f
        };

        DrawTexturePro(
            mItemSheetTexture,
            itemSource,
            itemDestination,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        if (mCurrentItemIndex == 0)
        {
            DrawText("This looks like my textbook.", (int)(itemBoxX + 430.0f), (int)(itemBoxY + 150.0f), 24, BLACK);
            DrawText("My name is printed on the cover...", (int)(itemBoxX + 430.0f), (int)(itemBoxY + 200.0f), 24, BLACK);
        }
        else if (mCurrentItemIndex == 1)
        {
            DrawText("So this is my old high school", (int)(itemBoxX + 430.0f), (int)(itemBoxY + 150.0f), 24, BLACK);
            DrawText("student ID card...", (int)(itemBoxX + 430.0f), (int)(itemBoxY + 200.0f), 24, BLACK);
            DrawText("That feels so long ago.", (int)(itemBoxX + 430.0f), (int)(itemBoxY + 250.0f), 24, BLACK);
        }
    }

    if (mIsLockerViewActive)
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

        if (mLockerResultCanClose)
        {
            DrawText(
                mLockerResultText.c_str(),
                (int)(itemBoxX + 220.0f),
                (int)(itemBoxY + 220.0f),
                28,
                BLACK
            );
        }
        else if (mLockerStep == 0)
        {
            DrawText("Which column is your locker?", (int)(itemBoxX + 220.0f), (int)(itemBoxY + 130.0f), 28, BLACK);
            DrawText("Press 1 ~ 6 to choose", (int)(itemBoxX + 270.0f), (int)(itemBoxY + 190.0f), 24, BLACK);

            char columnText[64];
            snprintf(columnText, sizeof(columnText), "Current choice: %d", mSelectedLockerColumn);
            DrawText(columnText, (int)(itemBoxX + 300.0f), (int)(itemBoxY + 260.0f), 26, BLACK);
        }
        else if (mLockerStep == 1)
        {
            DrawText("Upper or Lower?", (int)(itemBoxX + 300.0f), (int)(itemBoxY + 130.0f), 28, BLACK);

            std::string upperText = (mSelectedLockerLayer == 0) ? "> Upper" : "Upper";
            std::string lowerText = (mSelectedLockerLayer == 1) ? "> Lower" : "Lower";

            DrawText(upperText.c_str(), (int)(itemBoxX + 340.0f), (int)(itemBoxY + 220.0f), 26, BLACK);
            DrawText(lowerText.c_str(), (int)(itemBoxX + 340.0f), (int)(itemBoxY + 270.0f), 26, BLACK);
        }
        else if (mLockerStep == 2)
        {
            DrawText("Do you have your student ID card?", (int)(itemBoxX + 180.0f), (int)(itemBoxY + 130.0f), 28, BLACK);

            std::string yesText = (mSelectedLockerHasCard == 0) ? "> Yes" : "Yes";
            std::string noText  = (mSelectedLockerHasCard == 1) ? "> No"  : "No";

            DrawText(yesText.c_str(), (int)(itemBoxX + 360.0f), (int)(itemBoxY + 220.0f), 26, BLACK);
            DrawText(noText.c_str(),  (int)(itemBoxX + 360.0f), (int)(itemBoxY + 270.0f), 26, BLACK);
        }
    }

    std::string bottomHint = "";

    if (mIsDialogueActive)
    {
        if (mHasDialogueOptions)
            bottomHint = "Use UP/DOWN to choose, Press ENTER to answer";
        else if (mDialogueCanClose || mDialogueAwaitingClose)
            bottomHint = "Press ENTER to close";
        else
            bottomHint = "Press SPACE to continue";
    }
    else if (mIsItemViewActive)
    {
        bottomHint = "Press ENTER to close";
    }
    else if (mIsLockerViewActive)
    {
        if (mLockerResultCanClose)
            bottomHint = "Press ENTER to close";
        else if (mLockerStep == 0)
            bottomHint = "Press 1 ~ 6 to choose | Press ENTER to continue";
        else if (mLockerStep == 1)
            bottomHint = "Use UP/DOWN to choose | Press ENTER to continue";
        else if (mLockerStep == 2)
            bottomHint = "Use UP/DOWN to choose | Press ENTER to confirm";
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
            { (float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight },
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

void Level2::shutdown()
{
    if (mGameState.player != nullptr)
    {
        delete mGameState.player;
        mGameState.player = nullptr;
    }

    if (mTeacher != nullptr)
    {
        delete mTeacher;
        mTeacher = nullptr;
    }

    if (mBestie != nullptr)
    {
        delete mBestie;
        mBestie = nullptr;
    }

    if (mGameState.map != nullptr)
    {
        delete mGameState.map;
        mGameState.map = nullptr;
    }

    UnloadTexture(mSchoolBackground);
    UnloadTexture(mDialogueBoxTexture);
    UnloadTexture(mPortraitSheet);
    UnloadTexture(mItemBoxTexture);
    UnloadTexture(mItemSheetTexture);
    UnloadTexture(mBagSheetTexture);
}