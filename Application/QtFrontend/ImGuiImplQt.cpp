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

static Qt::CursorShape MouseCursors[ImGuiMouseCursor_COUNT] =
{
	Qt::ArrowCursor,
	Qt::IBeamCursor,
	Qt::SizeAllCursor,
	Qt::SizeVerCursor,
	Qt::SizeHorCursor,
	Qt::SizeBDiagCursor,
	Qt::SizeFDiagCursor,
	Qt::PointingHandCursor
};

CImGuiImplQt::CImGuiImplQt(QWidget* parent) : Parent(parent)
{
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;       // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;        // We can honor io.WantSetMousePos requests (optional, rarely used)

	for (ImGuiKey key : keyMap.values()) {
		io.KeyMap[key] = key;
	}

#ifdef _WIN32
	io.ImeWindowHandle = (HWND)Parent->winId();
#endif

	Parent->installEventFilter(this);
}

CImGuiImplQt::~CImGuiImplQt()
{
	Parent->removeEventFilter(this);
}

void CImGuiImplQt::NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.Fonts->IsBuilt());     // Font atlas needs to be built, call renderer _NewFrame() function e.g. ImGui_ImplOpenGL3_NewFrame() 

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
	UpdateMousePosAndButtons();
	UpdateMouseCursor();
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
		return this->onWheel(static_cast<QWheelEvent*>(event));
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

bool CImGuiImplQt::onWheel(QWheelEvent* event)
{
	ImGuiIO& io = ImGui::GetIO();
	if (event->angleDelta().x() > 0) io.MouseWheelH += 1;
	if (event->angleDelta().x() < 0) io.MouseWheelH -= 1;
	if (event->angleDelta().y() > 0) io.MouseWheel += 1;
	if (event->angleDelta().y() < 0) io.MouseWheel -= 1;

	if (io.WantCaptureMouse)
		return true;
	return false;
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

void CImGuiImplQt::UpdateMousePosAndButtons()
{
	ImGuiIO& io = ImGui::GetIO();

	// Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
	if (io.WantSetMousePos)
	{
		QCursor c = Parent->cursor();
		c.setPos(Parent->mapToGlobal(QPoint((int)io.MousePos.x, (int)io.MousePos.y)));
		Parent->setCursor(c);
	}
	else
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

	for (int i = 0; i < 3; i++)
	{
		io.MouseDown[i] = MousePressed[i];
	}

	io.MousePos = ImVec2(MouseX, MouseY);
}

void CImGuiImplQt::UpdateMouseCursor()
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
		return;

	QCursor c = Parent->cursor();
	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		c.setShape(Qt::BlankCursor);
		Parent->setCursor(c);
	}
	else
	{
		// Show OS mouse cursor
		c.setShape(MouseCursors[imgui_cursor]);
		Parent->setCursor(c);
	}
}

}
