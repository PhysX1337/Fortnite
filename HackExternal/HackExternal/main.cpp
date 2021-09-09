#include "function.h"
#include "overlay.h"
#include "cfg.h"
#include <vector>


LPCSTR TargetProcess = "D3D9Test.exe";
bool ShowMenu = true;
bool ImGui_Initialised = false;
bool CreateConsole = true;



namespace OverlayWindow
{
	WNDCLASSEX WindowClass;
	HWND Hwnd;
	LPCSTR Name;
}

namespace DirectX9Interface 
{
	IDirect3D9Ex* Direct3D9 = NULL;
	IDirect3DDevice9Ex* pDevice = NULL;
	D3DPRESENT_PARAMETERS pParams = { NULL };
	MARGINS Margin = { -1 };
	MSG Message = { NULL };
}



typedef struct _FortniteEntity
{
	uint64_t Actor;
	int ID;
	uint64_t mesh;
}FortniteEntity;

std::vector<FortniteEntity> entityList;


auto GameCache()->VOID
{
	while (true)
	{
		std::vector<FortniteEntity> tmpList;

		GameVars.U_World = read<DWORD_PTR>(GameVars.dwProcess_Base + GameOffset.OFFSET_UWORLD);
		GameVars.Game_Instance = read<DWORD_PTR>(GameVars.U_World + 0x180);
		GameVars.BaseLocal_Player = read<DWORD_PTR>(GameVars.Game_Instance + 0x38);
		GameVars.Local_Player = read<DWORD_PTR>(GameVars.BaseLocal_Player);
		GameVars.Player_Controller = read<DWORD_PTR>(GameVars.Local_Player + 0x30);
		GameVars.Local_Pawn = read<DWORD_PTR>(GameVars.Player_Controller + 0x2A0);
		GameVars.Player_State = read<DWORD_PTR>(GameVars.Local_Pawn + 0x240);
		GameVars.Root_Comp = read<DWORD_PTR>(GameVars.Local_Pawn + 0x130);

		if (GameVars.Local_Pawn != NULL)
		{
			GameVars.Local_PlayerID = read<int>(GameVars.Local_Pawn + 0x18);
		}

		GameVars.Persistent_Level = read<DWORD_PTR>(GameVars.U_World + 0x30);
		GameVars.ActorCount = read<DWORD>(GameVars.Persistent_Level + 0xA0);
		GameVars.AActors = read<DWORD_PTR>(GameVars.Persistent_Level + 0x98);

		for (int i = 0; i < GameVars.ActorCount; i++)
		{
			uint64_t CurrentActor = read<uint64_t>(GameVars.AActors + i * 0x8);

			if (CurrentActor == GameVars.Local_Pawn)
				continue;

			int current_ActorId = read<int>(CurrentActor + 0x18);

			if (current_ActorId == GameVars.Local_PlayerID || current_ActorId == GameVars.Local_PlayerID + 765)
			{
				FortniteEntity fortniteEntity{ };
				fortniteEntity.Actor = CurrentActor;
				fortniteEntity.mesh = read<uint64_t>(CurrentActor + 0x280);
				fortniteEntity.ID = current_ActorId;
				tmpList.push_back(fortniteEntity);

			}
		}
		entityList = tmpList;
		Sleep(1);
	}
}
auto RenderVisual()->VOID
{
	auto EntityList_Copy = entityList;

	for (int i = 0; i < EntityList_Copy.size(); ++i)
	{
		FortniteEntity Entity = EntityList_Copy[i];

		Vector3 head_pos = Fortnite::GetBoneWithRotation(Entity.mesh, 66);
		Vector3 local_pos = read<Vector3>(GameVars.Root_Comp + 0x11C);

		float entity_distance = local_pos.Distance(head_pos) / 100.f;

		DWORD64 Entity_PlayerState = read<uint64_t>(Entity.Actor + 0x240);

		//W2S
		Vector3 bone0 = Fortnite::GetBoneWithRotation(Entity.mesh, 0);
		Vector3 bottom = Fortnite::ProjectWorldToScreen(bone0);
		Vector3 Headbox = Fortnite::ProjectWorldToScreen(Vector3(head_pos.x, head_pos.y, head_pos.z + 15));

		float CornerHeight = abs(Headbox.y - bottom.y);
		float CornerWidth = CornerHeight * 0.65;

		std::string Entity_Name = read<string>(Entity_PlayerState + 0x300);
		std::cout << Entity_Name << std::endl;

		if (CFG.b_Visual)
		{
			if (entity_distance < CFG.distance)
			{
				if (CFG.b_EspBox)
				{
					if (CFG.in_BoxType == 0)
					{
						DrawBox(CFG.BoxColor, Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight);
					}
					else if (CFG.in_BoxType == 1)
					{
						DrawCorneredBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, CFG.BoxColor, 1.5);

					}
				}
				if (CFG.b_EspLine)
				{

					if (CFG.in_LineType == 0)
					{
						DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), static_cast<float>(GameVars.ScreenHeight)), ImVec2(bottom.x, bottom.y), CFG.LineColor, 1.5f); //LINE FROM BOTTOM SCREEN
					}
					if (CFG.in_LineType == 1)
					{
						DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), 0.f), ImVec2(bottom.x, bottom.y), CFG.LineColor, 1.5f); //LINE FROM TOP SCREEN
					}
					if (CFG.in_LineType == 2)
					{
						DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), static_cast<float>(GameVars.ScreenHeight / 2)), ImVec2(bottom.x, bottom.y), CFG.LineColor, 1.5f); //LINE FROM BOTTOM SCREEN
					}

				}
				if (CFG.b_EspDistance)
				{
					char dist[64];
					sprintf_s(dist, "Dist:[%.fm]", entity_distance);
					DrawStrokeText(bottom.x - 30, bottom.y, ImColor(255, 255, 255), dist);
				}
				if (CFG.b_EspSkeleton)
				{

					Vector3 vHipOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 2));
					Vector3 vNeckOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 65));
					Vector3 vUpperArmLeftOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 34));
					Vector3 vUpperArmRightOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 91));
					Vector3 vLeftHandOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 35));
					Vector3 vRightHandOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 63));
					Vector3 vLeftHandOut1 = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 33));
					Vector3 vRightHandOut1 = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 60));
					Vector3 vRightThighOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 74));
					Vector3 vLeftThighOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 67));
					Vector3 vRightCalfOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 75));
					Vector3 vLeftCalfOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 68));
					Vector3 vLeftFootOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 69));
					Vector3 vRightFootOut = Fortnite::ProjectWorldToScreen(Fortnite::GetBoneWithRotation(Entity.mesh, 76));

					DrawLine(ImVec2(vHipOut.x, vHipOut.y), ImVec2(vNeckOut.x, vNeckOut.y), CFG.SkeletonColor, 1.5f);

					DrawLine(ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), ImVec2(vNeckOut.x, vNeckOut.y), CFG.SkeletonColor, 1.5f);
					DrawLine(ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), ImVec2(vNeckOut.x, vNeckOut.y), CFG.SkeletonColor, 1.5f);

					DrawLine(ImVec2(vLeftHandOut.x, vLeftHandOut.y), ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), CFG.SkeletonColor, 1.5f);
					DrawLine(ImVec2(vRightHandOut.x, vRightHandOut.y), ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), CFG.SkeletonColor, 1.5f);

					DrawLine(ImVec2(vLeftHandOut.x, vLeftHandOut.y), ImVec2(vLeftHandOut1.x, vLeftHandOut1.y), CFG.SkeletonColor, 1.5f);
					DrawLine(ImVec2(vRightHandOut.x, vRightHandOut.y), ImVec2(vRightHandOut1.x, vRightHandOut1.y), CFG.SkeletonColor, 1.5f);

					DrawLine(ImVec2(vLeftThighOut.x, vLeftThighOut.y), ImVec2(vHipOut.x, vHipOut.y), CFG.SkeletonColor, 1.5f);
					DrawLine(ImVec2(vRightThighOut.x, vRightThighOut.y), ImVec2(vHipOut.x, vHipOut.y), CFG.SkeletonColor, 1.5f);

					DrawLine(ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), ImVec2(vLeftThighOut.x, vLeftThighOut.y), CFG.SkeletonColor, 1.5f);
					DrawLine(ImVec2(vRightCalfOut.x, vRightCalfOut.y), ImVec2(vRightThighOut.x, vRightThighOut.y), CFG.SkeletonColor, 1.5f);

					DrawLine(ImVec2(vLeftFootOut.x, vLeftFootOut.y), ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), CFG.SkeletonColor, 1.5f);
					DrawLine(ImVec2(vRightFootOut.x, vRightFootOut.y), ImVec2(vRightCalfOut.x, vRightCalfOut.y), CFG.SkeletonColor, 1.5f);

				}
			}
		}
	}

}

void Render()
{
	if (GetAsyncKeyState(VK_INSERT) & 1) 
		ShowMenu = !ShowMenu;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	RenderVisual();
	ImGui::GetIO().MouseDrawCursor = ShowMenu;

	if (ShowMenu == true) 
	{
		ImGui::SetNextWindowSize(ImVec2(675, 410)); // 450,426
		ImGui::PushFont(Century_Gothic);
		ImGui::Begin("Jenrix Fortnite", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

		TabButton("Visual", &CFG.in_tab_index, 0, false);

		if (CFG.in_tab_index == 0)
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::Checkbox("Enabled Visual", &CFG.b_Visual);
			if (CFG.b_Visual)
			{
				ImGui::Checkbox("ESP Box", &CFG.b_EspBox);
				if (CFG.b_EspBox)
				{
					ImGui::Combo("ESP Box Type", &CFG.in_BoxType, CFG.BoxTypes, 2);
					if (ImGui::ColorEdit4("ESP Box Color", CFG.fl_BoxColor))
					{
						CFG.BoxColor = ImColor(CFG.fl_BoxColor[0], CFG.fl_BoxColor[1], CFG.fl_BoxColor[2], CFG.fl_BoxColor[3]);
					}
				}

				ImGui::Checkbox("ESP Line", &CFG.b_EspLine);
				if (CFG.b_EspLine)
				{
					ImGui::Combo("ESP Line Type", &CFG.in_LineType, CFG.LineTypes, 3);
					if (ImGui::ColorEdit4("ESP Line Color", CFG.fl_LineColor))
					{
						CFG.LineColor = ImColor(CFG.fl_LineColor[0], CFG.fl_LineColor[1], CFG.fl_LineColor[2], CFG.fl_LineColor[3]);
					}
				}
				ImGui::Checkbox("ESP Skeleton", &CFG.b_EspSkeleton);
				if (CFG.b_EspSkeleton)
				{
					if (ImGui::ColorEdit4("ESP Skeleton Color", CFG.fl_SkeletonColor))
					{
						CFG.SkeletonColor = ImColor(CFG.fl_SkeletonColor[0], CFG.fl_SkeletonColor[1], CFG.fl_SkeletonColor[2], CFG.fl_SkeletonColor[3]);
					}
				}
				ImGui::Checkbox("ESP Distance", &CFG.b_EspDistance);
				ImGui::SliderInt("Max Distance", &CFG.CFG.distance, 0.f, 300.f);
			}
		}
		ImGui::PopFont();
		ImGui::End();
	}
	ImGui::EndFrame();

	DirectX9Interface::pDevice->SetRenderState(D3DRS_ZENABLE, false);
	DirectX9Interface::pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	DirectX9Interface::pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	DirectX9Interface::pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	if (DirectX9Interface::pDevice->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		DirectX9Interface::pDevice->EndScene();
	}

	HRESULT result = DirectX9Interface::pDevice->Present(NULL, NULL, NULL, NULL);
	if (result == D3DERR_DEVICELOST && DirectX9Interface::pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
		ImGui_ImplDX9_InvalidateDeviceObjects();
		DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

void MainLoop() {
	static RECT OldRect;
	ZeroMemory(&DirectX9Interface::Message, sizeof(MSG));

	while (DirectX9Interface::Message.message != WM_QUIT) {
		if (PeekMessage(&DirectX9Interface::Message, OverlayWindow::Hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&DirectX9Interface::Message);
			DispatchMessage(&DirectX9Interface::Message);
		}
		HWND ForegroundWindow = GetForegroundWindow();
		if (ForegroundWindow == GameVars.gameHWND) {
			HWND TempProcessHwnd = GetWindow(ForegroundWindow, GW_HWNDPREV);
			SetWindowPos(OverlayWindow::Hwnd, TempProcessHwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		RECT TempRect;
		POINT TempPoint;
		ZeroMemory(&TempRect, sizeof(RECT));
		ZeroMemory(&TempPoint, sizeof(POINT));

		GetClientRect(GameVars.gameHWND, &TempRect);
		ClientToScreen(GameVars.gameHWND, &TempPoint);

		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameVars.gameHWND;

		POINT TempPoint2;
		GetCursorPos(&TempPoint2);
		io.MousePos.x = TempPoint2.x - TempPoint.x;
		io.MousePos.y = TempPoint2.y - TempPoint.y;

		if (GetAsyncKeyState(0x1)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else {
			io.MouseDown[0] = false;
		}

		if (TempRect.left != OldRect.left || TempRect.right != OldRect.right || TempRect.top != OldRect.top || TempRect.bottom != OldRect.bottom) {
			OldRect = TempRect;
			GameVars.ScreenWidth = TempRect.right;
			GameVars.ScreenHeight = TempRect.bottom;
			DirectX9Interface::pParams.BackBufferWidth = GameVars.ScreenWidth;
			DirectX9Interface::pParams.BackBufferHeight = GameVars.ScreenHeight;
			SetWindowPos(OverlayWindow::Hwnd, (HWND)0, TempPoint.x, TempPoint.y, GameVars.ScreenWidth, GameVars.ScreenHeight, SWP_NOREDRAW);
			DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		}
		Render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (DirectX9Interface::pDevice != NULL) {
		DirectX9Interface::pDevice->EndScene();
		DirectX9Interface::pDevice->Release();
	}
	if (DirectX9Interface::Direct3D9 != NULL) {
		DirectX9Interface::Direct3D9->Release();
	}
	DestroyWindow(OverlayWindow::Hwnd);
	UnregisterClass(OverlayWindow::WindowClass.lpszClassName, OverlayWindow::WindowClass.hInstance);
}

bool DirectXInit() {
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &DirectX9Interface::Direct3D9))) {
		return false;
	}

	D3DPRESENT_PARAMETERS Params = { 0 };
	Params.Windowed = TRUE;
	Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	Params.hDeviceWindow = OverlayWindow::Hwnd;
	Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	Params.BackBufferWidth = GameVars.ScreenWidth;
	Params.BackBufferHeight = GameVars.ScreenHeight;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.EnableAutoDepthStencil = TRUE;
	Params.AutoDepthStencilFormat = D3DFMT_D16;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (FAILED(DirectX9Interface::Direct3D9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, OverlayWindow::Hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Params, 0, &DirectX9Interface::pDevice))) {
		DirectX9Interface::Direct3D9->Release();
		return false;
	}

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(OverlayWindow::Hwnd);
	ImGui_ImplDX9_Init(DirectX9Interface::pDevice);
	DirectX9Interface::Direct3D9->Release();
	return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message) {
	case WM_DESTROY:
		if (DirectX9Interface::pDevice != NULL) {
			DirectX9Interface::pDevice->EndScene();
			DirectX9Interface::pDevice->Release();
		}
		if (DirectX9Interface::Direct3D9 != NULL) {
			DirectX9Interface::Direct3D9->Release();
		}
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (DirectX9Interface::pDevice != NULL && wParam != SIZE_MINIMIZED) {
			ImGui_ImplDX9_InvalidateDeviceObjects();
			DirectX9Interface::pParams.BackBufferWidth = LOWORD(lParam);
			DirectX9Interface::pParams.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void SetupWindow() {
	OverlayWindow::WindowClass = {
		sizeof(WNDCLASSEX), 0, WinProc, 0, 0, nullptr, LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, OverlayWindow::Name, LoadIcon(nullptr, IDI_APPLICATION)
	};

	RegisterClassEx(&OverlayWindow::WindowClass);
	if (GameVars.gameHWND) {
		static RECT TempRect = { NULL };
		static POINT TempPoint;
		GetClientRect(GameVars.gameHWND, &TempRect);
		ClientToScreen(GameVars.gameHWND, &TempPoint);
		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		GameVars.ScreenWidth = TempRect.right;
		GameVars.ScreenHeight = TempRect.bottom;
	}

	OverlayWindow::Hwnd = CreateWindowEx(NULL, OverlayWindow::Name, OverlayWindow::Name, WS_POPUP | WS_VISIBLE, GameVars.ScreenLeft, GameVars.ScreenTop, GameVars.ScreenWidth, GameVars.ScreenHeight, NULL, NULL, 0, NULL);
	DwmExtendFrameIntoClientArea(OverlayWindow::Hwnd, &DirectX9Interface::Margin);
	SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	ShowWindow(OverlayWindow::Hwnd, SW_SHOW);
	UpdateWindow(OverlayWindow::Hwnd);
}

int main() 
{

	GameVars.dwProcessId = GetProcessIdByName(GameVars.dwProcessName);
	GameVars.gameHWND = GetHwndById(GameVars.dwProcessId);
	if (!GameVars.dwProcessId)
	{
		printf("[!] process \"%s\ was not found\n", GameVars.dwProcessName);
	}
	Controller = new DriverController(GameVars.dwProcessId);
	GameVars.dwProcess_Base = GetBaseAddress();
	if (!GameVars.dwProcess_Base)
	{
		printf("[!] failed to get baseadress\n");
	}
	RECT TempRect;
	GetWindowRect(GameVars.gameHWND, &TempRect);
	GameVars.ScreenWidth = TempRect.right - TempRect.left;
	GameVars.ScreenHeight = TempRect.bottom - TempRect.top;
	GameVars.ScreenLeft = TempRect.left;
	GameVars.ScreenRight = TempRect.right;
	GameVars.ScreenTop = TempRect.top;
	GameVars.ScreenBottom = TempRect.bottom;

	printf("[+] ProcessName: %s ID: (%d) base: %llx\n", GameVars.dwProcessName, GameVars.dwProcessId, GameVars.dwProcess_Base);
	HANDLE handle = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(GameCache), nullptr, NULL, nullptr);
	CloseHandle(handle);

	OverlayWindow::Name = RandomString(10).c_str();
	SetupWindow();
	DirectXInit();
	while (TRUE) 
	{
		MainLoop();
	}

}