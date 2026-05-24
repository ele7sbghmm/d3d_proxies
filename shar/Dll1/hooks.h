#pragma once

#include "Minhook.h"
#pragma comment(lib, "libMinhook.x86.lib")

#include "shar.h"
#include "draw.h"

extern Draw g_draw;

namespace hooks {

	namespace SP1 {
		void __stdcall TriggerPoints(D3DXVECTOR3* center) {
			g_draw.Init();
			g_draw.m_data.DrawCross(*shar::TriggerVolumeTracker::p_sP1, .2f, 0xffff0000);
			g_draw.m_data.DrawCross(*shar::TriggerVolumeTracker::p_sP2, .2f, 0xffff0000);
			g_draw.m_data.DrawCross(*shar::TriggerVolumeTracker::p_sP3, .2f, 0xffff0000);
			g_draw.m_data.DrawCross(*shar::TriggerVolumeTracker::p_sP4, .2f, 0xffff0000);
			g_draw.m_data.DrawCross(*center, .2f, 0xff00ff00);
		}

		void* addr = (void*)0x51206a;
		void* orig = nullptr;
		__declspec(naked) void hook() {
			__asm {
				pushad

				lea eax, [esp + 0xa4 + 0x20]
				push eax

				call TriggerPoints

				popad
				jmp orig
			}
		}

		void inject() {
			MH_CreateHook(addr, hook, (void**)&orig);
			MH_EnableHook(addr);
		}
	}
	namespace test_volume_collision {
		void __stdcall VolumeCollided(shar::TriggerVolume* volume, bool collided, D3DXVECTOR3* center) {
			g_draw.Init();
			D3DXVECTOR3 x  = *center + D3DXVECTOR3(.1, 0, 0);
			D3DXVECTOR3 y  = *center + D3DXVECTOR3(0, .1, 0);
			D3DXVECTOR3 z  = *center + D3DXVECTOR3(0, 0, .1);
			D3DXVECTOR3 nx = *center - D3DXVECTOR3(.1, 0, 0);
			D3DXVECTOR3 ny = *center - D3DXVECTOR3(0, .1, 0);
			D3DXVECTOR3 nz = *center - D3DXVECTOR3(0, 0, .1);

			g_draw.m_data.AddLine(x, nx, 0xff00ff00);
			g_draw.m_data.AddLine(y, ny, 0xff00ff00);
			g_draw.m_data.AddLine(z, nz, 0xff00ff00);
		}

		void* addr = (void*)0x51206a;
		void* orig = nullptr;
		__declspec(naked) void hook() {
			__asm {
				pushad

				lea eax, [esp + 0xa4 + 0x20]
				push eax
				push al
				push esi // volume

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
	namespace test_volume_radius {
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
		void __stdcall Fcompp(shar::TriggerVolume* volume, float rangeSq, float distSq, float sumRadius) {
			g_draw.DrawTriggerVolume(volume, rangeSq, distSq, sumRadius);
		}
		void* addr = (void*)0x511cb0;
		void* orig = nullptr;
		__declspec(naked) void hook() {
			__asm {
				pushad

				sub esp, 0xc
				fst float ptr [esp + 0] // sumRadius
				fxch st(1)
				fst float ptr [esp + 4] // distSq
				fxch st(1)
				fxch st(2)
				fst float ptr [esp + 8] // rangeSq
				fxch st(2)

				push [esi] // trigger volume ptr

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

		//test_volume_active::inject();
		test_volume_collision::inject();
		render_translucent::inject();
	}
}

