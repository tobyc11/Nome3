#pragma once
#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

namespace Nome
{

class CImGuiImplQt : public QObject
{
	Q_OBJECT

public:
	CImGuiImplQt(QWidget* parent);
	~CImGuiImplQt();

	void NewFrame();

	bool eventFilter(QObject* watched, QEvent* event);

private:
	void onMousePressedChange(QMouseEvent* event);
	bool onWheel(QWheelEvent* event);
	void onKeyPressRelease(QKeyEvent* event);
	bool onMouseMove(QMouseEvent* event);

	void UpdateMousePosAndButtons();
	void UpdateMouseCursor();

	QWidget* Parent;

	double Time = 0.0f;
	bool MousePressed[3] = { false, false, false };
	int MouseX = -1, MouseY = -1;
};

}
