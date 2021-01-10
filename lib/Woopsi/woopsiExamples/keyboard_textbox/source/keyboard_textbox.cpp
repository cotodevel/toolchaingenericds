// Includes
#include "keyboard_textbox.h"
#include "woopsiheaders.h"

void KeyboardTextBox::startup() {

	/* Code below creates the output screen and associated gadgets */
	// Create screen
	AmigaScreen* outScreen = new AmigaScreen("Ouput Screen", Gadget::GADGET_PERMEABLE, AmigaScreen::AMIGA_SCREEN_SHOW_FLIP);
	woopsiApplication->addGadget(outScreen);
	
	// Move output screen to top display
	outScreen->flipToTopScreen();

	// Add output window
	AmigaWindow* window = new AmigaWindow(0, 13, 256, 179, "Key Test Window", Gadget::GADGET_DRAGGABLE, AmigaScreen::AMIGA_SCREEN_SHOW_FLIP);
	outScreen->addGadget(window);

	// Get available area within window
	Rect rect;
	window->getClientRect(rect);
	
	// Add textbox
	_output = new TextBox(rect.x, rect.y, rect.width, rect.height, "");
	_output->showCursor();
	window->addGadget(_output);
	
	/* Code below creates input screen and associated gadgets */
	// Create screen
	AmigaScreen* inScreen = new AmigaScreen("Input Screen", Gadget::GADGET_PERMEABLE, AmigaScreen::AMIGA_SCREEN_SHOW_FLIP);
	woopsiApplication->addGadget(inScreen);

	// Create keyboard
	_keyboard = new WoopsiKeyboard(0, 13, 0);
	_keyboard->addKeyboardEventHandler(_output);
	inScreen->addGadget(_keyboard);
	
	// Ensure Woopsi can draw itself
	enableDrawing();
	
	// Draw GUI
	redraw();
}

void KeyboardTextBox::shutdown() {

	// Call base shutdown method
	Woopsi::shutdown();
}
