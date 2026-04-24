#ifndef LEVEL1_H
#define LEVEL1_H

#include "CS3113/Scene.h"

class Level1 : public Scene
{
private:
    Entity *mGod;

    Vector2 mGodBasePosition;
    float mGodHoverAmplitude;
    float mGodHoverSpeed;

    Color mBackgroundColour;

    std::string mInteractionPrompt;
    Color mPromptBackgroundColour;
    Color mPromptTextColour;

    bool mIsDialogueActive;
    bool mDialogueStarted;
    bool mIntroFinished;
    float mIntroTimer;

    Texture2D mDialogueBoxTexture;
    Texture2D mPortraitSheet;
    Texture2D mItemBoxTexture;

    std::vector<std::string> mDialogueLines;
    int mVisibleDialogueLineCount;
    bool mAwaitingYes;
    int mDialogueStage;

    bool mIsAlbumViewActive;
    Texture2D mAlbumTexture;

    Texture2D mDoorTexture;
    Rectangle mDoorTriggerArea;
    Rectangle mDoorPassageArea;
    Vector2 mDoorPosition;
    Vector2 mDoorScale;
    bool mDoorUnlocked;
    std::string mDoorInput;

    Rectangle mAlbumTriggerArea;
    Vector2 mAlbumPosition;
    Vector2 mAlbumScale;

    bool mPasswordEnergyGiven;

public:
    Level1();
    Level1(Vector2 origin, const char *bgHexCode);
    ~Level1();

    Vector2 getGodPosition() const { return (mGod != nullptr) ? mGod->getPosition() : mGodBasePosition; }

    void startDialogueStage1();
    void startDialogueStage2();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void renderUI() override;
    void shutdown() override;
};

#endif