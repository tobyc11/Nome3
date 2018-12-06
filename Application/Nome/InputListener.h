#pragma once

namespace Nome
{

struct CMouseState
{
	int x, y;
	int dx, dy;
};

class IMouseListener
{
public:
	virtual ~IMouseListener() = default;

	virtual bool MouseMoved(const CMouseState& state) = 0;
	virtual bool MouseButtonPressed(int index) = 0;
	virtual bool MouseButtonReleased(int index) = 0;
};

}
