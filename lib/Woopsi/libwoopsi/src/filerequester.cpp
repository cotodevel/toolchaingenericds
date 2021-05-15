#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "filerequester.h"
#include "button.h"
#include "filepath.h"

using namespace WoopsiUI;

FileRequester::FileRequester(s16 x, s16 y, u16 width, u16 height, const WoopsiString& title, const WoopsiString& path, u32 flags, GadgetStyle* style) : AmigaWindow(x, y, width, height, title, flags, AMIGA_WINDOW_SHOW_DEPTH, style) {

	// Increase the size of the border to leave space between gadgets and the 
	// border decorations
	_borderSize.top += 2;
	_borderSize.right += 2;
	_borderSize.bottom += 2;
	_borderSize.left += 2;

	Rect rect;
	getClientRect(rect);

	// Create OK button
	_okButton = new Button(0, 0, 0, 0, "OK");

	Rect buttonRect;
	_okButton->getPreferredDimensions(buttonRect);

	// Calculate OK button dimensions
	buttonRect.width = (rect.width >> 1) - 1;
	buttonRect.x = rect.x;
	buttonRect.y = (rect.y + rect.height) - buttonRect.height;

	_okButton->changeDimensions(buttonRect.x, buttonRect.y, buttonRect.width, buttonRect.height);

	_okButton->addGadgetEventHandler(this);
	addGadget(_okButton);

	// Calculate cancel button dimensions
	buttonRect.x = rect.x + rect.width - buttonRect.width;
	buttonRect.y = (rect.y + rect.height) - buttonRect.height;

	// Create cancel button
	_cancelButton = new Button(buttonRect.x, buttonRect.y, buttonRect.width, buttonRect.height, "Cancel");
	_cancelButton->addGadgetEventHandler(this);
	addGadget(_cancelButton);

	// Calculate list box
	Rect listboxRect;
	listboxRect.width = rect.width;
	listboxRect.height = rect.height - buttonRect.height - 2;
	listboxRect.x = rect.x;
	listboxRect.y = rect.y;

	// Create list box
	_listbox = new FileListBox(listboxRect.x, listboxRect.y, listboxRect.width, listboxRect.height, 0, style);
	_listbox->addGadgetEventHandler(this);
	_listbox->setAllowMultipleSelections(false);
	_listbox->setPath(path);
	addGadget(_listbox);
}

void FileRequester::onResize(u16 width, u16 height) {

	// Call base class' method to ensure the basic window resizes correctly
	AmigaWindow::onResize(width, height);

	Rect rect;
	getClientRect(rect);

	// Calculate OK button dimensions
	Rect buttonRect;
	_okButton->getPreferredDimensions(buttonRect);

	buttonRect.width = (rect.width >> 1) - 1;
	buttonRect.x = rect.x;
	buttonRect.y = (rect.y + rect.height) - buttonRect.height;

	_okButton->changeDimensions(buttonRect.x, buttonRect.y, buttonRect.width, buttonRect.height);

	// Calculate cancel button dimensions
	buttonRect.x = rect.x + rect.width - buttonRect.width;
	buttonRect.y = (rect.y + rect.height) - buttonRect.height;

	_cancelButton->changeDimensions(buttonRect.x, buttonRect.y, buttonRect.width, buttonRect.height);

	// Calculate list box dimensions
	Rect listboxRect;
	listboxRect.width = rect.width;
	listboxRect.height = rect.height - buttonRect.height - 2;
	listboxRect.x = rect.x;
	listboxRect.y = rect.y;

	_listbox->changeDimensions(listboxRect.x, listboxRect.y, listboxRect.width, listboxRect.height);
}

void FileRequester::handleReleaseEvent(const GadgetEventArgs& e) {
	if (e.getSource() != NULL) {
		if (e.getSource() == _cancelButton) {

			// Close the window
			//close();
			return;
		} else if (e.getSource() == _okButton) {

			// Raise value changed event to event handler
			_gadgetEventHandlers->raiseValueChangeEvent();

			// Close the window
			//close();
			return;
		}
	}
	
	//AmigaWindow::handleReleaseEvent(e);
}

void FileRequester::handleValueChangeEvent(const GadgetEventArgs& e) {
	if (e.getSource() != NULL) {
		if (e.getSource() == _listbox) {

			// File selected; raise event
			//_gadgetEventHandlers->raiseValueChangeEvent();

			// Close the window
			//close();
		}
	}
}

void FileRequester::setPath(const char* path) {
	_listbox->setPath(path);
}

void FileRequester::appendPath(const char* path) {
	_listbox->appendPath(path);
}

const FilePath* FileRequester::getPath() const {
	return _listbox->getPath();
}