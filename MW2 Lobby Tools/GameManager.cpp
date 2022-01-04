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

#include "stdafx.h"
#include "GameManager.h"
#include "resource.h"

#include <future>
#include <string>
#include <map>
#include <algorithm>
#include "MemoryManager.h"
#include "EntityManager.h"
#include "ClientManager.h"
#include "HookManager.h"

#include <Mmsystem.h>
#include <mciapi.h>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")


HMODULE GameManager::hInst = NULL;
bool GameManager::ConsoleOpen = false;
int GameManager::chatMessagesAvailable = 0;
bool GameManager::thirdPersonEnabled = false;
bool GameManager::ForceHost = false;
bool GameManager::ExtraCrates = false;
bool GameManager::UnlimitedSprint = false;
// первый матч как хост
bool GameManager::FirstMatch = true;
bool GameManager::ForceStart = false;
bool GameManager::PrivateMatch = false;
bool GameManager::MaxPlayers = false;
bool GameManager::CustomMapRandomGenerator = false;
bool GameManager::AutoHost = false;
bool GameManager::AutoHostActionRequired = true;
bool GameManager::CurrentTeam = false;
bool GameManager::Godmode = false;
bool GameManager::FullBright = false;
bool GameManager::FieldOfView = true;
bool GameManager::InfiniteAmmo = false;
MenuEvent GameManager::WeaponMode = WEAPONMODE_DEFAULT;

std::map<std::string, std::string> mapNames = 
{
    { "mp_afghan", "Afghan" },
    { "mp_boneyard", "Scrapyard" },
    { "mp_brecourt", "Wasteland" },
    { "mp_checkpoint", "Karachi" },
    { "mp_derail", "Derail" },
    { "mp_estate", "Estate" },
    { "mp_favela", "Favela" },
    { "mp_highrise", "Highrise" },
    { "mp_nightshift", "Skidrow" },
    { "mp_invasion", "Invasion" },
    { "mp_quarry", "Quarry" },
    { "mp_rundown", "Rundown" },
    { "mp_rust", "Rust" },
    { "mp_subbase", "Sub Base" },
    { "mp_terminal", "Terminal" },
    { "mp_underpass", "Underpass" },
    { "mp_favela", "Favela" },
    { "mp_crash", "Crash" },
    { "mp_compact", "Salvage" },
    { "mp_storm", "Storm" },
    { "mp_strike", "Strike" },
    { "mp_trailerpark", "Trailer Park" },
    { "mp_vacant", "Vacant" },
    { "mp_fuel2", "Fuel" },
    { "mp_abandon", "Carnival" },
    { "mp_overgrown", "Overgrown" },
    { "mp_complex", "Bailout" }
};

std::vector <std::string> dlcMaps = 
{
    "mp_complex",
    "mp_overgrown",
    "mp_complex",
    "mp_abandon",
    "mp_crash",
    "mp_compact",
    "mp_storm",
    "mp_fuel2",
    "mp_trailerpark",
    "mp_vacant",
    "mp_strike"
};

CurrentGameManager *GameManager::CurrentGame;

void GameManager::SetCurrentGameManager(CurrentGameManager * CurrentGame)
{
	GameManager::CurrentGame = CurrentGame;
}

void GameManager::SetTeamNameAllies(std::string name)
{
	IW4::DVAR::SetStringByName("g_teamname_allies", name.c_str());
}

void GameManager::SetTeamNameAxis(std::string name)
{
	IW4::DVAR::SetStringByName("g_teamname_axis", name.c_str());
}

void GameManager::EndGame()
{
    int serverId = *reinterpret_cast<int*>(0x00B58150);
    char buffer[100] = { 0 };
    sprintf(buffer, "cmd mr %d -1 endround\n", serverId);
    IW4::CBUF::AddText(0, &buffer[0]);
}

void GameManager::PauseGame()
{
	// TODO: implement
}

void GameManager::OpenIW4Console()
{
	if (!ConsoleOpen)
	{
		//DoBeep(ON);
        PlayActivationSound(1);
		ConsoleOpen = true;
		printf("console open\n");
		//IW4::UNKN::CreateConsole();
	}
	else
	{
		ConsoleOpen = false;
		DoBeep(ERR);
	}
}

void GameManager::OpenGameSetup()
{
	DoBeep(ON);
	IW4::UI::OpenMenu(0, "popup_gamesetup");
}

void GameManager::GameForceStart()
{
	//DoBeep(ON);

	auto party_teamBased = IW4::DVAR::FindDvar("party_teambased");
	auto xblive_privatematch = IW4::DVAR::FindDvar("xblive_privatematch");
	std::string gamemode(IW4::DVAR::GetString("ui_gametype"));
	IW4::DVAR::SetBoolByName("party_teambased", gamemode != "dm");

	xblive_privatematch->current.enabled = true;

	IW4::PARTYHOST::BalanceTeams(reinterpret_cast<void*>(G_LOBBYDATA));
	IW4::PARTYHOST::BalanceTeams(reinterpret_cast<void*>(PARTYSESSION_P));
	IW4::PARTYHOST::StartMatch(reinterpret_cast<void*>(G_LOBBYDATA), 0);
	//((void(__cdecl*) ())0x4C5170)();

	xblive_privatematch->current.enabled = false;
}

void GameManager::ToggleExtraCrates()
{
	if (ExtraCrates)
	{
		printf("ExtraCrates disabled\n");
		PlayActivationSound(0);
	}
	else
	{
		printf("ExtraCrates enabled\n");
		PlayActivationSound(1);
	}

	ExtraCrates = !ExtraCrates;
}


void GameManager::ToggleForceHost()
{
	// FIXME: find a better way, unable to toggle forcehost off
	if (ForceHost)
	{
		printf("force host deactivated\n");
		//DoBeep(OFF);
        //PlayActivationSound(0);
		IW4::CMD::ExecuteSingleCommand(0, 0, "party_hostmigration 1");
		IW4::CMD::ExecuteSingleCommand(0, 0, "party_connectTimeout 1000");
        IW4::CMD::ExecuteSingleCommand(0, 0, "party_connectToOthers 1");
        IW4::CMD::ExecuteSingleCommand(0, 0, "bandwidthtest_enable 1");  
		//IW4::DVAR::SetFromStringByName("party_minplayers", "8");
		IW4::DVAR::SetBoolByName("requireOpenNat", true);
        IW4::DVAR::SetBoolByName("g_kickHostIfIdle", true);
        //IW4::DVAR::SetBoolByName("bandwidthtest_enable", true);
        IW4::CMD::ExecuteSingleCommand(0, 0, "bandwidthtest_enable 1");
        //IW4::CMD::ExecuteSingleCommand(0, 0, "g_kickHostIfIdle 1");
        Memory::ToggleFostHost(true);
        Memory::ToggleDisableSpectatorKick(true);
		Memory::DisableLobbyVac(true);
	}
	else
	{
		printf("force host activated\n");
		//DoBeep(ON);
        //PlayActivationSound(1);
		IW4::CMD::ExecuteSingleCommand(0, 0, "party_hostmigration 0");
		IW4::CMD::ExecuteSingleCommand(0, 0, "party_connectTimeout 1");
		IW4::CMD::ExecuteSingleCommand(0, 0, "party_connectToOthers 0");
		IW4::DVAR::SetBoolByName("requireOpenNat", false);
        IW4::CMD::ExecuteSingleCommand(0, 0, "bandwidthtest_enable 0");
        IW4::CMD::ExecuteSingleCommand(0, 0, "bandwidthtest_ingame_enable 0");
        IW4::CMD::ExecuteSingleCommand(0, 0, "partymigrate_pingtest_timeout 0");
        IW4::CMD::ExecuteSingleCommand(0, 0, "partymigrate_pingtest_retry 0");
        //IW4::CMD::ExecuteSingleCommand(0, 0, "g_kickHostIfIdle 0");
		//IW4::DVAR::SetFromStringByName("party_minplayers", "12");
        IW4::CMD::ExecuteSingleCommand(0, 0, "badhost_endGameIfISuck 0");
        IW4::CMD::ExecuteSingleCommand(0, 0, "badhost_maxDoISuckFrames 0");
        IW4::CMD::ExecuteSingleCommand(0, 0, "badhost_maxHappyPingTime 9999");
        IW4::CMD::ExecuteSingleCommand(0, 0, "badhost_minTotalClientsForHappyTest 9999");
        IW4::CMD::ExecuteSingleCommand(0, 0, "bandwidthtest_enable 0");

        Memory::ToggleFostHost(false);
        Memory::ToggleDisableSpectatorKick(false);
		Memory::DisableLobbyVac(false);

        IW4::DVAR::SetBoolByName("badhost_maxDoISuckFrames", false);
        IW4::DVAR::SetBoolByName("bandwidthtest_ingame_enable", false);
        IW4::DVAR::SetBoolByName("bandwidthtest_enable", false);
        IW4::DVAR::SetBoolByName("partymigrate_pingtest_timeout", false);
        IW4::DVAR::SetBoolByName("g_kickHostIfIdle", false);
        IW4::DVAR::SetBoolByName("bandwidthtest_enable", false);

        IW4::DVAR::SetBoolByName("party_hostmigration", false);
        IW4::DVAR::SetBoolByName("party_connectToOthers", false);


        IW4::CMD::ExecuteSingleCommand(0, 0, "bandwidthtest_enable 0");
        IW4::CMD::ExecuteSingleCommand(0, 0, "g_kickHostIfIdle 0");
	}

	ForceHost = !ForceHost;
}

void GameManager::ToggleForceStart()
{
    // FIXME: find a better way, unable to toggle forcehost off
    if (ForceStart)
    {
        printf("force start deactivated\n");
        //DoBeep(OFF);
        PlayActivationSound(0);
        //IW4::DVAR::SetFromStringByName("party_minplayers", "8");
    }
    else
    {
        printf("force start activated\n");
        //DoBeep(ON);
        PlayActivationSound(1);
        //IW4::DVAR::SetFromStringByName("party_minplayers", "1");
    }
    ForceStart = !ForceStart;
}

void GameManager::extraMatchSettings()
{
    
	//IW4::CMD::ExecuteSingleCommand(0, 0, "scr_airdrop_ac130 100000");
	// extra
    PlayActivationSound(1);
	printf("match settings changed\n");
	IW4::DVAR::SetFromStringByName("scr_war_scorelimit", "15000"); // War score limit
	IW4::DVAR::SetFromStringByName("scr_war_timelimit", "15");     // War time limit
	//IW4::DVAR::SetFromStringByName("g_kickHostIfIdle", "0");     // War time limit
	IW4::DVAR::SetBoolByName("g_kickHostIfIdle", false);
}

void GameManager::TogglePrivateMatch()
{
	if (PrivateMatch)
	{
		DoBeep(OFF);
		IW4::CMD::ExecuteSingleCommand(0, 0, "xblive_privatematch 0");
		IW4::CMD::ExecuteSingleCommand(0, 0, "ui_allow_teamchange 0");
	}
	else
	{
		DoBeep(ON);
		IW4::CMD::ExecuteSingleCommand(0, 0, "xblive_privatematch 1");
		IW4::CMD::ExecuteSingleCommand(0, 0, "ui_allow_teamchange 1");
	}

	PrivateMatch = !PrivateMatch;
}

void GameManager::ToggleMaxPlayers()
{
	if (MaxPlayers)
	{
        PlayActivationSound(0);
		printf("max players set to 12\n");
	}
	else
	{
        PlayActivationSound(1);
		printf("max players set to 18\n");
	}

    MaxPlayers = !MaxPlayers;
}

void GameManager::ToggleAutoHost()
{
    if (AutoHost)
    {
        PlayActivationSound(0);
        //EntityManager::SetGodMode(IW4::MISC::GetHostId(), EntityManager::godmode::MORTAL);
        printf("auto host disabled\n");
    }
    else
    {
        PlayActivationSound(1);
        printf("auto host enabled\n");
    }

    AutoHost = !AutoHost;
}

void GameManager::ToggleFullBright()
{
	GameManager::FullBright = !GameManager::FullBright;
	IW4::MISC::SendClientMessageCenter(IW4::MISC::GetHostId(), "FullBright is " + std::string((GameManager::FullBright ? "^2ON^7!" : "^1OFF^7!")));
}

void GameManager::ToggleFieldOfView()
{
	GameManager::FieldOfView = !GameManager::FieldOfView;
	IW4::MISC::SendClientMessageCenter(IW4::MISC::GetHostId(), "FoV is " + std::string((GameManager::FullBright ? "^280^7!" : "^165^7!")));
}

void GameManager::ToggleInfiniteAmmo()
{
	GameManager::InfiniteAmmo = !GameManager::InfiniteAmmo;
	IW4::MISC::SendClientMessageCenter(IW4::MISC::GetHostId(), "Infinite Ammo is " + std::string((GameManager::InfiniteAmmo ? "^2ON^7!" : "^1OFF^7!")));
}

void GameManager::SetWeaponMode(MenuEvent mevent)
{
	GameManager::WeaponMode = mevent;
}

void GameManager::SendChatMessage(std::string Message)
{
}

void GameManager::SendClientChatMessage(int from, int to, std::string message)
{
	auto froment = EntityManager::GetGEntity(from);
	auto targetent = EntityManager::GetGEntity(to);
	
	IW4::G::Say(froment, targetent, 0, message.c_str());
}

void GameManager::SendKillstreakMessage(int killstreak, int id)
{
	std::string clientname(ClientManager::GetClientById(id)->szName);
	std::string message;
	std::string sound;

	switch (killstreak)
	{
		case 5:
			message = clientname + " is on a ^2killing spree^7!";
			break;
		case 10:
			message = clientname + " is on a ^2killing frenzy^7!";
			break;
		case 15:
			message = clientname + " is on a ^2running riot^7!";
			break;
		case 20:
			message = clientname + " is on a ^2rampage^7!";
			break;
		case 25:
			message = clientname + " is ^2untouchable^7!";
			break;
		case 30:
			message = clientname + " is ^2Invinsible^7!";
			break;
	}

	if (!message.empty())
		IW4::MISC::SendAllClientsMessageCenter(message);
	
	// TODO: implement sounds
	//if (!sound.empty())
	//	IW4::SND::PlayClientSound(0, id, const_cast<char*>(sound.c_str()));
}

void GameManager::SendDeathstreakMessage(int deathstreak, int id)
{
	std::string clientname(ClientManager::GetClientById(id)->szName);
	std::string message;

	switch (deathstreak)
	{
	case 5:
		message = clientname + " is on a ^25 death streak^7!";
		break;
	case 10:
		message = clientname + " is on a ^210 death streak^7!";
		break;
	case 15:
		message = clientname + " is on a ^215 death streak^7!";
		break;
	case 20:
		message = clientname + " is on a ^220 death streak^7!";
		break;
	case 25:
		message = clientname + " is ^225 death streak^7!";
		break;
	case 30:
		message = clientname + " is ^230 death streak^7!";
		break;
	}

	if (!message.empty())
		IW4::MISC::SendAllClientsMessageCenter(message);
}

void GameManager::DoBeep(BeepState state)
{
	switch (state)
	{
	case ON:
		Beep(1000, 50);
		break;
	case OFF:
		Beep(500, 50);
		break;
	default:
		Beep(200, 50);
		break;
	}
}

BOOL PlayResource(int resourceId)
{
    BOOL bRtn;
    LPVOID lpRes;
    HRSRC hResInfo;
    HANDLE hRes;
    //HMODULE hInst = GetModuleHandleW(NULL);

    //HMODULE hModule;
    //GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
    //    (LPCSTR)&myDLLfuncName, &hModule)

    // Find the WAVE resource.

    hResInfo = FindResourceW(GameManager::hInst, MAKEINTRESOURCE(resourceId), L"WAVE");
    if (hResInfo == NULL)
    {
        int lastError = GetLastError();

        return FALSE;
    }

    // Load the WAVE resource. 

    hRes = LoadResource(GameManager::hInst, hResInfo);
    if (hRes == NULL)
        return FALSE;

    // Lock the WAVE resource and play it. 

    lpRes = LockResource(hRes);
    if (lpRes != NULL) {
        bRtn = sndPlaySoundW((LPCWSTR)lpRes, 
                             SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
        UnlockResource(hRes);
    }
    else
    {
        bRtn = 0;
    }

    // Free the WAVE resource and return success or failure. 

    FreeResource(hRes);
    return bRtn;
}

void GameManager::PlayActivationSound(int activated)
{
    switch (activated)
    {
    case 0:
        PlayResource(IDR_WAVE_DEACTIVATED);
        break;
    case 1:
        PlayResource(IDR_WAVE_ACTIVATED);
        break;
    default:
        break;
    }
}

