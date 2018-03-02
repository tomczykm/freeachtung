#pragma once

#include "BaseGameState.h"
#include "CTexture.h"
#include "Global.h"
#include "PlayerThing.h"

#include <vector>

enum class GameplayStatus {
	null = 0,
	roundBegin, //at the start of each round; move to RUNNING
	running, //when everybody is furiously trying to kill each other; move to PAUSE, ROUND_END or GAME_END
	pause, //when a player needs a break; move to RUNNING or quit the game in progress
	roundEnd, //when there's only one standing; move to ROUND_BEGIN
	gameEnd //when a player has reached the score goal and is declared a winner; move to another gamestate (TODO)
};

class CStateTest: public CBaseGameState {
public:
	CStateTest ();
	~CStateTest ();

	void Input();
	void Logic();
	void Render();
	void PolledInput();
private:
	CTexture playerTex;
	CTexture trailTex;

	//dimensions for border
	//cannot be static const as they use SettingsHandler::GetResH() to initialize
	const int wt, bx, bw;

	std::vector <CPlayerThing> players;
	std::vector <CTrailThing> trails;

	//for calculating per-pixel movement speed
	Uint32 timer;

	GameplayStatus status;
};
