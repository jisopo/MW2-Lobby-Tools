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
#include "HookManager.h"
#include <future>

#include <iostream>
#include <fstream>

#include "ConfigManager.h"

#include "GameManager.h"
#include "CurrentGameManager.h"
#include "EventManager.h"
#include "CustomEventManager.h"

#include "PatchDvars.h"
#include "PatchJumps.h"
#include "MemoryManager.h"
#include "DrawManager.h"
#include "detours.h"
#include "ChatManager.h"
#include "PunishmentManager.h"

//#include <string>

#define F_PROLOG __asm \
{ \
	__asm push ebp \
	__asm mov ebp, esp \
	__asm sub esp, __LOCAL_SIZE \
	__asm pushad \
}

#define F_EPILOG(X) __asm \
{ \
	__asm popad \
	__asm mov esp, ebp \
	__asm pop ebp \
	__asm jmp [X] \
}

std::vector<std::string> mapList
{
    "mp_afghan",
    "mp_boneyard",
    "mp_brecourt",
    "mp_checkpoint",
    "mp_derail",
    "mp_estate",
    "mp_favela",
    "mp_highrise",
    "mp_nightshift",
    //"mp_invasion",
    "mp_quarry",
    "mp_rundown",
    "mp_rust",
    "mp_subbase",
    "mp_terminal",
    "mp_underpass",
    "mp_favela",
    "mp_crash",
    "mp_compact",
    "mp_storm",
    "mp_strike",
    "mp_trailerpark",
    "mp_vacant",
    "mp_fuel2",
    "mp_abandon"
};

// Original functions
typedef LRESULT(WINAPI* DispatchMessage_t)(MSG*);
DispatchMessage_t oDispatchMessage;

typedef void(*Refresh_t)(int localClientNum);
Refresh_t oRefresh;

typedef void(*Notify_fp)(unsigned int notifyListOwnerId, unsigned int stringValue, void *VariableValue_top);
Notify_fp oNotify;

typedef int(*Info_ValueForKey_0)(int a1, int a2, int a3, int a4);
Info_ValueForKey_0 oInfoValueForKey;

typedef void(*Resume_fp)(unsigned int timeId);
Resume_fp oResume;

typedef void(*Say_fp)(gentity_t *from, gentity_t *to, int mode, const char *chatText);
Say_fp oSay;

typedef void(__cdecl* AddText_t)(int localClientNum, const char *text);
AddText_t oAddText;

typedef void(*DropClient_fp)(int clientNum, const char *reason);
DropClient_fp oDropClient;

typedef void(*DropClient2_fp)(void *client_s, const char *reason, bool tellThem);
DropClient2_fp oDropClient2;

typedef void(*ComError_fp)(int errorParamT, const char* msg, bool tellThem);
ComError_fp oComError;

typedef DWORD(*GameStyles_fp)(int ScreenPlacement_scrPlace,
                            int64_t x,
                            int y,
                            int width,
                            int height,
                            int t1,
                            int s2,
                            int angle,
                            int arg0,
                            int arg4,
                            int arg8,
                            int argC,
                            int arg10,
                            int arg14,
                            int arg18,
                            int arg1C,
                            int arg20,
                            int arg24,
                            int arg28,
                            int color,
                            char** Material_material);
GameStyles_fp oGameStyles;

typedef void(*RceFixFunc_fp)(const char* localClientNum, const char* netadr_t_from, const char* msg_t_msg, const char* time, const char* arg5, const char* arg6);
RceFixFunc_fp oRceFix;

typedef dvar_t *(*SetFromStringByNameFromSource_fp)(const char *dvarName, const char *string, DvarSetSource source);
SetFromStringByNameFromSource_fp oSetFromStringByNameFromSource;

typedef void(*setClientDvar_fp)(scr_entref_t entref);
setClientDvar_fp osetClientDvar;

typedef void(*SetPlayerData_fp)();
SetPlayerData_fp oSetPlayerData;

// Managers
MemoryManager* mMemoryManager;
DrawManager* mDrawManager;
CurrentGameManager *mCurrentGameManager;

void HookRefresh()
{
    oRefresh = reinterpret_cast<Refresh_t>(DetourFunction(reinterpret_cast<PBYTE>(0x0059DFE0), reinterpret_cast<PBYTE>(mRefresh)));
    //printf("UI_Refresh hooked\n");
}

void HookDispatchMessageA()
{
    unsigned long DispatchMessageAddress = 0;

    while (!((DispatchMessageAddress = reinterpret_cast<unsigned long>(GetProcAddress(GetModuleHandleA("User32.dll"), "DispatchMessageA")))))
        Sleep(250);

    oDispatchMessage = reinterpret_cast<DispatchMessage_t>(DetourFunction(reinterpret_cast<PBYTE>(DispatchMessageAddress), reinterpret_cast<PBYTE>(mDispatchMessage)));
    //printf("DispatchMessageA hooked\n");
}

void HookNotify()
{
    oNotify = reinterpret_cast<Notify_fp>(DetourFunction(reinterpret_cast<PBYTE>(0x0057FBB0), reinterpret_cast<PBYTE>(mNotify)));
    //printf("VM_Notify hooked\n");
}

void HookPlayerData()
{
    
}

void HookResume()
{
    oResume = reinterpret_cast<Resume_fp>(DetourFunction(reinterpret_cast<PBYTE>(0x00584122), reinterpret_cast<PBYTE>(mResume)));
    //printf("VM_Resume hooked\n");
}

void HookSay()
{
    oSay = reinterpret_cast<Say_fp>(DetourFunction(reinterpret_cast<PBYTE>(0x005260E0), reinterpret_cast<PBYTE>(mSay)));
}

// изменение цвета заднего фона из assumingagate
void HookGameStyles()
{
    oGameStyles = reinterpret_cast<GameStyles_fp>(DetourFunction(reinterpret_cast<PBYTE>(0x004BC1B0), reinterpret_cast<PBYTE>(mGameStyles)));
}

// логичнее перенести в секцию с оффсетами игры наверное
DWORD Rce1FixHookHookLoc = 0x005C97AF;
DWORD Rce1FixHookHookRet = 0x005C97B4;
StompHook Rce1FixHook;

void* joinParty = "join";

// чиним rce #1 joinParty
void __declspec(naked) Rce1FixHookStub()
{
    __asm
    {
        push ebx
        // TODO: понять как сравнить строку без лишнего регистра
        push ecx
        mov ecx, joinParty
        mov ecx, [ecx]
        mov ebx, [edi + 8]
        cmp [ebx + 5], ecx // 'join'
        jne normal_exit
        // overflow check
        cmp eax, 78h
        jg rce
    normal_exit:
        pop ebx
        pop ecx
        mov ebx, eax
        cmp ebx, -01
        jmp Rce1FixHookHookRet
    rce:
        pop ebx
        pop ecx
        // jmp WsaGetLastError
        mov ebx, 0
        // next check in original code checked with jne
        // so we go to WsaGetLastError and ebx register wont used later
        // rce fixed
        cmp ebx, 0
        jmp Rce1FixHookHookRet
    }
}

void RceFix()
{
    DWORD origProtect;
    // ставим аттрибуты памяти на rwx вместо rx иначе игра упадет при попытке писать в память где нету прав
    VirtualProtect(reinterpret_cast<LPVOID>(Rce1FixHookHookLoc), 5, PAGE_EXECUTE_READWRITE, &origProtect); // 0x51DDA3
    // jmp на нашу память с фиксом
    Rce1FixHook.initialize(Rce1FixHookHookLoc, Rce1FixHookStub);
    Rce1FixHook.installHook();
    printf("rce#1 joinParty fix applied\n");
}

DWORD LobbyMaxPlayersHookLoc = 0x00622459;
DWORD LobbyMaxPlayersHookRet = 0x0062245F;
DWORD LobbyMaxPlayersGameMemoryAddress = 0x0664157C;
StompHook LobbyMaxPlayersHook;

int LobbyMaxPlayersCountGlobal = 12;

void __declspec(naked) LobbyMaxPlayersHookStub()
{
    __asm
    {
        push ecx
        push edx
        lea edx, [ebx + 0000021Ch]
        mov ecx, LobbyMaxPlayersGameMemoryAddress
        cmp ecx, edx
        jne normal_exit
        mov ecx, LobbyMaxPlayersCountGlobal
        mov[ebx + 0000021Ch], ecx
        pop ecx
        pop edx
        jmp LobbyMaxPlayersHookRet
        normal_exit :
            pop ecx
            pop edx
            mov[ebx + 0000021Ch], edx
            jmp LobbyMaxPlayersHookRet
    }
}


// где хранится статическое значение я так и не нашел
// вернее сказать нашел, но оно перезаписывается
// это своего рода обход, пишем в значение максимальных игроков сразу
// после того как это сделает оригинальная функция
void MaxPlayersHook()
{
    DWORD origProtect;
    // ставим аттрибуты памяти на rwx вместо rx иначе игра упадет при попытке писать в память где нету прав
    VirtualProtect(reinterpret_cast<LPVOID>(LobbyMaxPlayersHookLoc), 6, PAGE_EXECUTE_READWRITE, &origProtect);
    LobbyMaxPlayersHook.initialize(LobbyMaxPlayersHookLoc, LobbyMaxPlayersHookStub, 6);
    LobbyMaxPlayersHook.installHook();
}

DWORD MinPartyPlayersHookLoc = 0x004D9EAF;
DWORD MinPartyPlayersHookRet = 0x004D9EB5;
StompHook MinPartyPlayersHook;

int MinPartyPlayersCount = 4;

void __declspec(naked) MinPartyPlayersHookStub()
{
    __asm
    {
        push ebx
        mov ebx, MinPartyPlayersCount
        mov edx, 0x010FB650 // iw4mp.exe + CFB650
        mov edx, [edx]
        // в edx+0C лежит минимальное количество игроков для старта игры - 8
        mov[edx + 0Ch], ebx
        pop ebx
        jmp MinPartyPlayersHookRet
    }
}

// меняем минимальное количество игроков для начала игры
// значение по умолчанию 4
void MinPartyPlayers()
{
    DWORD origProtect;
    // ставим аттрибуты памяти на rwx вместо rx иначе игра упадет при попытке писать в память где нету прав
    VirtualProtect(reinterpret_cast<LPVOID>(MinPartyPlayersHookLoc), 6, PAGE_EXECUTE_READWRITE, &origProtect);
    MinPartyPlayersHook.initialize(MinPartyPlayersHookLoc, MinPartyPlayersHookStub, 6);
    MinPartyPlayersHook.installHook();

}

void HookSetFromStringByNameFromSource()
{
    oSetFromStringByNameFromSource = reinterpret_cast<SetFromStringByNameFromSource_fp>(DetourFunction(reinterpret_cast<PBYTE>(0x5BE7A0), reinterpret_cast<PBYTE>(mSetFromStringByNameFromSource)));
    //printf("Dvar_SetFromStringByNameFromSource hooked\n");
}

void HooksetClientDvar()
{
    osetClientDvar = reinterpret_cast<setClientDvar_fp>(DetourFunction(reinterpret_cast<PBYTE>(0x005221C0), reinterpret_cast<PBYTE>(msetClientDvar)));
    //printf("PlayerCmd_setClientDvar hooked\n");
}

void HookSetPlayerData()
{
    oSetPlayerData = reinterpret_cast<SetPlayerData_fp>(DetourFunction(reinterpret_cast<PBYTE>(0x005A45E0), reinterpret_cast<PBYTE>(mSetPlayerData)));
    printf("SetPlayerData hooked\n");
}

std::vector<std::string> messages;

LRESULT WINAPI mDispatchMessage(MSG* lpmsg)
{
    auto wParam = lpmsg->wParam;

    auto handled = false;

    switch (lpmsg->message)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_F1:
            // TODO: auto open on startup
            // GameManager::OpenIW4Console();
            GameManager::ToggleExtraCrates();
            break;
        case VK_F2:
        {
            GameManager::PlayActivationSound(1);
            // host ended game
            GameManager::EndGame();
            // IW4::SV::AddTestClient();
            // IW4::SV::ClientSpawn(EntityManager::GetGEntity(IW4::MISC::GetHostId()), 0, 0);
            // IW4::SV::ClientEndFrame(EntityManager::GetGEntity(IW4::MISC::GetHostId()));
        }
        //GameManager::OpenGameSetup();
        break;
        case VK_F3:
        {
            GameManager::PlayActivationSound(1);
            GameManager::GameForceStart();
        }
            break;
        case VK_F4:
        {
        }
        break;
        case VK_F5:
        {
            if (!GameManager::UnlimitedSprint)
            {
                GameManager::PlayActivationSound(1);
                printf("unlimited sprint activated\n");
            }
            else
            {
                GameManager::PlayActivationSound(0);
                printf("unlimited sprint deactivated\n");
            }

            GameManager::UnlimitedSprint = !GameManager::UnlimitedSprint;

            GameManager::PlayActivationSound(0);
        }
            break;
        case VK_F6:
        {
            GameManager::PlayActivationSound(1);
            GameManager::ToggleMaxPlayers();
        }
        break;
        case VK_F7:
        {
            GameManager::ToggleAutoHost();
        }
        break;
        case VK_F8:
        {
            
        }
        break;
        case VK_F9:
        {
            GameManager::thirdPersonEnabled = !GameManager::thirdPersonEnabled;
            GameManager::PlayActivationSound(1);
        }
        break;
        case VK_F10:
        {
        }
        break;
        case VK_NUMPAD0:
        {
            GameManager::PlayActivationSound(1);
            IW4::DVAR::SetStringByName("ui_mapname", "mp_highrise");
            IW4::UNKN::UIMapSet("mp_highrise");
        }
        break;
        case VK_NUMPAD1:
        {
            GameManager::PlayActivationSound(1);
            IW4::DVAR::SetStringByName("ui_mapname", "mp_afghan");
            IW4::UNKN::UIMapSet("mp_afghan");
        }
        break;
        case VK_NUMPAD2:
        {
            GameManager::PlayActivationSound(1);
            IW4::DVAR::SetStringByName("ui_mapname", "mp_derail");
            IW4::UNKN::UIMapSet("mp_derail");
        }
        break;
        case VK_NUMPAD3:
        {
            GameManager::PlayActivationSound(1);
            IW4::DVAR::SetStringByName("ui_mapname", "mp_quarry");
            IW4::UNKN::UIMapSet("mp_quarry");
        }
        break;
        case VK_NUMPAD4:
        {
            GameManager::PlayActivationSound(1);
            IW4::DVAR::SetStringByName("ui_mapname", "mp_brecourt");
            IW4::UNKN::UIMapSet("mp_brecourt");
        }
        break;
        case VK_NUMPAD5:
        {
            GameManager::PlayActivationSound(1);
            IW4::DVAR::SetStringByName("ui_mapname", "mp_nightshift");
            IW4::UNKN::UIMapSet("mp_nightshift");
        }
        break;
        case VK_NUMPAD6:
        {
            GameManager::PlayActivationSound(1);
            IW4::DVAR::SetStringByName("ui_mapname", "mp_rundown");
            IW4::UNKN::UIMapSet("mp_rundown");
        }
        break;
        case VK_NUMPAD7:
        {
            GameManager::PlayActivationSound(1);
            IW4::DVAR::SetStringByName("ui_mapname", "mp_subbase");
            IW4::UNKN::UIMapSet("mp_subbase");
        }
        break;
        case VK_NUMPAD8:
        {
            GameManager::PlayActivationSound(1);
            IW4::DVAR::SetStringByName("ui_mapname", "mp_favela");
            IW4::UNKN::UIMapSet("mp_favela");
        }
        break;
        case VK_NUMPAD9:
        {
            GameManager::PlayActivationSound(1);
            IW4::DVAR::SetStringByName("ui_mapname", "mp_terminal");
            IW4::UNKN::UIMapSet("mp_terminal");
            break;
        }
        case VK_ADD:
        {
            GameManager::PlayActivationSound(1);
            IW4::DVAR::SetStringByName("ui_mapname", "mp_rust");
            IW4::UNKN::UIMapSet("mp_rust");
            break;
        }
        case VK_SUBTRACT:
        {
            IW4::DVAR::SetStringByName("ui_mapname", "mp_boneyard"); // Scrapyard
            IW4::UNKN::UIMapSet("mp_boneyard");
            break;
        }
        case VK_MULTIPLY: // *
        {
            IW4::DVAR::SetStringByName("ui_mapname", "mp_estate");
            IW4::UNKN::UIMapSet("mp_estate");
            break;
        }
        case VK_DELETE:
        {
            IW4::DVAR::SetStringByName("ui_mapname", "mp_underpass");
            IW4::UNKN::UIMapSet("mp_underpass");
            break;
        }
        case VK_DIVIDE: // /
        {
            IW4::DVAR::SetStringByName("ui_mapname", "mp_invasion");
            IW4::UNKN::UIMapSet("mp_invasion");
            break;
        }

        /*
       case VK_F9:
           mDrawManager->MenuToggle();
           break;

       case VK_UP:
           mDrawManager->CursorUp();
           handled = true;
           break;
       case VK_DOWN:
           mDrawManager->CursorDown();
           handled = true;
           break;
       case VK_LEFT:
           mDrawManager->CursorBack();
           handled = true;
           break;
       case VK_RIGHT:
           mDrawManager->CursorNext();
           handled = true;
           break;
       case VK_RETURN:
           mDrawManager->CursorSelect();
           handled = true;
           break;*/
        }
    case WM_CHAR:
        //mDrawManager->KeyEvent(wParam);
        break;
    }

    if (!handled || !mDrawManager->IsOpen())
        return oDispatchMessage(lpmsg);
    return false;
}

void mRefresh(int localClientNum)
{
    oRefresh(localClientNum);

    mCurrentGameManager->HandleGamestate();

    /* === DO UI HERE === */

    mDrawManager->DrawMenu();

    /* ===   END UI   === */
}

int Info_ValueForKey(int a1, int a2, int a3, int a4)
{
    int result = oInfoValueForKey(a1, a2, a3, a4);

    if (GameManager::MaxPlayers)
    {
        MemoryManager::WriteValueToAddress(MAXPLAYERS_a, 18);
    }
    else
    {
        MemoryManager::WriteValueToAddress(MAXPLAYERS_a, 12);
    }

    return result;
}

void mNotify(unsigned int notifyListOwnerId, unsigned int stringValue, VariableValue *top)
{
    EventManager::HandleEvent(notifyListOwnerId, stringValue, top);

    return oNotify(notifyListOwnerId, stringValue, top);
}

__declspec(naked) void __cdecl mResume(unsigned int timeId)
{
    F_PROLOG;

    mCurrentGameManager->GameTick();
    CustomEventManager::ProcessCustomEventQueue();
    EventManager::ProcessEventQueue();

    F_EPILOG(oResume);
}

void mRceFix(const char* localClientNum, const char* netadr_t_from, const char* msg_t_msg, const char* time, const char* arg5, const char* arg6)
{
	return oRceFix(localClientNum, netadr_t_from, msg_t_msg, time, arg5, arg6);
}

float* RgbColorToIntColor(float a2, float a3, float a4, float a5)
{
    float* a1;
    *a1 = a2 / 255.0;
    a1[1] = a3 / 255.0;
    a1[2] = a4 / 255.0;
    a1[3] = a5 / 255.0;
    return a1;
}

DWORD mGameStyles(int ScreenPlacement_scrPlace,
                  int64_t x,
                  int y,
                  int width,
                  int height,
                  int t1,
                  int s2,
                  int angle,
                  int arg0,
                  int arg4,
                  int arg8,
                  int argC,
                  int arg10,
                  int arg14,
                  int arg18,
                  int arg1C,
                  int arg20,
                  int arg24,
                  int arg28,
                  int color,
                  char** Material_material
)
{
    char* str = *Material_material;
    int a11;
    a11 = color;

    if (!strcmp(str, "mw2_main_mp_image")
        || !strcmp(str, "mw2_main_sp_image")
        || !strcmp(str, "mw2_main_co_image")
        || !strcmp(str, "mw2_main_mp_image")
        || !strcmp(str, "mockup_bg_glow")
        || !strcmp(str, "logo_cod2")
        || !strcmp(str, "mw2_main_cloud_overlay")
        || !strcmp(str, "mw2_main_background")
        || !strcmp(str, "menu_background")
        || !strcmp(str, "menu_cloud_overlay")
        || !strcmp(str, "popmenu_bg")
        || !strcmp(str, "mw2_popup_bg_fogstencil")
        || !strcmp(str, "mw2_popup_bg_fogscroll")
        || !strcmp(str, "mockup_popup_bg_stencilfill")
        || !strcmp(str, "drop_shadow_tl")
        || !strcmp(str, "drop_shadow_t")
        || !strcmp(str, "drop_shadow_tr")
        || !strcmp(str, "drop_shadow_r")
        || !strcmp(str, "drop_shadow_br")
        || !strcmp(str, "drop_shadow_b")
        || !strcmp(str, "drop_shadow_bl")
        || !strcmp(str, "drop_shadow_l")
        || !strcmp(str, "animbg_blur_fogscroll")
        || !strcmp(str, "animbg_blur_back")
        || !strcmp(str, "animbg_blur_front"))
    {
        // 1132396544
        // 1123024896
        a11 = (int)RgbColorToIntColor((float)1.813667107E17, (float)0, (float)0, (int)1132396544);
    }
    
    return oGameStyles(ScreenPlacement_scrPlace,
                 x,
                 y,
                 width,
                 height,
                 t1,
                 s2,
                 angle,
                 arg0,
                 arg4,
                 arg8,
                 argC,
                 arg10,
                 arg14,
                 arg18,
                 arg1C,
                 arg20,
                 arg24,
                 arg28,
                 a11,
                 Material_material);
}

void mSay(gentity_t *from, gentity_t *to, int mode, const char *chatText)
{
    auto handled = false;

    //auto muted = PunishmentManager::IsUserPunished(IW4::MISC::GetXuid(from->EntityNumber), PUNISH_MUTE);

    if (mode == 99)
    {
        //printf("mode = 99\n");
        return oSay(from, to, 0, chatText);
    }
    else
        handled = ChatManager::HandleChatMessage(from, to, mode, chatText);

    if (!handled)
        return oSay(from, to, mode, chatText);
    /*else if (!handled && muted)
        return oSay(EntityManager::GetGEntity(IW4::MISC::GetHostId()), from, 0, "^7You are ^1muted^7!");*/
}

dvar_t* mSetFromStringByNameFromSource(const char* dvarName, const char* string, DvarSetSource source)
{
    //printf("byName: (%d) %s set to %s\n", source, dvarName, string);

    if (strcmp(dvarName, "g_speed") == 0)
    {
        auto value = atoi(string);
        auto g_speed = IW4::DVAR::FindDvar("g_speed");

        if (value >= g_speed->domain.integer.min && value <= g_speed->domain.integer.max)
        {
            auto bytes = Memory::IntToBytes(value);
            auto bsource = reinterpret_cast<BYTE*>(0x51D527);
            BYTE bdest[7] = { 0xC7, 0x45, 0x5C, bytes[3], bytes[2], bytes[1], bytes[0] };
            memcpy(bsource, bdest, 7 * sizeof BYTE);
        }
    }
    else if (strcmp(dvarName, "g_gravity") == 0)
    {
        auto value = atoi(string);
        auto g_speed = IW4::DVAR::FindDvar("g_gravity");

        if (value >= g_speed->domain.decimal.min && value <= g_speed->domain.decimal.max)
        {
            auto bytes = Memory::IntToBytes(value);
            auto bsource = reinterpret_cast<BYTE*>(0x51DDA3);
            BYTE bdest[7] = { 0xC7, 0x45, 0x54, bytes[3], bytes[2], bytes[1], bytes[0] };
            memcpy(bsource, bdest, 7 * sizeof BYTE);
        }
    }

    return oSetFromStringByNameFromSource(dvarName, string, source);
}

void msetClientDvar(scr_entref_t entref)
{
    auto client = entref.entnum;
    auto dvar = IW4::SCR::GetString(0);
    auto value = IW4::SCR::GetString(1);
    //printf("(%d) %s -> %s\n", client, dvar, value);

    /* Disble the forced cg_fov on spawn */
    if (strcmp(dvar, "cg_fov") == 0 && strcmp(value, "65") == 0) return;

    return osetClientDvar(entref);
}

__declspec(naked) void mSetPlayerData()
{
    F_PROLOG;

    //printf("SetPlayerData\n");

    //int result;
    //char **string;
    //char out[0x400];

    //__asm
    //{
    //	push ebx
    //	mov ebx, [esp + 0xC]
    //	mov string, ebx
    //	pop ebx
    //}

    ///*for (int i = 0; i < 0x400; i++)
    //	result = IW4::UNKN::String_Parse(string, &out[i], 0x400);*/

    //for (int i = 0; i < 8; i++)
    //{
    //	printf("string[%d] = \"%s\"\n", i, string[i]);
    //}

    //printf("String_Parse returned %d -> \"%s\".\n", result, out);

    F_EPILOG(oSetPlayerData);
}


void WriteMemory(LPVOID address, const char* bytes, const int len) {

    DWORD oProtect;
    VirtualProtect(address, len, PAGE_EXECUTE_READWRITE, &oProtect);

    memcpy(reinterpret_cast<LPVOID>(address), bytes, len);

    VirtualProtect(address, len, oProtect, nullptr);
}

void unprotectMemory(LPVOID address, const int len)
{
    DWORD oProtect;

    //DWORD oProtect;
    VirtualProtect(address, len, PAGE_EXECUTE_READWRITE, &oProtect);


    //VirtualProtect(address, len, oProtect, nullptr);
}


/*
CreateDetour initializes MW2LT.
All static managers are initialized and all hooking methods are called.
Patches various bytes in the IW4MP game.
After completion MW2LT is fully functional.
*/
DWORD WINAPI CreateDetour(LPVOID lpvoid)
{
    Logger::InitLogger(Logger::LOGGER_LEVEL_ALL);

    ConfigManager::InitWeaponList();
    ClientManager::InitClientInfos();
    ClientManager::InitClientInfos2();

    mMemoryManager = new MemoryManager;
    mDrawManager = new DrawManager;
    mCurrentGameManager = new CurrentGameManager;
    EventManager::SetCurrentGameManager(mCurrentGameManager);
    CustomEventManager::SetCurrentGameManager(mCurrentGameManager);
    GameManager::SetCurrentGameManager(mCurrentGameManager);
    GameManager::hInst = (HMODULE)lpvoid;

    printf("f1 - extra crates\nf2 - end game\nf3 - game force start\nf5 - infinity sprint\nf6 - toggle max players\nf7 - toggle auto host\nf9 - toggle third person\n");
    HookRefresh();
    HookDispatchMessageA();
    HookNotify();
    HookResume();
    MaxPlayersHook();
    MinPartyPlayers();
    HookSay();

    Memory::PatchDlcMapCheck();

    RceFix();
    HooksetClientDvar();

    PatchAllDvars();
    //PatchAllJumps();

    // always server
    GameManager::ToggleForceHost();

    ConfigManager::LoadConfig();

    return 0;
}

DWORD WINAPI doJob(LPVOID lpvoid)
{
    while (true)
    {
        LobbyMaxPlayersCountGlobal = GameManager::MaxPlayers ? 18 : 12;

        Sleep(500);
    }
    return 0;
}

class Resource {
public:
    struct Parameters {
        std::size_t size_bytes = 0;
        void* ptr = nullptr;
    };

private:
    HRSRC hResource = nullptr;
    HGLOBAL hMemory = nullptr;

    Parameters p;

public:
    Resource(int resource_id, const std::wstring &resource_class) {
        hResource = FindResourceW(nullptr, MAKEINTRESOURCEW(resource_id), resource_class.c_str());
        hMemory = LoadResource(nullptr, hResource);

        p.size_bytes = SizeofResource(nullptr, hResource);
        p.ptr = LockResource(hMemory);
    }
};
