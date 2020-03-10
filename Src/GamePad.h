/**
*@file GamePad.h
*/

#ifndef GAMEPAD_H_INCLUDED
#define GAMEPAD_H_INCLUDED
#include <stdint.h>

/**
*ゲームパッド情報.
*/

struct GamePad
{

	//キーとボタンに対応するビット定数.
	static const uint32_t DPAD_UP = 0x0001;
	static const uint32_t DPAD_DOWN = 0x0002;
	static const uint32_t DPAD_LEFT = 0x0004;
	static const uint32_t DPAD_RIGHT = 0x0008;
	static const uint32_t START = 0x0010;
	static const uint32_t A = 0x0020;
	static const uint32_t B = 0x0040;
	static const uint32_t X = 0x0080;
	static const uint32_t Y = 0x0100;
	static const uint32_t L = 0x0200;
	static const uint32_t R = 0x0400;
	static const uint32_t SPACE = 0x0800;
	static const uint32_t H = 0x1000;
	static const uint32_t O = 0x2000;
	static const uint32_t P = 0x4000;
	static const uint32_t Z = 0x8000;
	static const uint32_t I = 0x10000;
	static const uint32_t YY = 0x20000;
	static const uint32_t N = 0x40000;
	static const uint32_t M = 0x80000;
	static const uint32_t XX = 0x100000;
	static const uint32_t C = 0x200000;
	static const uint32_t L2 = 0x40'0000;
	static const uint32_t R2 = 0x80'0000;
	static const uint32_t LL = 0x100'0000;
	static const uint32_t V = 0x200'0000;
	static const uint32_t BB = 0x400'0000;

	
	uint32_t buttons = 0;		///<押されている間フラグが立つ.
	uint32_t buttonDown = 0;	///<押された瞬間だけフラグが立つ.
};


#endif // !GAMEPAD_H_INCLUDED

