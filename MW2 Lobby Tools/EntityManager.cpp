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
#include "EntityManager.h"

std::string EntityManager::modelclasses[] = {
	"info_notnull",
	"trigger_radius",
	"script_model",
	"script_origin",
	"script_vehicle_collmap",
	"script_brushmodel"
};

gentity_t * EntityManager::GetGEntity(int id)
{
	return reinterpret_cast<gentity_t*>(GENTITY_a + (id * GENTITY_s));
}

void * EntityManager::GetPlayerstate(int id)
{
    return reinterpret_cast<void*>(PlayerState_a + (id * PlayerState_b));
}

gentity_t * EntityManager::PlayerStateToGEntity(playerstate_t *ps)
{
    for (auto i = 0; i < MAX_CLIENTS; i++)
    {
        if (ps == GetGEntity(i)->playerstate_s)
            return GetGEntity(i);
    }

    return nullptr;
}

gentity_t* EntityManager::SpawnEntity(modelclass c, std::string model, vec3_t origin, vec3_t angle)
{
    auto ent = IW4::G::Spawn();

    IW4::G::SetOrigin(ent, origin);
    IW4::G::SetAngle(ent, angle);
    ent->modelclass = IW4::SL::GetString(modelclasses[c].c_str(), 0);

    if (!IW4::G::CallSpawnEntity(ent))
    {
        Logger::LogWarning("Unable to spawn additional entity");
        IW4::G::FreeEntity(ent);
        return nullptr;
    }

    printf("Spawning %s \"%s\" #%d @ (%g, %g, %g)\n", modelclasses[c].c_str(), model.c_str(), ent->EntityNumber, origin[0], origin[1], origin[2]);

    IW4::G::SetModel(ent, model.c_str());
    IW4::G::EnterWorld(ent, 0);

    //for (auto i = 0; i < 2000; i++)
    //{
    //	auto ent = EntityManager::GetGEntity(i);
    //	if (ent->modelclass != 0)
    //	{
    //		auto classname = IW4::SL::ConvertToString(ent->modelclass);
    //		//if (strcmp(classname, "script_brushmodel") == 0)
    //		{
    //			auto modelname = IW4::SL::ConvertToString(IW4::G::ModelName(*(WORD*)((char*)ent + 0x168)));
    //			printf("script_brushmodel %d: %s\n", ent->EntityNumber, modelname);
    //		}
    //	}
    //}

    return ent;
}

void EntityManager::SetGodMode(int id, godmode mode)
{
    // TODO: fix
    *reinterpret_cast<int*>((char*)GetGEntity(id) + 0x184) = mode;
}

void EntityManager::DisableWeapon(int id)
{
    *reinterpret_cast<int*>((char*)GetPlayerstate(id) + 0x2BC) = 0x80;
}

// 0 или 1
void EntityManager::SetNoClipMode(int id, uint16_t value)
{
    // TODO: fix
    *reinterpret_cast<int*>((char*)GetPlayerstate(id) + 0x3394) = value;
}

void fix_str(char* str)
{
    size_t stringlen = strlen(str);

    for (size_t i = 0; i < stringlen; i++)
    {
        if (str[i] == '0')
        {
            str[i] = ' ';
        }
        else
        {
            break;
        }
    }

    for (size_t i = stringlen - 1; i > 0; i--)
    {
        if (str[i] == '0')
        {
            str[i] = '\0';
        }
        else
        {
            break;
        }
    }
}

void setPlayerData(int clientIndex, const char* playerData, int value) {
    IW4::SCR::AddInt(value);
    IW4::SCR::AddString(playerData);
    IW4::MISC::SetNumParam(2);
    // iw4.GScr_SetPlayerData(clientIndex);
    IW4::SCR::GScr_SetPlayerData(clientIndex);
    IW4::SCR::ClearOutParams();
}

void setPlayerData(int clientIndex, const char* playerData, const char* input1, int value) {
    IW4::SCR::AddInt(value);
    IW4::SCR::AddString(input1);
    IW4::SCR::AddString(playerData);
    //*(iw4.ScrNumParams) = 3;
    IW4::MISC::SetNumParam(3);
    IW4::SCR::GScr_SetPlayerData(clientIndex);
    // iw4.Scr_ClearOutParams();
    IW4::SCR::ClearOutParams();
}
