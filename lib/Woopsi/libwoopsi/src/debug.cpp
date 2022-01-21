
#include "typedefsTGDS.h"
#include "ipcfifoTGDS.h"
#include "posixHandleTGDS.h"
#include "linkerTGDS.h"

#include "dsregs_asm.h"
#include "devoptab_devices.h"
#include "errno.h"
#include "sys/stat.h"
#include "dirent.h"
#include "consoleTGDS.h"
#include "clockTGDS.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "limitsTGDS.h"
#include "dswnifi_lib.h"

#include "debug.h"
#include "woopsifuncs.h"
#include "amigascreen.h"
#include "amigawindow.h"
#include "woopsi.h"
#include "scrollingtextbox.h"
#include "fonts/tinyfont.h"
#include "consoleTGDS.h"
#include "posixHandleTGDS.h"

using namespace WoopsiUI;
Debug* Debug::_debug = NULL;

Debug::Debug() {
	_screen = NULL;
	_window = NULL;
	_textBox = NULL;
	_style = NULL;

	createGUI();
}

Debug::~Debug() {
	_screen->close();

	delete _style->font;
	delete _style;
}

void Debug::createDebug() {
	if (_debug == NULL) {
		_debug = new Debug();
	}
}

void Debug::output(const char* text) {
	if (DEBUG_ACTIVE) {
		if (woopsiApplication != NULL) {
			createDebug();

			_debug->_textBox->disableDrawing();
			_debug->_textBox->appendText(">");
			_debug->_textBox->appendText(text);
			_debug->_textBox->enableDrawing();
			_debug->_textBox->appendText("\n");
		}
	}
}

void Debug::printf(const char *fmt, ...) {
	//Indentical Implementation as GUI_printf
	va_list args;
	va_start (args, fmt);
	vsnprintf ((sint8*)ConsolePrintfBuf, (int)sizeof(ConsolePrintfBuf), fmt, args);
	va_end (args);
	
    // FIXME
    bool readAndBlendFromVRAM = false;	//we discard current vram characters here so if we step over the same character in VRAM (through printfCoords), it is discarded.
	t_GUIZone zone;
    zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
    zone.font = &smallfont_7_font;
	
	int color = (int)TGDSPrintfColor_LightGrey;	//default color
	int stringSize = (int)strlen((const char*)ConsolePrintfBuf);
	
	//Separate the TGDS Console font color if exists
	char cpyBuf[256+1] = {0};
	strcpy(cpyBuf, (const char*)ConsolePrintfBuf);
	char * outBuf = (char *)TGDSARM9Malloc(256*10);
	char * colorChar = (char*)((char*)outBuf + (1*256));
	int matchCount = str_split((char*)cpyBuf, ">", outBuf, 10, 256);
	if(matchCount > 0){
		color = atoi(colorChar);
		ConsolePrintfBuf[strlen((const char*)ConsolePrintfBuf) - (strlen((const char*)colorChar)+1) ] = '\0';
	}
	
    GUI_drawText(&zone, 0, GUI.printfy, color, (sint8*)ConsolePrintfBuf, readAndBlendFromVRAM);
    GUI.printfy += GUI_getFontHeight(&zone);
	TGDSARM9Free(outBuf);
}


void Debug::busyWait() {
	if (DEBUG_ACTIVE) {
		while (!Pad.Held.B) {
			woopsiWaitVBL(woopsiApplication->_waitForHardwareVblank);
		}
		while (Pad.Held.B) {
			woopsiWaitVBL(woopsiApplication->_waitForHardwareVblank);
		}
	}
}

void Debug::createGUI() {

	// Add debug screen
	if (_screen == NULL) {
		_screen = new AmigaScreen("Debug", Gadget::GADGET_DRAGGABLE, AmigaScreen::AMIGA_SCREEN_SHOW_DEPTH | AmigaScreen::AMIGA_SCREEN_SHOW_FLIP);
		woopsiApplication->addGadget(_screen);
		_screen->redraw();
	}

	// Add debug window
	if (_window == NULL) {
		_window = new AmigaWindow(0, 13, 256, 179, "Debug Output", Gadget::GADGET_DRAGGABLE, 0);
		_screen->addGadget(_window);
		_window->redraw();
	}

	// Create style
	if (_style == NULL) {
		_style = new GadgetStyle();
		_style->colours.back = defaultGadgetStyle->colours.back;
		_style->colours.shine = defaultGadgetStyle->colours.shine;
		_style->colours.highlight = defaultGadgetStyle->colours.highlight;
		_style->colours.shadow = defaultGadgetStyle->colours.shadow;
		_style->colours.fill = defaultGadgetStyle->colours.fill;
		_style->colours.dark = defaultGadgetStyle->colours.dark;
		_style->font = new TinyFont();
		_style->glyphFont = defaultGadgetStyle->glyphFont;
	}

	// Add textbox
	if (_textBox == NULL) {
		Rect rect;
		_window->getClientRect(rect);

		_textBox = new ScrollingTextBox(rect.x, rect.y, rect.width, rect.height, "", Gadget::GADGET_DRAGGABLE, 50, _style);
		_textBox->disableDrawing();
		_window->addGadget(_textBox);
		_textBox->setTextAlignmentHoriz(MultiLineTextBox::TEXT_ALIGNMENT_HORIZ_LEFT);
		_textBox->setTextAlignmentVert(MultiLineTextBox::TEXT_ALIGNMENT_VERT_TOP);
		_textBox->appendText("Woopsi Version ");
		_textBox->appendText(WOOPSI_VERSION);
		_textBox->appendText("\n");
		_textBox->appendText(WOOPSI_COPYRIGHT);
		_textBox->enableDrawing();
		_textBox->appendText("\n");
	}
}
