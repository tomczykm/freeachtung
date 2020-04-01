#pragma once

#include <string>
#include <string_view>
#include <deque>

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>

#include "app/ProfileManager.hpp"
#include "engine/TrailThing.hpp"
#include "engine/Effect.hpp"

struct PlayerInfo {
    std::string name;
    sf::Keyboard::Key left, right;
    sf::Color color;
};

using PlayerInfos = std::map<ProfileId, PlayerInfo>;

class PlayerThing {
public:
    using Score = uint32_t;

    PlayerThing(const PlayerInfo&, float radius, int vel, Timer::Ptr gapSwitchTimer);
    virtual ~PlayerThing() = default;

    void step(double timeStep, std::deque<TrailThing>& trails);
    void input(const sf::Event&);

    void newRoundSetup(uint32_t xPos, uint32_t yPos, std::deque<TrailThing>&);

    const sf::Shape& getShape() const;
    sf::Color getColor() const { return info_.color; }
    int getVelocity() const { return vel_; }

    bool checkCollision(const sf::Shape&) const;

    void addPoint();
    Score getScore() const { return score_; }

    void kill();
    bool isDead() const { return dead_; }
    bool isGap() const { return gap_; }

    const std::string& name() const { return info_.name; }

    void changeVelocity(int d) { vel_ += d; }
    void changeTurn(int d) { turnDegrees_ += d; }
    void setRightAngleMovement(bool);
    void swapControls();

    template <typename... Ts>
    void addTimedEffect(Ts&&... args) {
        effects_.emplace_back(std::forward<Ts>(args)...);
    }

protected:
    void gapSwitch();
    void endExpiredEffects();

    void move(double timeStep, std::deque<TrailThing>& trails);

    void setPosition(float x, float y);

    PlayerInfo info_;

    sf::CircleShape shape_;
    sf::RectangleShape recShape_; // for right angle movement

    double direction_ = 0.0; // in degrees
    int vel_; // pixels per seconds
    double turnDegrees_ = 130;
    bool rightAngleMovement_ = false;

    Score score_ = 0;

    bool dead_ = false;

    bool gap_ = false;
    Timer::Ptr gapSwitchTimer_;

    std::vector<TimedEffect> effects_;
};

class PlayerTestable : public PlayerThing {
public:
    PlayerTestable(float radius, int vel, Timer::Ptr gapSwitchTimer):
        PlayerThing{
            PlayerInfo{"test", sf::Keyboard::Q, sf::Keyboard::W, sf::Color::White},
            radius, vel, gapSwitchTimer
        }
    {}

    void setPosition(float x, float y) {
        PlayerThing::setPosition(x, y);
    }

    sf::Vector2f getPosition() { return shape_.getPosition(); }
    void setDirection(double deg) { direction_ = deg; }
    float getRadius() { return shape_.getRadius(); }
    void setDead(bool dead) { dead_ = dead; }
};