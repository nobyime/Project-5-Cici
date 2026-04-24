#ifndef LEVEL2_H
#define LEVEL2_H

#include "CS3113/Scene.h"
#include "HouseRoom.h"

class Level2 : public Scene
{
private:
    Texture2D mSchoolBackground;
    Vector2 mSchoolBackgroundPosition;
    Vector2 mSchoolBackgroundScale;

    std::vector<HouseRoom> mRooms;
    std::vector<HouseDoor> mDoors;

    int mCurrentRoomID;

    float mSchoolScaleFactor;
    Vector2 mSchoolTopLeft;

    Entity *mTeacher;
    Entity *mBestie;

    Texture2D mItemBoxTexture;
    Texture2D mItemSheetTexture;

    std::string mInteractionPrompt;
    Color mPromptBackgroundColour;
    Color mPromptTextColour;

    bool mIsItemViewActive;
    bool mItemViewJustOpened;
    int mCurrentItemIndex;

    enum Speaker { NO_SPEAKER, TEACHER, BESTIE };
    Speaker mCurrentSpeaker;
    Speaker getNearbySpeaker();

    bool mIsDialogueActive;
    Texture2D mDialogueBoxTexture;
    Texture2D mPortraitSheet;

    std::string mDialogueText;
    std::vector<std::string> mDialogueLines;
    int mCurrentDialogueLineIndex;
    bool mDialogueAwaitingClose;

    bool mHasDialogueOptions;
    int mSelectedDialogueOption;
    std::vector<std::string> mDialogueOptions;
    bool mDialogueCanClose;

    bool mTalkedToBestie;
    bool mHasStudentCard;
    bool mPendingStudentCardItemView;

    bool mAskedBestieAboutLocker;
    bool mLockerInteractionUnlocked;
    bool mHasBag;
    bool mPendingGoHomeChoice;

    bool mIsLockerViewActive;
    int mLockerStep;
    int mSelectedLockerColumn;
    int mSelectedLockerLayer;
    int mSelectedLockerHasCard;
    std::string mLockerResultText;
    bool mLockerResultCanClose;
    bool mLockerEnergyGiven;

    Rectangle mLockerTriggerArea;

    Texture2D mBagSheetTexture;

    enum ItemType
    {
        ITEM_INSPECT_ONLY,
        ITEM_PICKUP
    };

    struct Item
    {
        Rectangle triggerArea;

        int frameIndex;
        std::string promptText;
        std::string itemName;

        ItemType type;

        bool isVisible;
        bool isCollected;
        bool canInspect;
        bool canPickup;
        bool followsPlayer;

        Vector2 worldPixelPosition;
        Vector2 worldScale;
    };

    std::vector<Item> mItems;

public:
    Level2();
    Level2(Vector2 origin, const char *bgHexCode);
    ~Level2();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void renderUI() override;
    void shutdown() override;
};

#endif