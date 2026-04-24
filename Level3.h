#ifndef LEVEL3_H
#define LEVEL3_H

#include "CS3113/Scene.h"
#include "HouseRoom.h"

class Level3 : public Scene
{
private:
    Texture2D mHouseBackground;
    Vector2 mHouseBackgroundPosition;
    Vector2 mHouseBackgroundScale;
    std::vector<HouseRoom> mRooms;
    std::vector<HouseDoor> mDoors;

    Entity *mMom;
    Entity *mDad;
    Entity *mSister;

    std::vector<Vector2> mMomRoute;
    std::vector<Vector2> mDadRoute;
    std::vector<Vector2> mSisterRoute;

    Vector2 mMomTarget;
    Vector2 mDadTarget;
    Vector2 mSisterTarget;

    float mMomWaitTimer;
    float mDadWaitTimer;
    float mSisterWaitTimer;

    std::string mInteractionPrompt;
    Color mPromptBackgroundColour;
    Color mPromptTextColour;

    bool mIsDialogueActive;
    Texture2D mDialogueBoxTexture;
    Texture2D mPortraitSheet;
    enum Speaker { NO_SPEAKER, MOMMY, DADDY, SISTER };
    Speaker mCurrentSpeaker;
    Speaker getNearbySpeaker();

    bool mMomEnergyGiven;
    bool mSisterEnergyGiven;
    bool mDadEnergyGiven;

    enum ItemType
    {
        ITEM_INSPECT_ONLY,
        ITEM_PICKUP,
        ITEM_BAG,
        ITEM_DIARY,
        ITEM_COFFEE_MACHINE
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

        // **
        // void drawTriggerArea(Vector2 houseTopLeft, float scaleFactor) const
        // {
        //     Rectangle worldRect = pixelRectToWorld(triggerArea, houseTopLeft, scaleFactor);

        //     DrawRectangleLines(
        //         (int)worldRect.x,
        //         (int)worldRect.y,
        //         (int)worldRect.width,
        //         (int)worldRect.height,
        //         GREEN
        //     );
        // }
    };

    struct TextSegment
    {
        std::string text;
        Color color;
    };

    void drawColoredTextLine(int x, int y, int fontSize, const std::vector<TextSegment> &segments);

    std::vector<Item> mItems;
    std::vector<int> mCollectedItemIndices;

    bool mRoom3Unlocked;
    bool mHasUsedKey;
    bool mItemViewJustOpened;

    bool mTalkedToMom;
    bool mMomDialogueFinished;
    int mMomDialogueStage;
    std::string mDialogueText;

    bool mMomAskedLunchBox;
    bool mMomNeedsDrink;

    bool mSisterAskedKey;
    bool mSisterDialogueFinished;
    bool mSisterKnowsKeyLocation;

    bool mDadDrinkDelivered;
    bool mMomSentToBed;
    bool mCanSleep;

    std::vector<std::string> mDialogueLines;
    int mCurrentDialogueLineIndex;
    bool mDialogueAwaitingClose;

    bool mHasDialogueOptions;
    int mSelectedDialogueOption;
    std::vector<std::string> mDialogueOptions;
    bool mDialogueCanClose;

    bool mIsItemViewActive;
    int mCurrentItemIndex;

    Texture2D mItemBoxTexture;
    Texture2D mItemSheetTexture;

    bool mBagPlaced;
    bool mHasBedroomKey;
    bool mIsCoffeeChoiceActive;
    int mSelectedCoffeeIndex;

    int mCurrentRoomID;

    float mHouseScaleFactor;
    Vector2 mHouseTopLeft;

public:
    Level3();
    Level3(Vector2 origin, const char *bgHexCode);
    ~Level3();

    void chooseNewTarget(Entity *npc, Vector2 &target, const std::vector<Vector2> &route);
    void updateNPCMovement(Entity *npc, Vector2 &target, float &waitTimer, const std::vector<Vector2> &route, float deltaTime);
    void setNPCDirectionFromMovement(Entity *npc, Vector2 movement);

    int getNearbyItemIndex(Vector2 playerPixelPosition);
    void collectItem(int itemIndex);
    void renderFollowingItems();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void renderUI() override;
    void shutdown() override;
};

#endif