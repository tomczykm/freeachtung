#include "Global.h"

SDL_Window *window = nullptr;
SDL_Event events;
SDL_Renderer *renderer = nullptr;

const std::string projectName = PROJECT_NAME;

std::unique_ptr<Logger> logger;
std::unique_ptr<SettingsHandler> settings;

std::unique_ptr<BaseGameState> currentGameState;
GameState stateID = GameState::null, nextState = GameState::null;

int randomInt(int min, int max) {
    if (min >= max) return max;
    return min + (std::rand() % (max - min + 1));
}
