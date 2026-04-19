#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include <memoryapi.h>
#include <stdint.h>
#include <cmath>

extern "C" {
	typedef struct HWND__* HWND;
	typedef intptr_t(__stdcall* WNDPROC)(HWND, unsigned int, uintptr_t, uintptr_t);
	#define GWL_EXSTYLE       -20
	#define WS_EX_LAYERED      0x00080000
	#define WS_EX_TRANSPARENT  0x00000020
	#define WM_NCHITTEST       0x84
	#define HTCLIENT           1
	#define HTTRANSPARENT     -1
	#define GWLP_WNDPROC      -4

	__declspec(dllimport) long __stdcall GetWindowLongA(HWND hWnd, int nIndex);
	__declspec(dllimport) long __stdcall SetWindowLongA(HWND hWnd, int nIndex, long dwNewLong);
	__declspec(dllimport) long __stdcall CallWindowProcA(WNDPROC lpPrevWndFunc, HWND hWnd, unsigned int Msg, uintptr_t wParam, uintptr_t lParam);
	// __declspec(dllimport) long __stdcall SetWindowLongPtrA(HWND hWnd, int nIndex, intptr_t dwNewLong);
}

WNDPROC originalWndProc = nullptr;
intptr_t __stdcall HookedWndProc(HWND hwnd, unsigned int msg, uintptr_t wp, uintptr_t lp) {
	if (msg == WM_NCHITTEST) {
		intptr_t hit = CallWindowProcA(originalWndProc, hwnd, msg, wp, lp);
		if (hit == HTCLIENT)
			return HTTRANSPARENT;
		return hit;
	}
	return CallWindowProcA(originalWndProc, hwnd, msg, wp, lp);
}

const int FENCE_INIT_COUNT = 10000;
struct Fence {
	Vector3 s;
	Vector3 e;
	int c;
};
struct Map {
	Vector3 cameraPos;
	Vector3 cameraTarget;
	float cameraFov;
	float cameraAspect;
	Vector3 avatarPos;
	Fence fences[FENCE_INIT_COUNT];
};

int main() {
	HANDLE hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"Local\\MazaFileMapping3");
	if (hMapFile == nullptr)
		return 1;
	
	Map* map = (Map*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Map));
	if (map == nullptr)
		return 1;

	SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_TOPMOST);
	InitWindow(800, 600, "rl");

	HWND hwnd = (HWND)GetWindowHandle();
	originalWndProc = (WNDPROC)SetWindowLongA(hwnd, GWLP_WNDPROC, (intptr_t)HookedWndProc);
	SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLongA(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	// long style = GetWindowLongA(hwnd, GWL_EXSTYLE);
	// SetWindowLongA(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED | WS_EX_TRANSPARENT);
	SetWindowPosition(80, 600);

	Camera3D cam = { 0 };
	cam.position = Vector3{};
	cam.target = Vector3{};
	cam.up = Vector3{ 0., 1., 0. };
	cam.projection = CAMERA_PERSPECTIVE;// CAMERA_ORTHOGRAPHIC;

	SetTargetFPS(60.f);

	Font customFont = LoadFont("C:\\Windows\\Fonts\\cour.ttf");
	float fontSize = 30.f;
	float fontSpacing = 0.f;
	float ySpacing = fontSize / 3.f * 2.f;

	while (!WindowShouldClose()) {

		BeginDrawing();
		ClearBackground(BLANK);

		// rlScalef(-1.f, 1.f, 1.f);

		cam.position = map->cameraPos;
		cam.target = map->cameraTarget;

		cam.fovy = 2.f * atanf(tanf(map->cameraFov / 2.f) / map->cameraAspect) * (180.f / PI);

		Matrix viewMatrix = GetCameraMatrix(cam);
		Matrix projMatrix = rlGetMatrixProjection();
		Matrix worldToView = MatrixMultiply(viewMatrix, projMatrix);
		Vector2 screenPos = GetWorldToScreen(map->avatarPos, cam);

		float screenWidth = 800.f;
		float screenHeight = 600.f;
		screenPos.x = 800.f - screenPos.x;

		Vector3 avatarView = Vector3Transform(map->avatarPos, worldToView);

		cam.position.z *= -1;
		cam.target.z *= -1;

		BeginMode3D(cam);
		rlPushMatrix();
		
		// DrawGrid(10.f, 100.f);
		// DrawLine3D(map->avatarPos, map->cameraTarget, GREEN);
		for (int i = 0; i < FENCE_INIT_COUNT; i++) {
			Fence f = map->fences[i];
			Vector3 s = Vector3{ f.s.x, map->avatarPos.y, -f.s.z };
			Vector3 e = Vector3{ f.e.x, map->avatarPos.y, -f.e.z };
			Color c;
			switch (f.c) {
				case 0: c = BLACK; break;
				case 1: c = GRAY; break;
				case 2: c = RED; break;
				case 3: c = GREEN; break;
			}
			DrawLine3D(s, e, c
			);
		}

		rlPopMatrix();
		EndMode3D();

		DrawTextEx(customFont, TextFormat("camera: %+5.2f %+5.2f %+5.2f", map->cameraPos.x, map->cameraPos.y, map->cameraPos.z), Vector2{ 10.f, ySpacing * 1.f }, fontSize, fontSpacing, WHITE);
		DrawTextEx(customFont, TextFormat("target: %+5.2f %+5.2f %+5.2f", map->cameraTarget.x, map->cameraTarget.y, map->cameraTarget.z), Vector2{ 10.f, ySpacing * 2.f }, fontSize, fontSpacing, WHITE);
		DrawTextEx(customFont, TextFormat("avatar: %+5.2f %+5.2f %+5.2f", map->avatarPos.x, map->avatarPos.y, map->avatarPos.z), Vector2{ 10.f, ySpacing * 3.f }, fontSize, fontSpacing, WHITE);
		DrawTextEx(customFont, TextFormat("avView: %+5.2f %+5.2f %+5.2f", avatarView.x, avatarView.y, avatarView.z), Vector2{ 10.f, ySpacing * 4.f }, fontSize, fontSpacing, WHITE);
		DrawTextEx(customFont, TextFormat("screen: %+5.2f %+5.2f", screenPos.x, screenPos.y), Vector2{ 10.f, ySpacing * 5.f }, fontSize, fontSpacing, WHITE);
		DrawTextEx(customFont, TextFormat("%+.0f %+.0f", screenPos.x, screenPos.y), Vector2{ floorf(screenPos.x), floorf(screenPos.y) + 15.f}, fontSize, fontSpacing, WHITE);

		EndDrawing();
	}
}
