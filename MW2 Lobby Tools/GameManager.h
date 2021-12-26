/*
Copyright (c) 2016 Scoud

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "CurrentGameManager.h"
#include "MenuNode.h"

class GameManager
{
public:
	static CurrentGameManager *CurrentGame;
	static void SetCurrentGameManager(CurrentGameManager *CurrentGame);
    static std::string GetNextMapMessage(bool& isDlcMap);

	enum BeepState
	{
		ON,
		OFF,
		ERR
	};

	static void SetTeamNameAllies(std::string name);
	static void SetTeamNameAxis(std::string name);

	static void EndGame();
	static void PauseGame();

	static void extraMatchSettings();

	// TODO: implement DVAR toggle nodes in menu
	static void OpenIW4Console();
	static void OpenGameSetup();
	static void GameForceStart();
	static void ToggleForceHost();
	static void ToggleExtraCrates();
    static void ToggleForceStart();
	static void TogglePrivateMatch();
	static void ToggleMaxPlayers();
    static void ToggleAutoHost();
	static void ToggleFullBright();
	static void ToggleFieldOfView();
	static void ToggleInfiniteAmmo();
	static void SetWeaponMode(MenuEvent mevent);

	static void SendChatMessage(std::string message);
	static void SendClientChatMessage(int from, int to, std::string message);
	static void SendKillstreakMessage(int killstreak, int id);
	static void SendDeathstreakMessage(int deathstreak, int id);
	static void DoBeep(BeepState state);
    static void PlayActivationSound(int activated);

	static bool ConsoleOpen;
    static int chatMessagesAvailable;
    static bool thirdPersonEnabled;
	static bool ForceHost;
	static bool ExtraCrates;
    static bool UnlimitedSprint;
	// первый матч как хост
	static bool FirstMatch;
    static bool ForceStart;
	static bool PrivateMatch;
	static bool MaxPlayers;
    static bool CustomMapRandomGenerator;
    static bool AutoHost;
    static bool AutoHostActionRequired;
    static bool CurrentTeam;
    static bool Godmode;
	static bool FullBright;
	static bool FieldOfView;
	static bool InfiniteAmmo;
    static char nextMapName[50];
    static HMODULE hInst;
	static MenuEvent WeaponMode;
};

