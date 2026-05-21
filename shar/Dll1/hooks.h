#pragma once

#include "Minhook.h"
#pragma comment(lib, "libMinhook.x86.lib")

#include "shar.h"
#include "draw.h"


namespace hooks {
	namespace test_volume {
		void __stdcall TestVolume(
			shar::TriggerVolumeTracker* this_,
			shar::TriggerVolume* vol,
			shar::rmt::Vector* center,
			float radius
		) {
			if (g_draw.inited)
				g_draw.m_data.AddCircle(*center, radius, 0xff00fff00);
		}

		void* addr = (void*)0x511f30;
		void* orig = nullptr;
		__declspec(naked) void hook() {
			__asm {
				pushad
				
				lea edi, [esp + 0x20]
				push [edi+8] // radius
				push [edi+4] // center*
				push ecx   // RectTriggerVolume
				push edx   // TriggerVolumeTracker
				call TestVolume

				popad
				jmp orig
			}
		}

		void inject() {
			MH_CreateHook(addr, hook, (void**)&orig);
			MH_EnableHook(addr);
		}
	}

	namespace fcompp {
		void __stdcall Fcompp(float rangeSq, float distSq,
			float sumRadius, float centerX, float centerY, float centerZ) {
			g_draw.Init();
			D3DXVECTOR3 center{ centerX, centerY, centerZ };
			D3DCOLOR c = distSq < rangeSq ? 0xff00ffff : 0xff000000;
			if (distSq < rangeSq) g_draw.m_data.AddCircle(center, sumRadius, c);
		}
		void* addr = (void*)0x511cb0;
		void* orig = nullptr;
		__declspec(naked) void hook() {
			__asm {
				pushad

				push [esp + 0x40]
				push [esp + 0x40]
				push [esp + 0x40]
				
				sub esp, 0xc
				fst float ptr [esp + 0]
				fxch st(1)
				fst float ptr [esp + 4]
				fxch st(1)
				fxch st(2)
				fst float ptr [esp + 8]
				fxch st(2)

				call Fcompp

				popad
				jmp orig
			}
		}
		void inject() {
			MH_CreateHook(addr, hook, (void**)&orig);
			MH_EnableHook(addr);
		}
	}
	namespace render_transparent {
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

		//test_volume::inject();
		fcompp::inject();
		render_transparent::inject();
	}
}

