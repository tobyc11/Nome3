#include "ImGuiImplQt.h"
#include <imgui.h>
#include <QDateTime>

namespace Nome
{

//https://github.com/seanchas116/qtimgui/blob/master/ImGuiRenderer.cpp
/*
The MIT License (MIT)

Copyright (c) 2017 Ryohei Ikegami

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

QHash<int, ImGuiKey> keyMap =
{
	{ Qt::Key_Tab, ImGuiKey_Tab },
	{ Qt::Key_Left, ImGuiKey_LeftArrow },
	{ Qt::Key_Right, ImGuiKey_RightArrow },
	{ Qt::Key_Up, ImGuiKey_UpArrow },
	{ Qt::Key_Down, ImGuiKey_DownArrow },
	{ Qt::Key_PageUp, ImGuiKey_PageUp },
	{ Qt::Key_PageDown, ImGuiKey_PageDown },
	{ Qt::Key_Home, ImGuiKey_Home },
	{ Qt::Key_End, ImGuiKey_End },
	{ Qt::Key_Delete, ImGuiKey_Delete },
	{ Qt::Key_Backspace, ImGuiKey_Backspace },
	{ Qt::Key_Enter, ImGuiKey_Enter },
	{ Qt::Key_Escape, ImGuiKey_Escape },
	{ Qt::Key_A, ImGuiKey_A },
	{ Qt::Key_C, ImGuiKey_C },
	{ Qt::Key_V, ImGuiKey_V },
	{ Qt::Key_X, ImGuiKey_X },
	{ Qt::Key_Y, ImGuiKey_Y },
	{ Qt::Key_Z, ImGuiKey_Z },
};

CImGuiImplQt::CImGuiImplQt(QWidget* parent) : Parent(parent)
{
	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	// Setup style
	ImGui::StyleColorsDark();

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'misc/fonts/README.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	for (ImGuiKey key : keyMap.values()) {
		io.KeyMap[key] = key;
	}

	parent->installEventFilter(this);
}

CImGuiImplQt::~CImGuiImplQt()
{
	ImGui::DestroyContext();
	Parent->removeEventFilter(this);
}

void CImGuiImplQt::NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	io.DisplaySize = ImVec2(Parent->size().width(), Parent->size().height());
	io.DisplayFramebufferScale = ImVec2(Parent->devicePixelRatio(), Parent->devicePixelRatio());

	// Setup time step
	double current_time = QDateTime::currentMSecsSinceEpoch() / double(1000);
	io.DeltaTime = Time > 0.0 ? (float)(current_time - Time) : (float)(1.0f / 60.0f);
	Time = current_time;

	// Setup inputs
	//if (Parent->isActive())
	//{
	//	auto pos = Parent->mapFromGlobal(QCursor::pos());
	//	io.MousePos = ImVec2(pos.x(), pos.y());   // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
	//}
	//else
	//{
	//	io.MousePos = ImVec2(-1, -1);
	//}
	io.MousePos = ImVec2(MouseX, MouseY);

	for (int i = 0; i < 3; i++)
	{
		io.MouseDown[i] = MousePressed[i];
	}

	io.MouseWheelH = MouseWheelH;
	io.MouseWheel = MouseWheel;
	MouseWheelH = 0;
	MouseWheel = 0;

	// Hide OS mouse cursor if ImGui is drawing it
	// glfwSetInputMode(g_Window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

	// Start the frame
	ImGui::NewFrame();
}

bool CImGuiImplQt::eventFilter(QObject* watched, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
		this->onMousePressedChange(static_cast<QMouseEvent*>(event));
		break;
	case QEvent::Wheel:
		this->onWheel(static_cast<QWheelEvent*>(event));
		break;
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
		this->onKeyPressRelease(static_cast<QKeyEvent*>(event));
		break;
	case QEvent::MouseMove:
		return this->onMouseMove(static_cast<QMouseEvent*>(event));
		break;
	default:
		break;
	}
	return QObject::eventFilter(watched, event);
}

void CImGuiImplQt::onMousePressedChange(QMouseEvent* event)
{
	MousePressed[0] = event->buttons() & Qt::LeftButton;
	MousePressed[1] = event->buttons() & Qt::RightButton;
	MousePressed[2] = event->buttons() & Qt::MiddleButton;
}

void CImGuiImplQt::onWheel(QWheelEvent* event)
{
	// 5 lines per unit
	MouseWheelH += event->pixelDelta().x() / (ImGui::GetTextLineHeight());
	MouseWheel += event->pixelDelta().y() / (5.0 * ImGui::GetTextLineHeight());
}

void CImGuiImplQt::onKeyPressRelease(QKeyEvent* event)
{
	ImGuiIO& io = ImGui::GetIO();
	if (keyMap.contains(event->key()))
	{
		io.KeysDown[keyMap[event->key()]] = event->type() == QEvent::KeyPress;
	}

	if (event->type() == QEvent::KeyPress)
	{
		QString text = event->text();
		if (text.size() == 1)
		{
			io.AddInputCharacter(text.at(0).unicode());
		}
	}

#ifdef Q_OS_MAC
	io.KeyCtrl = event->modifiers() & Qt::MetaModifier;
	io.KeyShift = event->modifiers() & Qt::ShiftModifier;
	io.KeyAlt = event->modifiers() & Qt::AltModifier;
	io.KeySuper = event->modifiers() & Qt::ControlModifier; // Comamnd key
#else
	io.KeyCtrl = event->modifiers() & Qt::ControlModifier;
	io.KeyShift = event->modifiers() & Qt::ShiftModifier;
	io.KeyAlt = event->modifiers() & Qt::AltModifier;
	io.KeySuper = event->modifiers() & Qt::MetaModifier;
#endif
}

bool CImGuiImplQt::onMouseMove(QMouseEvent* event)
{
	MouseX = event->x();
	MouseY = event->y();

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return true;
	return false;
}

}
