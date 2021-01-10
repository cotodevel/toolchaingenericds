#include "pong.h"
#include "amigascreen.h"
#include "amigawindow.h"
#include "woopsifuncs.h"
#include "woopsitimer.h"

Pong::Pong(AmigaScreen* screen) {
	_screen = screen;
	_p1x = 0;
	_p1y = 0;
	_p2x = 107;
	_p2y = 0;
	_pHeight = 10;
	_pWidth = 5;
	_bx = 10;
	_by = 14;
	_bRadius = 2;
	_bxSpeed = 1;
	_bySpeed = 1;
	_upHeld = false;
	_downHeld = false;
	
	_font = defaultGadgetStyle->font;
	_fontRColour = 0;
	_fontGColour = 0;
	_fontBColour = 0;

	initGUI();
}

Pong::~Pong() {
	delete _font;
	_font = NULL;
}

void Pong::initGUI() {
	// Create window
	_window = new AmigaWindow(0, 13, 120, 60, "Pong", Gadget::GADGET_DRAGGABLE, AmigaWindow::AMIGA_WINDOW_SHOW_CLOSE | AmigaWindow::AMIGA_WINDOW_SHOW_DEPTH);
	_screen->addGadget(_window);
	_window->addGadgetEventHandler(this);
	
	// Create timer
	_timer = new WoopsiTimer(1, true);
	_window->addGadget(_timer);
	_timer->addGadgetEventHandler(this);
	_timer->start();
}

void Pong::play() {
	draw();
	moveObjects();
}

void Pong::moveObjects() {
	_bx += _bxSpeed;
	_by += _bySpeed;

	Rect rect;
	_window->getClientRect(rect);

	if (_bx > rect.width) {
		_bx = rect.width;
		_bxSpeed = -_bxSpeed;
	}

	if (_bx < 0) {
		_bx = 0;
		_bxSpeed = -_bxSpeed;
	}

	if (_by > rect.height - _bRadius) {
		_by = rect.height - _bRadius;
		_bySpeed = -_bySpeed;
	}

	if (_by < _bRadius) {
		_by = _bRadius;
		_bySpeed = -_bySpeed;
	}

	if (_upHeld && (_p1y > 0)) {
		_p1y--;
	} else if (_downHeld && (_p1y < 33)) {
		_p1y++;
	}

	if (_bxSpeed > 0) {
		if ((_p2y < _by) && (_p2y < 33)) {
			_p2y++;
		} else if ((_p2y > _by) && (_p2y > 0)) {
			_p2y--;
		}
	}
}

void Pong::draw() {
	// Get graphics port
	GraphicsPort* port = _window->newGraphicsPort(false);

	// Background
	port->drawFilledRect(0, 0, 112, 43, woopsiRGB(0, 0, 0));

	// Draw dividing line
	port->drawLine(56, 0, 56, 99, woopsiRGB(0, 31, 0));

	// Players
	port->drawFilledRect(_p1x, _p1y, _pWidth, _pHeight, woopsiRGB(0, 31, 0));
	port->drawFilledRect(_p2x, _p2y, _pWidth, _pHeight, woopsiRGB(0, 31, 0));

	// Ball
	port->drawFilledEllipse(_bx, _by, _bRadius, _bRadius, woopsiRGB(0, 31, 0));

	// Text
	port->drawText(15, 2, _font, "It's Pong!", 0, 10, woopsiRGB(_fontRColour, _fontGColour, _fontBColour));

	delete port;

	// Font colour rotation
	_fontRColour++;
	if (_fontRColour > 31) {
		_fontRColour = 0;
		_fontGColour++;
		if (_fontGColour > 31) {
			_fontGColour = 0;
			_fontBColour++;
			if (_fontBColour > 31) {
				_fontBColour = 0;
			}
		}
	}
}

void Pong::handleActionEvent(const GadgetEventArgs& e) {
	if (e.getSource() == _timer) {
		play();
	}
}

void Pong::handleKeyPressEvent(const GadgetEventArgs& e) {
	if (_window->hasFocus()) {
		switch (e.getKeyCode()) {
			case KEY_CODE_UP:
				_upHeld = true;
				break;
			case KEY_CODE_DOWN:
				_downHeld = true;
				break;
			default:
				break;
		}
	}
}

void Pong::handleKeyReleaseEvent(const GadgetEventArgs& e) {
	if (_window->hasFocus()) {
		switch (e.getKeyCode()) {
			case KEY_CODE_UP:
				_upHeld = false;
				break;
			case KEY_CODE_DOWN:
				_downHeld = false;
				break;
			default:
				break;
		}
	}
}
