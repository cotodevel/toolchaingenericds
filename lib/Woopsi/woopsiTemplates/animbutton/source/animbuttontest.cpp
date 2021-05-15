// Includes
#include "animbuttontest.h"
#include "amigascreen.h"
#include "amigawindow.h"
#include "debug.h"
#include "ik.h"

void  AnimButtonTest::startup() {

	// Create screen
	AmigaScreen* screen = new AmigaScreen("Test Screen", Gadget::GADGET_DRAGGABLE, AmigaScreen::AMIGA_SCREEN_SHOW_DEPTH | AmigaScreen::AMIGA_SCREEN_SHOW_FLIP);
	woopsiApplication->addGadget(screen);

	// Add window
	AmigaWindow* window = new AmigaWindow(0, 13, 256, 179, "Test Window", Gadget::GADGET_DRAGGABLE, AmigaWindow::AMIGA_WINDOW_SHOW_CLOSE | AmigaWindow::AMIGA_WINDOW_SHOW_DEPTH);
	screen->addGadget(window);
	
	// Add test buttons
	_shelve = new Button(10, 70, 100, 20, "Shelve");
	_unshelve = new Button(10, 90, 100, 20, "Unshelve");
	_show = new Button(10, 110, 100, 20, "Show");
	_hide = new Button(10, 130, 100, 20, "Hide");
	_dimensions = new Button(10, 150, 100, 20, "Dimensions");
	_move = new Button(110, 70, 100, 20, "Move");
	_resize = new Button(110, 90, 100, 20, "Resize");
	_autosize = new Button(110, 110, 100, 20, "Auto size");
	_enable = new Button(110, 130, 100, 20, "Enable");
	_disable = new Button(110, 150, 100, 20, "Disable");
	
	_shelve->setRefcon(2);
	_unshelve->setRefcon(3);
	_show->setRefcon(4);
	_hide->setRefcon(5);
	_move->setRefcon(6);
	_resize->setRefcon(7);
	_autosize->setRefcon(8);
	_enable->setRefcon(9);
	_disable->setRefcon(10);
	_dimensions->setRefcon(11);
	
	window->addGadget(_shelve);
	window->addGadget(_unshelve);
	window->addGadget(_show);
	window->addGadget(_hide);
	window->addGadget(_move);
	window->addGadget(_resize);
	window->addGadget(_autosize);
	window->addGadget(_enable);
	window->addGadget(_disable);
	window->addGadget(_dimensions);
	
	_shelve->addGadgetEventHandler(this);
	_unshelve->addGadgetEventHandler(this);
	_show->addGadgetEventHandler(this);
	_hide->addGadgetEventHandler(this);
	_move->addGadgetEventHandler(this);
	_resize->addGadgetEventHandler(this);
	_autosize->addGadgetEventHandler(this);
	_enable->addGadgetEventHandler(this);
	_disable->addGadgetEventHandler(this);
	_dimensions->addGadgetEventHandler(this);
	
	// Create bitmaps for button
	_bitmaps = (BitmapWrapper**)TGDSARM9Malloc(10 * sizeof(BitmapWrapper*));
	_bitmaps[0] = new BitmapWrapper(ik1_Bitmap, 111, 53);
	_bitmaps[1] = new BitmapWrapper(ik2_Bitmap, 111, 53);
	_bitmaps[2] = new BitmapWrapper(ik3_Bitmap, 111, 53);
	_bitmaps[3] = new BitmapWrapper(ik4_Bitmap, 111, 53);
	_bitmaps[4] = new BitmapWrapper(ik5_Bitmap, 111, 53);
	_bitmaps[5] = new BitmapWrapper(ik6_Bitmap, 111, 53);
	_bitmaps[6] = new BitmapWrapper(ik7_Bitmap, 111, 53);
	_bitmaps[7] = new BitmapWrapper(ik8_Bitmap, 111, 53);
	_bitmaps[8] = new BitmapWrapper(ik9_Bitmap, 111, 53);
	_bitmaps[9] = new BitmapWrapper(ik10_Bitmap, 111, 53);

	// Add button
	_button = new AnimButton(30, 30, 111, 53, 0, 0);
	
	for (u8 i = 0; i < 5; ++i) {
		_button->getNormalAnimation()->addFrame(_bitmaps[i + 5], 0);
		_button->getClickedAnimation()->addFrame(_bitmaps[i], 0);
	}
	
	_button->getNormalAnimation()->setSpeed(4);
	_button->getClickedAnimation()->setSpeed(4);
	
	window->addGadget(_button);
	_button->addGadgetEventHandler(this);
	_button->setDoubleClickable(true);
	_button->setRefcon(1);

	// Get preferred dimensions for label and resize
	Debug::printf("getPreferredDimensions()");
	Rect rect;
	_button->getPreferredDimensions(rect);
	_button->resize(rect.width, rect.height);
	
	// Ensure Woopsi can draw itself
	enableDrawing();
	
	// Draw GUI
	redraw();
}

void  AnimButtonTest::shutdown() {

	// Clean up
	for (u8 i = 0; i < 10; ++i) {
		delete _bitmaps[i];
	}
	
	TGDSARM9Free(_bitmaps);

	// Call base shutdown method
	Woopsi::shutdown();
}

void  AnimButtonTest::handleClickEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Clicked");
			break;
	}
}

void  AnimButtonTest::handleDragEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Dragged");
			break;
	}
}

void  AnimButtonTest::handleReleaseEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Released");
			break;
	}
}

void  AnimButtonTest::handleReleaseOutsideEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Released outside");
			break;
	}
}

void  AnimButtonTest::handleKeyPressEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Key pressed");
			break;
	}
}

void  AnimButtonTest::handleKeyReleaseEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Key released");
			break;
	}
}

void  AnimButtonTest::handleLidOpenEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Lid opened");
			break;
	}
}

void  AnimButtonTest::handleLidCloseEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Lid closed");
			break;
	}
}

void  AnimButtonTest::handleFocusEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Focused");
			break;
	}
}

void  AnimButtonTest::handleBlurEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Blurred");
			break;
	}
}

void  AnimButtonTest::handleCloseEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Closed");
			break;
	}
}

void  AnimButtonTest::handleHideEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Hidden");
			break;
	}
}

void  AnimButtonTest::handleShowEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Shown");
			break;
	}
}

void  AnimButtonTest::handleEnableEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Enabled");
			break;
	}
}

void  AnimButtonTest::handleDisableEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Disabled");
			break;
	}
}

void  AnimButtonTest::handleValueChangeEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Value changed");
			break;
	}
}

void  AnimButtonTest::handleResizeEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Resized");
			break;
	}
}

void  AnimButtonTest::handleMoveEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Moved");
			break;
	}
}

void  AnimButtonTest::handleScrollEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Scrolled");
			break;
	}
}

void  AnimButtonTest::handleShiftClickEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Shift clicked");
			break;
	}
}

void  AnimButtonTest::handleContextMenuSelectionEvent(const ContextMenuEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Context menu selected");
			break;
	}
}

void  AnimButtonTest::handleDoubleClickEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Double clicked");
			break;
	}
}

void  AnimButtonTest::handleShelveEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Shelved");
			break;
	}
}

void  AnimButtonTest::handleUnshelveEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Unshelved");
			break;
	}
}

void  AnimButtonTest::handleActionEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Action");
			break;
		case 2:
			_button->shelve();
			break;
		case 3:
			_button->unshelve();
			break;
		case 4:
			_button->show();
			break;
		case 5:
			_button->hide();
			break;
		case 6:
			{
				u16 newPos;
				if (_button->getX() == 10) {
					newPos = 30;
				} else {
					newPos = 10;
				}
				_button->moveTo(newPos, newPos);
				break;
			}
		case 7:
			_button->resize(10, 10);
			break;
		case 8:
			{
				Rect rect;
				_button->getPreferredDimensions(rect);
				_button->resize(rect.width, rect.height);
				break;
			}
		case 9:
			_button->enable();
			break;
		case 10:
			_button->disable();
			break;
		case 11:
			{
				u16 newPos;
				if (_button->getX() == 10) {
					newPos = 30;
				} else {
					newPos = 10;
				}
				_button->changeDimensions(newPos, newPos, newPos, newPos);
				break;
			}
	}
}

void  AnimButtonTest::handleMoveForwardEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Moved forwards");
			break;
	}
}

void  AnimButtonTest::handleMoveBackwardEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		case 1:
			Debug::printf("Moved backwards");
			break;
	}
}
