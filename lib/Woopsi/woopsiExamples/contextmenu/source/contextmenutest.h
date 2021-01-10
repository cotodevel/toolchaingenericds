#ifndef _CONTEXT_MENU_TEST_H_
#define _CONTEXT_MENU_TEST_H_

#include "woopsi.h"
#include "gadgeteventhandler.h"
#include "multilinetextbox.h"

using namespace WoopsiUI;

class ContextMenuTest : public Woopsi, public GadgetEventHandler {
public:
	void handleContextMenuSelectionEvent(const ContextMenuEventArgs& e);
	
private:
	MultiLineTextBox* _output;
	
	void startup();
	void shutdown();
};

#endif
