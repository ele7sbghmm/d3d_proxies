#pragma once

#include "Minhook.h"
#pragma comment(lib, "libMinhook.x86.lib")

#include "shar.h"
#include "draw.h"

extern Draw g_draw;

namespace hooks {
	namespace trigger_test_rect {
		void __stdcall draw_stuff(shar::TriggerVolume** volume) {
			g_draw.DrawTriggerVolume(*volume, 1);
		}

		void* addr_rect = (void*)0x51206a;
		void* orig_rect = nullptr;
		__declspec(naked) void hook() {
			__asm {
				push [esp + 4];
				call draw_stuff
				jmp orig_rect
			}
		}
		void inject() {
			MH_CreateHook(addr_rect, hook, (void**)&orig_rect);
			MH_EnableHook(addr_rect);
		}
	}
	namespace test_volume_collision {
		void __stdcall VolumeCollided(D3DXVECTOR3* center) {
			g_draw.Init();
			g_draw.m_line.DrawTeethLine(
				*shar::TriggerVolumeTracker::p_sP1,
				*shar::TriggerVolumeTracker::p_sP2,
				.5f, 0xffff0000
			);
			g_draw.m_line.DrawTeethLine(
				*shar::TriggerVolumeTracker::p_sP3,
				*shar::TriggerVolumeTracker::p_sP4,
				.5f, 0xffff0000
			);
			g_draw.m_line.DrawTeethLine(
				*shar::TriggerVolumeTracker::p_sP1,
				*shar::TriggerVolumeTracker::p_sP4,
				.5f, 0xffff0000
			);
			g_draw.m_line.DrawTeethLine(
				*shar::TriggerVolumeTracker::p_sP2,
				*shar::TriggerVolumeTracker::p_sP3,
				.5f, 0xffff0000
			);

			g_draw.m_line.DrawCross(*center, .2f, 0xff00ff00);
		}


		void* addr = (void*)0x511c24;
		void* orig = nullptr;
		__declspec(naked) void hook() {
			__asm {
				pushad

				lea eax, [esp + 0x58 + 0x20]
				push eax

				call VolumeCollided	

				popad
				jmp orig
			}
		}

		void inject() {
			MH_CreateHook(addr, hook, (void**)&orig);
			MH_EnableHook(addr);
		}
	}
	namespace render_translucent {
		void __stdcall draw_stuff() {
			g_draw.DrawStuff();
		}

		void* addr = (void*)0x4aadc2;
		void* orig = nullptr;
		__declspec(naked) void hook() {
			__asm {
				pushad

				call draw_stuff

				popad
				jmp orig
			}
		}
		void inject() {
			MH_CreateHook(addr, hook, (void**)&orig);
			MH_EnableHook(addr);
		}
	}

	void inject() {
		MH_Initialize();

		trigger_test_rect::inject();
		test_volume_collision::inject();
		render_translucent::inject();
	}
}

