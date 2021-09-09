#include <windows.h>
#include <tchar.h>
#include <memory>
#include <thread>
#include <functional>
#include <array>
#include <dwmapi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include <sstream>

#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_dx9.h"
#include "Imgui/imgui_impl_win32.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dwmapi.lib")
#pragma comment(lib,"d3dcompiler") 

using namespace std;


ImFont* Century_Gothic, *Default_Font;

typedef struct
{
	DWORD R;
	DWORD G;
	DWORD B;
	DWORD A;
}RGBA;

std::string string_To_UTF8(const std::string& str) 
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	wchar_t* pwBuf = new wchar_t[nwLen + 1];
	ZeroMemory(pwBuf, nwLen * 2 + 2);
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
	char* pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);
	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
	std::string retStr(pBuf);
	delete[]pwBuf;
	delete[]pBuf;
	pwBuf = NULL;
	pBuf = NULL;
	return retStr;
}

auto DrawCrossHair(const FLOAT aSize, ImU32 aColor)-> VOID
{
	auto vList = ImGui::GetOverlayDrawList();

	vList->AddLine({ GameVars.ScreenCenterX ,GameVars.ScreenCenterY - (aSize + 1) }, { GameVars.ScreenCenterX ,GameVars.ScreenCenterY + (aSize + 1) }, aColor, 2);
	vList->AddLine({GameVars.ScreenCenterX - (aSize + 1),GameVars.ScreenCenterY }, { GameVars.ScreenCenterX + (aSize + 1)  ,GameVars.ScreenCenterY }, aColor, 2);
}

auto DrawLine(const ImVec2& aPoint1, const ImVec2 aPoint2, ImU32 aColor, const FLOAT aLineWidth) -> VOID
{
	auto vList = ImGui::GetOverlayDrawList();
	vList->AddLine(aPoint1, aPoint2, aColor, aLineWidth);
}

auto DrawBox(ImColor color, float x, float y, float w, float h)-> VOID
{
	DrawLine(ImVec2(x, y), ImVec2(x + w, y), color, 1.3f); // top 
	DrawLine(ImVec2(x, y - 1.3f), ImVec2(x, y + h + 1.4f), color, 1.3f); // left
	DrawLine(ImVec2(x + w, y - 1.3f), ImVec2(x + w, y + h + 1.4f), color, 1.3f);  // right
	DrawLine(ImVec2(x, y + h), ImVec2(x + w, y + h), color, 1.3f);   // bottom 
}
auto RectFilled(float x0, float y0, float x1, float y1, ImColor color, float rounding, int rounding_corners_flags)-> VOID
{
	auto vList = ImGui::GetOverlayDrawList();
	vList->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), color, rounding, rounding_corners_flags);
}
auto ProgressBar(float x, float y, float w, float h, int value, int v_max, ImColor barColor, bool Outlined, ImColor Outlinecolor)-> VOID
{
	auto vList = ImGui::GetOverlayDrawList();
	if (Outlined)
		vList->AddRect(ImVec2(x - 1, y - 1), ImVec2(x + w + 1, y + h + 1), Outlinecolor, 0.0f, 0, 2.0f);
	RectFilled(x, y, x + w, y + ((h / float(v_max)) * (float)value), barColor, 0.0f, 0);
}

auto DrawStrokeText(int x, int y, ImU32 color, const char* str) -> VOID
{
	ImFont a;
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = string_To_UTF8(utf_8_1);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x - 1, y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 1, y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::GetColorU32(color), utf_8_2.c_str());

}
auto DrawCorneredBox(float X, float Y, float W, float H, const ImU32& color, float thickness) -> VOID
{
	auto vList = ImGui::GetOverlayDrawList();

	float lineW = (W / 3);
	float lineH = (H / 3);
	//black outlines
	auto col = ImGui::GetColorU32(color);

	//corners
	vList->AddLine(ImVec2(X, Y - thickness / 2), ImVec2(X, Y + lineH), col, thickness);//top left
	vList->AddLine(ImVec2(X - thickness / 2, Y), ImVec2(X + lineW, Y), col, thickness);

	vList->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W + thickness / 2, Y), col, thickness);//top right horizontal
	vList->AddLine(ImVec2(X + W, Y - thickness / 2), ImVec2(X + W, Y + lineH), col, thickness);

	vList->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H + (thickness / 2)), col, thickness);//bot left
	vList->AddLine(ImVec2(X - thickness / 2, Y + H), ImVec2(X + lineW, Y + H), col, thickness);

	vList->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W + thickness / 2, Y + H), col, thickness);//bot right
	vList->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H + (thickness / 2)), col, thickness);

}

auto DrawCircle(const ImVec2& aPoint, const FLOAT aR, ImU32 aColor, const FLOAT aLineWidth) -> VOID
{
	auto vList = ImGui::GetOverlayDrawList();
	vList->AddCircle(aPoint, aR, aColor, 120, aLineWidth);
}
auto DrawCircle(float x, float y, float radius, ImVec4 color, int segments)-> VOID
{
	auto vList = ImGui::GetOverlayDrawList();
	vList->AddCircle(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(color), segments);
}
auto DrawString(const ImVec2& aPos, const std::string& aString, ImU32 aColor) -> VOID
{
	auto vList = ImGui::GetOverlayDrawList();
	vList->AddText(aPos, aColor, aString.data());
}
auto TabButton(const char* label, int* index, int val, bool sameline) -> VOID
{
	if (*index == val)
	{
		if (ImGui::Button(label, ImVec2(80, 25)))
			*index = val;
		if (sameline)
			ImGui::SameLine();
	}
	else
	{
		if (ImGui::Button(label, ImVec2(80, 25)))
			*index = val;
		if (sameline)
			ImGui::SameLine();
	}
}