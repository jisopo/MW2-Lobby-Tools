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
class Memory
{
public:
    static void ToggleFostHost(bool state);
    static void PatchDlcMapCheck();
    static void ToggleDisableSpectatorKick(bool state);
	static void DisableLobbyVac(bool state);
	static void ToggleChopperBoxes(bool state);
	static void ToggleThermalVision(bool state);
	static void ToggleSpread(bool state);
	static void ToggleRecoil(bool state);

	static void ToggleOpcodes(BYTE op1, BYTE op2, LPVOID address);
	static void WriteOpCodes(BYTE op, int number, LPVOID address);
	static void WriteOpCodeBuffer(BYTE *buffer, int length, LPVOID address);

	static void WriteBytes(unsigned int address, char* bytes, int length);
	static void WriteBytes(LPVOID address, char* bytes, int length);

	static void RewriteFloatOp(DWORD source, float *destination);

	static BYTE *IntToBytes(int source);
};
