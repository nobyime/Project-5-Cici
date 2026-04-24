#include "HouseRoom.h"

HouseDoor::HouseDoor():
    triggerArea { 0.0f, 0.0f, 0.0f, 0.0f },
    passageArea { 0.0f, 0.0f, 0.0f, 0.0f },
    isOpen { false },
    roomA { -1 },
    roomB { -1 },
    texture { NULL },
    frameSize { 320.0f, 320.0f },
    currentFrame { 0 },
    hingePosition { 0.0f, 0.0f } { }

HouseDoor::HouseDoor(
    Rectangle newTriggerArea,
    Rectangle newPassageArea,
    int newRoomA,
    int newRoomB
):
    triggerArea { newTriggerArea },
    passageArea { newPassageArea },
    isOpen { false },
    roomA { newRoomA },
    roomB { newRoomB },
    texture { NULL },
    frameSize { 320.0f, 320.0f },
    currentFrame { 0 },
    hingePosition { 0.0f, 0.0f } { }

bool HouseDoor::belongsToRoom(int roomID) const
{
    return roomA == roomID || roomB == roomID;
}

int HouseDoor::getOtherRoom(int roomID) const
{
    if (roomA == roomID) return roomB;
    if (roomB == roomID) return roomA;
    return -1;
}

HouseRoom::HouseRoom()
    : mRoomID { -1 }, mWalkableAreas {} { }

HouseRoom::HouseRoom(int roomID)
    : mRoomID { roomID }, mWalkableAreas {} { }

void HouseRoom::addWalkableArea(Rectangle area)
{
    mWalkableAreas.push_back(area);
}

int HouseRoom::getRoomID() const
{
    return mRoomID;
}

const std::vector<Rectangle>& HouseRoom::getWalkableAreas() const
{
    return mWalkableAreas;
}

bool isPointInsideRect(Vector2 point, Rectangle rect)
{
    return point.x >= rect.x &&
           point.x <= rect.x + rect.width &&
           point.y >= rect.y &&
           point.y <= rect.y + rect.height;
}

bool HouseRoom::containsPoint(Vector2 point) const
{
    for (int i = 0; i < (int) mWalkableAreas.size(); i++)
    {
        if (isPointInsideRect(point, mWalkableAreas[i]))
            return true;
    }

    return false;
}

bool HouseRoom::canMoveTo(
    Vector2 playerFeetPosition,
    Vector2 colliderDimensions,
    const std::vector<HouseDoor> &doors
) const
{
    float halfWidth = colliderDimensions.x * 0.45f;
    float height    = colliderDimensions.y;

    Vector2 probePoints[5] =
    {
        { playerFeetPosition.x - 1.0f, playerFeetPosition.y },
        { playerFeetPosition.x - halfWidth - 1.0f, playerFeetPosition.y },
        { playerFeetPosition.x + halfWidth - 1.0f, playerFeetPosition.y },
        { playerFeetPosition.x - halfWidth - 1.0f, playerFeetPosition.y - height },
        { playerFeetPosition.x + halfWidth - 1.0f, playerFeetPosition.y - height }
    };

    for (int p = 0; p < 5; p++)
    {
        bool pointIsValid = false;

        for (int i = 0; i < (int) mWalkableAreas.size(); i++)
        {
            if (isPointInsideRect(probePoints[p], mWalkableAreas[i]))
            {
                pointIsValid = true;
                break;
            }
        }

        if (!pointIsValid)
        {
            for (int d = 0; d < (int) doors.size(); d++)
            {
                if (!doors[d].isOpen) continue;
                if (!doors[d].belongsToRoom(mRoomID)) continue;

                if (isPointInsideRect(probePoints[p], doors[d].passageArea))
                {
                    pointIsValid = true;
                    break;
                }
            }
        }

        if (!pointIsValid) return false;
    }

    return true;
}

Vector2 pixelToWorld(
    Vector2 pixelPoint,
    Vector2 houseTopLeft,
    float scaleFactor
)
{
    return {
        houseTopLeft.x + pixelPoint.x * scaleFactor,
        houseTopLeft.y + pixelPoint.y * scaleFactor
    };
}

Vector2 worldToPixel(
    Vector2 worldPoint,
    Vector2 houseTopLeft,
    float scaleFactor
)
{
    return {
        (worldPoint.x - houseTopLeft.x) / scaleFactor,
        (worldPoint.y - houseTopLeft.y) / scaleFactor
    };
}

Rectangle pixelRectToWorld(
    Rectangle pixelRect,
    Vector2 houseTopLeft,
    float scaleFactor
)
{
    return {
        houseTopLeft.x + pixelRect.x * scaleFactor,
        houseTopLeft.y + pixelRect.y * scaleFactor,
        pixelRect.width * scaleFactor,
        pixelRect.height * scaleFactor
    };
}

void HouseRoom::drawWalkableAreas(Vector2 topLeft, float scaleFactor) const
{
    for (int i = 0; i < (int)mWalkableAreas.size(); i++)
    {
        Rectangle worldRect = pixelRectToWorld(mWalkableAreas[i], topLeft, scaleFactor);

        DrawRectangleLines(
            (int)worldRect.x,
            (int)worldRect.y,
            (int)worldRect.width,
            (int)worldRect.height,
            GREEN
        );
    }
}