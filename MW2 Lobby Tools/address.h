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

// Memory patches
#define CHOPPER_ESP_1_a	        0x0048858B // 0x48854B
#define CHOPPER_ESP_2_a	        0x00488593 // 0x488553
#define CHOPPER_ESP_3_a	        0x004885A5 // 0x488565
#define COLDBLOOD_FIX	        0x004B751E // 0x4B74CE
#define THERMAL_a		        0x004AFACB // 0x4AFA7B
#define RECOIL_a		        0x004B9FCB // 0x4B9F7B
#define SPREAD_a		        0x0047A1AF // 0x47A16F
// скорее всего можно поменять через какой-то игровой dvar
#define SKIP_FIND_BEST_HOST     0x004D9ED5
// тоже криво выставил кварты
// просто патчим проверку что хост всегда 'unreach'
#define HOST_ALWAYS_UNREACH     0x004D5096
#define DISABLE_SPECTATOR_KICK  0x0058555F
// Steam game server vac insecure flag
// запускаем сервер который хостуем в 'небезопасном' режиме
// чтобы не получить вак бан
#define INSECURE_LOBBY 0x006293C4
#define DLC_MAP_CHECK 0x005BB978

// патчим проверку на открытый нат
#define NAT_TYPE 0x0085CCC0


#define doWeSuckAsHost 0x06427DE6

// User defined
#define MAX_CLIENTS		18
#define MAX_ENTITIES	2048
#define MAX_PLAYERSTATES 18

// iw4mp structs
#define CGS_a		    0x0089D420 // 0x89D3A0
#define CG_a			0x8A0E50
#define REFDEF_a		0x90B5C8
#define ENTITY_a		0x009A4110 //0x9A4090
#define ENTITY_s		0x204
#define CLIENT_a		0x00498750 //0x99785C
#define CLIENT_s		0x52C
#define CLIENT_b		0x10F9362 // Структура из Blasts_test
#define CLIENT_b_s		0xE0 // Структура из Blasts_test
#define CLIENT_S_s		0xA6790
#define CLIENT_S_a		0x3172010
#define GENTITY_a		0x0194B9D0 // 0x194B950
#define PlayerState_a   0x01B0E1C0
#define PlayerState_b   0x366C // NoClip
#define GENTITY_s		0x274
#define PARTYSESSION_a	0x6B3580
#define GAMESTATE_a		0x06641358 // 0x66412D8

#define G_LOBBYDATA		0x010F9268 //0x10F91E8
#define PARTYSESSION_P	0x010F5A98 //0x10F5A18

#define SELF_PLAYER_PRESTIGE 0x1B8B7F0
#define SELF_PLAYER_LEVEL    0x01B8B7E8 // rank

// iw4mp data
#define NUMPARAM		0x01FDE0F8 // 0x1FDE078
#define NEXTMAP			0x01B8F1A0 // 0x1B8F120
#define MAXPLAYERS_a	0x0664157C // 0x66414FC
#define JUMP_HEIGHT_a	0x67EC88
#define BG_FALLDAMAGEMINHEIGHT_a 0x67EC80
#define BG_FALLDAMAGEMAXHEIGHT_a 0x681DB4
#define G_GRAVITY_a		0x6831F0
