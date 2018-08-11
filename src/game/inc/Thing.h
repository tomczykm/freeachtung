#pragma once

#include <SDL.h>
#include <utility>

using Coords = std::pair<double, double>;

class Thing {
public:
    Thing(double xPos, double yPos);
    virtual ~Thing() = default;

    Coords getPos() const;
    virtual SDL_Rect getRenderRect() const;
protected:
    double xPos_, yPos_;
};
