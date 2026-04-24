#ifndef HOUSE_ROOM_H
#define HOUSE_ROOM_H

#include "raylib.h"
#include <vector>

struct HouseDoor
{
    Rectangle triggerArea;
    Rectangle passageArea;

    bool isOpen;

    int roomA;
    int roomB;

    Texture2D texture;
    Vector2 frameSize;
    int currentFrame;
    Vector2 hingePosition;

    HouseDoor();
    HouseDoor(
        Rectangle newTriggerArea,
        Rectangle newPassageArea,
        int newRoomA,
        int newRoomB
    );

    bool belongsToRoom(int roomID) const;
    int getOtherRoom(int roomID) const;
};

class HouseRoom
{
private:
    int mRoomID;
    std::vector<Rectangle> mWalkableAreas;

public:
    HouseRoom();
    HouseRoom(int roomID);

    void addWalkableArea(Rectangle area);

    int getRoomID() const;
    const std::vector<Rectangle>& getWalkableAreas() const;

    bool containsPoint(Vector2 point) const;

    bool canMoveTo(
        Vector2 playerFeetPosition,
        Vector2 colliderDimensions,
        const std::vector<HouseDoor> &doors
    ) const;

    void drawWalkableAreas(Vector2 topLeft, float scaleFactor) const;
};

bool isPointInsideRect(Vector2 point, Rectangle rect);

Vector2 pixelToWorld(
    Vector2 pixelPoint,
    Vector2 houseTopLeft,
    float scaleFactor
);

Vector2 worldToPixel(
    Vector2 worldPoint,
    Vector2 houseTopLeft,
    float scaleFactor
);

Rectangle pixelRectToWorld(
    Rectangle pixelRect,
    Vector2 houseTopLeft,
    float scaleFactor
);

#endif