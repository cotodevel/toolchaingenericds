// Includes
#include "helloworld.h"
#include "amigascreen.h"
#include "amigawindow.h"
#include "textbox.h"

void HelloWorld::startup() {

	// Create screen
	AmigaScreen* screen = new AmigaScreen("Hello World Screen", Gadget::GADGET_DRAGGABLE, AmigaScreen::AMIGA_SCREEN_SHOW_DEPTH | AmigaScreen::AMIGA_SCREEN_SHOW_FLIP);
	woopsiApplication->addGadget(screen);

	// Add window
	//AmigaWindow* window = new AmigaWindow(0, 13, 256, 179, "Hello World Window", Gadget::GADGET_DRAGGABLE, AmigaWindow::AMIGA_WINDOW_SHOW_CLOSE | AmigaWindow::AMIGA_WINDOW_SHOW_DEPTH);
	//screen->addGadget(window);
	
	// Add Welcome notice
	_alert = new Alert(2, 2, 200, 80, "Welcome!", "Welcome to Woopsi-TGDS!");
	screen->addGadget(_alert);
	
	// Get available area within window
	Rect rect;
	screen->getClientRect(rect);
	
	// Add textbox
	//TextBox* textbox = new TextBox(rect.x, rect.y, rect.width, rect.height, "Hello World!");
	//screen->addGadget(textbox);

	// Ensure Woopsi can draw itself
	enableDrawing();
	
	// Draw GUI
	redraw();
}

void HelloWorld::shutdown() {

	// Call base shutdown method
	Woopsi::shutdown();
}
