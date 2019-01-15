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
	void onWheel(QWheelEvent* event);
	void onKeyPressRelease(QKeyEvent* event);
	void onMouseMove(QMouseEvent* event);

	QWidget* Parent;

	double Time = 0.0f;
	bool MousePressed[3] = { false, false, false };
	float MouseWheel;
	float MouseWheelH;
	int MouseX = -1, MouseY = -1;
};

}
