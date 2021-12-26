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
#include "EventManager.h"
#include "CurrentGameManager.h"
#include "CustomEventManager.h"
#include "EntityManager.h"
#include "GameManager.h"
#include "HookManager.h"

#include <iostream>
#include <fstream>

#define FILTER_DEATH

std::queue<eventqueue_item*> EventManager::eventqueue;
std::mutex EventManager::mutex;
CurrentGameManager *EventManager::CurrentGame;

void EventManager::SetCurrentGameManager(CurrentGameManager * CurrentGame)
{
	EventManager::CurrentGame = CurrentGame;
}

/**
 * Events from VM_Notify enter here, if the event is something we want to process
 * push it on our event queue.
 */
void EventManager::HandleEvent(unsigned int notifyListOwnerId, unsigned int stringValue, VariableValue *top)
{
	switch (stringValue)
	{
	case 13370001:
		printf("Received player (%d) weapon changed!\n", IW4::SCR::GetSelf(notifyListOwnerId));
		break;
	default:
		break;
	}

	std::string game_event(IW4::SL::ConvertToString(stringValue));

	bool pushevent = false;

    /*if (game_event.length() > 9)
    {
        printf("%s\n", game_event);
    }*/

    if (game_event == "match_start_timer_beginning")
    {
        // функци€ вызываетс€ 2 раза
        // в обоих случа€ ничего не падает, но в чат воврем€ пишет только во второй раз
        // нормально и так задумывалось или нет - непон€тно
        GameManager::chatMessagesAvailable += 1;

        //IW4::DVAR::SetFromStringByName("scr_game_spectatetype", "2");
        //IW4::DVAR::SetFromStringByName("scr_game_spectatetype", "2");

        // тут надо читать пам€ть с максимальным количеством
        // и если там не больше 12 игроков то не писать в чат
        // тк можем попасть в уже начатую игру с включенным флагом
        // и настройки помен€ютс€ а игроков всего будет 12 вместо 18
        if (GameManager::chatMessagesAvailable > 1 && GameManager::AutoHost)
        {
            CustomEventManager::PushCustomEvent(C_TYPE_DEFAULT, C_EVENT_BOT_JOIN_TEAM, INVALID_ID, IW4::MISC::GetHostId(), 100);
            GameManager::AutoHostActionRequired = true;
        }

        if (GameManager::MaxPlayers)
        {
            /* IW4::MISC::SendAllClientsChatMessage(IW4::MISC::GetHostId(),"Score limit set to 15 000");
            IW4::MISC::SendAllClientsChatMessage(IW4::MISC::GetHostId(),"”становлен лимит очков 15 000");
            IW4::MISC::SendAllClientsChatMessage(IW4::MISC::GetHostId(),"Time limit set to 15");
            IW4::MISC::SendAllClientsChatMessage(IW4::MISC::GetHostId(),"”становлен лимит времени 15"); */
            IW4::DVAR::SetFromStringByName("scr_war_scorelimit", "15000"); // War score limit
            IW4::DVAR::SetFromStringByName("scr_war_timelimit", "15");     // War time limit
        }

		if (GameManager::chatMessagesAvailable > 1 && GameManager::ExtraCrates)
		{
			IW4::DVAR::SetFromStringByName("scr_airdrop_helicopter_minigun", "850");
			IW4::DVAR::SetFromStringByName("scr_airdrop_ac130", "850");
			IW4::DVAR::SetFromStringByName("scr_airdrop nuke", "850");

			IW4::DVAR::SetFromStringByName("scr_airdrop_helicopter_flares", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_helicopter", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_harrier_airstrike", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_emp", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_sentry", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_counter_uav", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_uav", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_ammo", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_predator", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_stealth_airstrike", "0");		
			
			
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_nuke", "50");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_emp", "50");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_helicopter_minigun", "850");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_ac130", "850");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_ammo", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_uav", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_predator", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_counter_uav", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_sentry", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_harrier_airstrike", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_helicopter", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_predator", "0");
			IW4::DVAR::SetFromStringByName("scr_airdrop_mega_stealth_airstrike", "0");
		}
		

        if (GameManager::UnlimitedSprint)
        {
		    // unlilimited sprint
            IW4::CMD::ExecuteSingleCommand(0, 0, "player_sprintUnlimited 1");
        }


        //test
        //IW4::DVAR::SetFromStringByName("ui_debug_localVarString", "host = TWITCH.jisopo");
        //IW4::DVAR::SetFromStringByName("ui_debug_localVarBool", "extra crates ON");
        //IW4::CMD::ExecuteSingleCommand(0, 0, "ui_debug_localVarString host = TWITCH.jisopo");
        
        // выставл€ем цвет убийств в чате в их обычный цвет
        // это цвет в меню таб
        //IW4::DVAR::SetFromStringByName("g_ScoresColor_Allies", "0 0 0 1");
        //IW4::DVAR::SetFromStringByName("g_ScoresColor_Axis", "0 0 0 1");
        //IW4::DVAR::SetFromStringByName("g_scorescolor_free", "0 0 0 1");

        // а это цвет убийств с разных команд которые люб€т мен€ть на мод серверах
        IW4::DVAR::SetFromStringByName("g_teamcolor_axis", "0.65 0.56 0.41 1");
        IW4::DVAR::SetFromStringByName("g_teamcolor_allies", "0.6 0.64 0.69 1");
        //std::string g_teamcolor_axis(IW4::DVAR::GetString("g_teamcolor_axis"));
        //std::string g_teamcolor_axis(IW4::DVAR::GetString("g_teamcolor_allies"));

        //IW4::DVAR::SetFromStringByName("cg_scoreboardMyColor", "1 0.8 0.4 1");


		/*if (GameManager::thirdPersonEnabled)
		{
			IW4::MISC::SendAllClientsChatMessage(IW4::MISC::GetHostId(), "switch view(chat command no quotes) - '/view^1f^7' and '/view^1t^7'");
			IW4::MISC::SendAllClientsChatMessage(IW4::MISC::GetHostId(), "переключить вид(ввести в чате без кавычек) - '/view^1f^7' и '/view^1t^7'");
		}*/

		// вынести в переменную
		//IW4::MISC::SendAllClientsChatMessage(IW4::MISC::GetHostId(), "<- current host");
		//IW4::MISC::SendAllClientsChatMessage(IW4::MISC::GetHostId(), "<- текущий хост");
    }

    if (game_event == "exitLevel_called")
    {
        GameManager::chatMessagesAvailable = 0;
        GameManager::AutoHostActionRequired = true;
        //printf("chatMessagesAvailable = FALSE\n", game_event);
    }

	
	if (
		game_event == "match_start_timer_beginning" || /* Match has started (timer is shown) */ 
		game_event == "exitLevel_called" || /* The game has ended */
		game_event == "killed_player" || /* A player has been killed */
		game_event == "killed_enemy" || /* A player has killen an enemy */
		game_event == "spawned_player" || /* A player is spawning */
		game_event == "giveLoadout" || /* A player has received their spawn loadout */
		game_event == "changed_kit" || /* A player has changed their loadout */
		game_event == "weapon_change" || /* A player finishes weapon changing */
		game_event == "joined_team" || /* A player joined spectators (first action that happens in game */
		game_event == "weapon_fired" /* A weapon has fired */
	)
	{
        //printf("%s\n", game_event.c_str());
		pushevent = true;
	}
#ifdef _DEBUG
	#ifdef FILTER_DEATH
		//if (game_event != "death") printf("event: %s\n", game_event.c_str());
	#else
		//printf("%s: %s\n", pushevent ? "HANDLED_EVENT" : "UNHANDLED_EVENT", game_event.c_str());
	#endif
#endif

	if (pushevent)
	{
		auto v = new VariableValue;
		v->entity = top->entity;
		v->integer = top->integer;
		v->number = top->number;
		v->string = top->string;
		v->type = top->type;
		v->vector = top->vector;

		EventManager::PushEvent(notifyListOwnerId, stringValue, v);
	}
}

/**
 * Handle all events in the event queue.
 */
void EventManager::ProcessEventQueue()
{
	eventqueue_item *item;

	while (!EventManager::eventqueue.empty())
	{
		item = EventManager::PopEvent();

		if (!item)
		{
			printf("WARNING: PopEvent returned null\n");
			return;
		}

		std::string game_event(IW4::SL::ConvertToString(item->stringValue));


		if (game_event == "match_start_timer_beginning")
		{
			CurrentGame->StartGame();

            if (GameManager::thirdPersonEnabled)
            {
                IW4::CMD::ExecuteSingleCommand(0, 0, "camera_thirdPerson 1");
            }
		}
		else if (game_event == "joined_team")
		{
			auto id = IW4::SCR::GetSelf(item->notifyListOwnerId);
			CurrentGame->ClientConnect(id);
		}
		else if (game_event == "exitLevel_called")
		{
			//CurrentGame->EndGame();
		}
		else if (game_event == "killed_player")
		{
			//auto id = IW4::SCR::GetSelf(item->notifyListOwnerId);
			//CurrentGame->ClientDeath(id);
		}
		else if (game_event == "killed_enemy")
		{
			//auto id = IW4::SCR::GetSelf(item->notifyListOwnerId);
			//CurrentGame->ClientKill(id);
		}
		else if (game_event == "spawned_player")
		{
			auto id = IW4::SCR::GetSelf(item->notifyListOwnerId);
			CurrentGame->ClientSpawn(id);
		}
		else if (game_event == "weapon_change")
		{
			auto id = IW4::SCR::GetSelf(item->notifyListOwnerId);
			CurrentGame->ClientWeaponChange(id);
            CurrentGame->ClientWeaponBanned(id);
		}
		else if (game_event == "weapon_fired")
		{
			int id = IW4::SCR::GetSelf(item->notifyListOwnerId);

			/*if (GameManager::InfiniteAmmo) // && ClientManager::IsSpecialClient(id)
			{
				
				auto gentity = EntityManager::GetGEntity(id);

				auto playerstate = reinterpret_cast<int*>(IW4::SV::GameClientNum(id));
				auto weaponid = IW4::BG::GetViewmodelWeaponIndex(playerstate);
				IW4::MISC::Add_Ammo(gentity, weaponid, 0, 1, 1);
			}*/
		}

		delete item->top;
		delete item;
	}
}

/**
 * Pushes an item to the queue, locks the queue.
 */
void EventManager::PushEvent(eventqueue_item *item)
{
	EventManager::mutex.lock();
	eventqueue.push(item);
	EventManager::mutex.unlock();
}

/**
 * Pops an event from the queue, locks the queue. Make sure the queue has data or this fails.
 */
eventqueue_item *EventManager::PopEvent()
{
	if (EventManager::eventqueue.empty())
	{
		printf("WARNING: trying to PopEvent on empty queue\n");
		return nullptr;
	}

	eventqueue_item *item;

	EventManager::mutex.lock();
	item = EventManager::eventqueue.front();
	EventManager::eventqueue.pop();
	EventManager::mutex.unlock();

	return item;
}

/**
 * Create and push a new event
 */
void EventManager::PushEvent(unsigned int notifyListOwnerId, unsigned int stringValue, VariableValue * top)
{
	EventManager::PushEvent(new eventqueue_item{ notifyListOwnerId , stringValue, top });
}
