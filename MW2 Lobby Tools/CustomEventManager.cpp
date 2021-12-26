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
#include "CustomEventManager.h"
#include "EntityManager.h"
#include "ClientManager.h"
#include "WeaponManager.h"
#include "PerkManager.h"
#include "GameManager.h"
#include "Punishment.h"
#include "HookManager.h"

#include <iostream>
#include <fstream>

std::queue<CustomEvent*> CustomEventManager::customevents;
std::mutex CustomEventManager::mutex;
CurrentGameManager *CustomEventManager::CurrentGame;
float lastLocation[3];

void CustomEventManager::SetCurrentGameManager(CurrentGameManager * CurrentGame)
{
    CustomEventManager::CurrentGame = CurrentGame;
}

/**
* Custom events are handled here
*/
void CustomEventManager::HandleEvent()
{
}

/**
* Handle all events in the custom event queue.
*/
void CustomEventManager::ProcessCustomEventQueue()
{
    CustomEvent *event;

    int size = CustomEventManager::customevents.size();
    for (auto i = 0; i < size; i++)
    {
        event = CustomEventManager::PopCustomEvent();

        if (!event)
        {
            printf("WARNING: PopCustomEvent returned null\n");
            return;
        }

        if (event->GetType() == C_TYPE_DELAYED && event->GetDelay() > 0)
        {
            event->DecreaseDelay();
            CustomEventManager::PushCustomEvent(event);
            continue;
        }

        switch (event->GetEvent())
        {
        case C_EVENT_NULL:
            break;
        case C_EVENT_PUNISH:
            HandlePunishEvent(event);
            break;
        case C_EVENT_PUNISH_UPDATE:
            CurrentGame->ClientPunish(event->GetTargetId());
            break;
        case C_EVENT_SPAWN:
        {
            CurrentGame->ClientWeaponChange(event->GetTargetId());
            CurrentGame->ClientWeaponBanned(event->GetTargetId());
        }
            break;
        case C_EVENT_KILL:
            break;
        case C_EVENT_DEATH:
            break;
        case C_EVENT_JOIN:
            break;
        case C_EVENT_LEAVE:
            break;
        case C_EVENT_BEGIN:
            break;
        case C_EVENT_END:
            break;
        case C_EVENT_MOVE_TO_SPECTATOR:
        {
            IW4::SCR::AddString("spectator");
            IW4::SCR::AddString("team_marinesopfor");
            IW4::SCR::NotifyNum(event->GetTargetId(), 0, *reinterpret_cast<short*>(0x01B6AB60), 2); // 0x1B6AAE0
            CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_BOT_JOIN_TEAM, INVALID_ID, event->GetTargetId(), 100);
        }
        break;
        case C_EVENT_BOT_JOIN_GAME:
            // вызывается хренову тучу раз
            // хоть флаг поставить какой что игра начата и игра закончилась
            IW4::SCR::AddString("autoassign");
            IW4::SCR::AddString("team_marinesopfor");
            IW4::SCR::NotifyNum(event->GetTargetId(), 0, *reinterpret_cast<short*>(0x01B6AB60), 2); // 0x1B6AAE0
            CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_BOT_JOIN_TEAM, INVALID_ID, event->GetTargetId(), 100);
            break;
        case C_EVENT_GIVE_WEAPON:
        {
            WeaponManager::GiveWeapon(event->GetTargetId(), (char*)event->GetExtraData(), 0, 30, 0, true);
            //free(event->GetExtraData());
        }
        break;
        case C_EVENT_BAN_CLIENT_WEAPON:
        {
            //CurrentGameManager::banClientWeapon(event->GetTargetId());
        }
        break;
        case C_EVENT_UNBAN_CLIENT_WEAPON:
        {

        }
        break;
        case C_EVENT_SWITCH_TEAM:
        {
            if (GameManager::AutoHost)
            {
                IW4::SCR::AddString("axis");
                IW4::SCR::AddString("team_marinesopfor");
                IW4::SCR::NotifyNum(event->GetTargetId(), 0, *reinterpret_cast<short*>(0x01B6AB60), 2); // 0x1B6AAE0
                //CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_BOT_JOIN_TEAM, INVALID_ID, event->GetTargetId(), 100);
            }
            else
            {
                IW4::SCR::AddString("allies");
                IW4::SCR::AddString("team_marinesopfor");
                IW4::SCR::NotifyNum(event->GetTargetId(), 0, *reinterpret_cast<short*>(0x01B6AB60), 2); // 0x1B6AAE0
                //CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_BOT_JOIN_TEAM, INVALID_ID, event->GetTargetId(), 100);    
            }
            GameManager::AutoHost = !GameManager::AutoHost;
        }
        break;
        case C_EVENT_BOT_JOIN_TEAM:
        {
            //IW4::SCR::AddString("class3");
            if (GameManager::AutoHostActionRequired)
            {
                IW4::SCR::AddString("class1");
                IW4::SCR::AddString("changeclass");
                IW4::SCR::NotifyNum(event->GetTargetId(), 0, *reinterpret_cast<short*>(0x01B6AB60), 2); //TODO: find SL_ConvertFromString()

                auto hostId = IW4::MISC::GetHostId();

                // меняем ник на ник из файла
                //std::ifstream ifs("D:\\Data\\Soft\\C++\\MW2-Lobby-Tools-master\\Debug\\playername.txt");
                /*std::ifstream ifs("C:\\Users\\Admin0\\Desktop\\MW2\\playername.txt");
                std::string str(std::istreambuf_iterator<char>{ifs}, {});

                printf("AutoHost: name changed to %s\n", str.c_str());
                std::string result = "userinfo \\xuid\\01100001041049c2\\name\\" + str + ";";
                IW4::CBUF::AddText(0, result.c_str());*/

                // вместо годмода переходим в группу наблюдателей
                CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_MOVE_TO_SPECTATOR, INVALID_ID, hostId, 500);

                GameManager::AutoHostActionRequired = false;
            }
            /*if (event->GetTargetId() == hostId)
            {
                char map_name[100] = { 0 };
                ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(CGS_a + 0x14C), &map_name[0], 100, nullptr);
                std::string current_map(map_name);
                // printf("current map is %s\n", map_name);
                if (current_map.find("afghan") != std::string::npos)
                {
                    // TODO: добавить время
                    printf("current map is afghan\n");
                    lastLocation[0] = 45835.406250;
                    lastLocation[1] = -21874.089844;
                    lastLocation[2] = -31231.882813;
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                    //EntityManager::GetGEntity(hostId)->playerstate_s->SetOrigin(&lastLocation);
                }

                if (current_map.find("derail") != std::string::npos)
                {
                    printf("current map is derail\n");
                    lastLocation[0] = 384.948212;
                    lastLocation[1] = -3751.786133;
                    lastLocation[2] = 117.7091412;
                    //EntityManager::GetGEntity(hostId)->playerstate_s->SetOrigin(&location);
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("highrise") != std::string::npos)
                {
                    printf("current map is highrise\n");
                    lastLocation[0] = 1114.101074;
                    lastLocation[1] = 6906.238281;
                    lastLocation[2] = 2984.125732;
                    //EntityManager::GetGEntity(hostId)->playerstate_s->SetOrigin(&location);
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("boneyard") != std::string::npos)
                {
                    printf("current map is boneyard\n");
                    lastLocation[0] = 1431.137939;
                    lastLocation[1] = 1245.967773;
                    lastLocation[2] = 60.139664;
                    //EntityManager::GetGEntity(hostId)->playerstate_s->SetOrigin(&location);
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("quarry") != std::string::npos)
                {
                    printf("current map is quarry\n");
                    lastLocation[0] = 848.291016;
                    lastLocation[1] = 1280.119995;
                    lastLocation[2] = 230.811844;
                    //EntityManager::GetGEntity(hostId)->playerstate_s->SetOrigin(&location);
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("underpass") != std::string::npos)
                {
                    printf("current map is underpass\n");
                    lastLocation[0] = 2111.300049;
                    lastLocation[1] = 1240.853271;
                    lastLocation[2] = 448.125000;
                    //EntityManager::GetGEntity(hostId)->playerstate_s->SetOrigin(&location);
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("rust") != std::string::npos)
                {
                    printf("current map is rust\n");
                    //float location = 2028.762573f;
                    lastLocation[0] = 2058.0000;
                    lastLocation[1] = 1659.036743;
                    lastLocation[2] = -102.889816;
                    //EntityManager::GetGEntity(hostId)->playerstate_s->SetOrigin(&location);
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("rundown") != std::string::npos)
                {
                    printf("current map is rundown\n");
                    //lastLocation[0] = -597.816406;
                    //lastLocation[1] = -1226.27466;
                    //lastLocation[2] = 25.125004;
                    lastLocation[0] = -761.332214;
                    lastLocation[1] = -4023.426025;
                    lastLocation[2] = 124.600151;
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("brecourt") != std::string::npos)
                {
                    printf("current map is wasteland\n");
                    //lastLocation[0] = 3351.109619;
                    //lastLocation[1] = -3379.174805;
                    //lastLocation[2] = 45.624771;
                    lastLocation[0] = 16.238281;
                    lastLocation[1] = 583.886230;
                    lastLocation[2] = -3519.878662;
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("nightshift") != std::string::npos)
                {
                    printf("current map is skidrow\n");
                    lastLocation[0] = 10.953278;
                    lastLocation[1] = -2372.059326;
                    lastLocation[2] = 200.125000;
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("estate") != std::string::npos)
                {
                    printf("current map is estate\n");
                    lastLocation[0] = -4656.215820;
                    lastLocation[1] = 4392.960938;
                    lastLocation[2] = -275.875000;
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("checkpoint") != std::string::npos)
                {
                    printf("current map is karachi\n");
                    lastLocation[0] = -2261.109863;
                    lastLocation[1] = -2301.930908;
                    lastLocation[2] = -0.552544;
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("terminal") != std::string::npos)
                {
                    printf("current map is terminal\n");
                    lastLocation[0] = 3737.885254;
                    lastLocation[1] = 2004.098511;
                    lastLocation[2] = 193.122925;
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("subbase") != std::string::npos)
                {
                    printf("current map is subbase\n");
                    lastLocation[0] = 2384.509277;
                    lastLocation[1] = 1439.315674;
                    lastLocation[2] = 200.123749;
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("invasion") != std::string::npos)
                {
                    printf("current map is invasion\n");
                    lastLocation[0] = -4767.858398;
                    lastLocation[1] = -3249.326172;
                    lastLocation[2] = 348.949249;
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }

                if (current_map.find("favela") != std::string::npos)
                {
                    printf("current map is favela\n");
                    lastLocation[0] = -245.498459;
                    lastLocation[1] = 2795.079102;
                    lastLocation[2] = 572.125000;
                    CustomEventManager::PushCustomEvent(C_TYPE_DELAYED, C_EVENT_TELEPORT_HOST, INVALID_ID, INVALID_ID, 500);
                }*/

                // не забывать убрать
                // IW4::SV::AddTestClient();

            //}
        }
        break;
        case C_EVENT_TELEPORT:
        {
            auto ent1 = EntityManager::GetGEntity(event->GetSourceId());
            auto ent2 = EntityManager::GetGEntity(event->GetTargetId());

            if (ent1 == nullptr || ent2 == nullptr || ent1->playerstate_s == nullptr || ent2->playerstate_s == nullptr) break;

            ent2->playerstate_s->SetOrigin(ent1->Origin);
        }
        break;
        case C_EVENT_TELEPORT_HOST:
        {
            //auto hostId = IW4::MISC::GetHostId();

            //EntityManager::GetGEntity(hostId)->playerstate_s->SetOrigin(&lastLocation[0]);

            //PerkManager::SetPerk(hostId, "specialty_coldblooded");
            //PerkManager::SetPerk(hostId, "specialty_coldblooded");

            //EntityManager::SetGodMode(hostId, EntityManager::godmode::GOD);

            /*if (GameManager::thirdPersonEnabled)
            {
                IW4::MISC::SendAllClientsChatMessage(hostId, "switch view(chat command) - '/view^1f^7' and '/view^1t^7'");
                IW4::MISC::SendAllClientsChatMessage(hostId, "переключить вид(ввести в чате) - '/view^1f^7' и '/view^1t^7'");
            }*/
        }
        break;
        default:
            printf("WARNING: Invalid CustomEvent received\n");
        }

        delete event;
    }
}


/**
* Pushes an item to the queue, locks the queue.
*/
void CustomEventManager::PushCustomEvent(CustomEvent *event)
{
    CustomEventManager::mutex.lock();
    customevents.push(event);
    CustomEventManager::mutex.unlock();
}

void CustomEventManager::PushCustomEvent(C_TYPE type, C_EVENT event, unsigned int source_id, unsigned int target_id, unsigned int delay)
{
    CustomEventManager::PushCustomEvent(new CustomEvent(type, event, source_id, target_id, delay, nullptr));
}

void CustomEventManager::PushCustomEvent(C_TYPE type, C_EVENT event, unsigned source_id, unsigned target_id, unsigned delay, void* extradata)
{
    CustomEventManager::PushCustomEvent(new CustomEvent(type, event, source_id, target_id, delay, extradata));
}

/**
* Pops an event from the queue, locks the queue. Make sure the queue has data or this fails.
*/
CustomEvent *CustomEventManager::PopCustomEvent()
{
    if (CustomEventManager::customevents.empty())
    {
        printf("WARNING: trying to PopCustomEvent on empty queue\n");
        return nullptr;
    }

    CustomEvent *event;

    CustomEventManager::mutex.lock();
    event = CustomEventManager::customevents.front();
    CustomEventManager::customevents.pop();
    CustomEventManager::mutex.unlock();

    return event;
}

void CustomEventManager::AdminMessageAll(unsigned id, std::string message)
{
    IW4::MISC::SendAllClientsMessageBold("^2Admin^7: " + std::string(ClientManager::GetClientById(id)->szName) + "^7 " + message);
}

void CustomEventManager::HandlePunishEvent(CustomEvent *event)
{
    auto targetid = event->GetTargetId();
    auto ent = EntityManager::GetGEntity(targetid);

    if (!CurrentGame->ClientAlive(targetid)) return;

    auto flag = static_cast<punishment_wrapper*>(event->GetExtraData())->flag;

    switch (flag)
    {
    case PUNISH_EMPTY:
        break;
    case PUNISH_ROLLERCOASTER:
        // TODO: implement
        break;
    case PUNISH_KILL:
        IW4::UNKN::player_die(ent, ent, ent, 100, 0, 0, nullptr, 0, 0);
        CustomEventManager::AdminMessageAll(targetid, "was ^1killed^7 by the server!");
        break;
    case PUNISH_PLANT:
        // TODO: implement
        break;
    case PUNISH_SLOW:
        // TODO: implement
        break;
    case PUNISH_INVERSE:
        // TODO: implement
        break;
    case PUNISH_STRIP_WEAPON:
        WeaponManager::StripWeapons(targetid);
        CustomEventManager::AdminMessageAll(targetid, "had his weapons ^1stripped^7!");
        break;
    case PUNISH_STRIP_PERK:
        // TODO: implement
        break;
    case PUNISH_STRIP_ALL:
        // TODO: implement
        break;
    case PUNISH_BLIND:
        // TODO: implement
        break;
    case PUNISH_AMMO_MAG:
        // TODO: implement
        break;
    case PUNISH_AMMO_ALL:
        // TODO: implement
        break;
    case PUNISH_CRAP:
        // TODO: implement
        break;
    case PUNISH_MUTE:
        // TODO: implement
        break;
    case PUNISH_ALL:
        // TODO: implement
        break;
    default:
        // TODO: implement
        break;
    }
}
