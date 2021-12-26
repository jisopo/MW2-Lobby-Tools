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

#include "EntityManager.h"
#include "Hooking.h"

LRESULT WINAPI mDispatchMessage(MSG* lpmsg);
void mRefresh(int localClientNum);
void mNotify(unsigned int notifyListOwnerId, unsigned int stringValue, VariableValue *top);
int Info_ValueForKey(int a1, int a2, int a3, int a4);
void mResume(unsigned int timeId);
void mAddText(int localClientNum, const char *text);
void mSay(gentity_t *from, gentity_t *to, int mode, const char *chatText);
void mDropClient(int clientNum, const char *reason);
void mDropClient2(void *client_s, const char *reason, bool tellThem);
void mComError(int errorParamT, const char* msg, bool tellThem);
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
    char** Material_material);
void mRceFix(const char* localClientNum, const char* netadr_t_from, const char* msg_t_msg, const char* time, const char* arg5, const char* arg6);
dvar_t * mSetFromStringByNameFromSource(const char *dvarName, const char *string, DvarSetSource source);
void msetClientDvar(scr_entref_t entref);

void mSetPlayerData();

DWORD WINAPI CreateDetour(LPVOID lpvoid);
DWORD WINAPI InitNamedPipe(LPVOID lpvoid);
DWORD WINAPI doJob(LPVOID lpvoid);