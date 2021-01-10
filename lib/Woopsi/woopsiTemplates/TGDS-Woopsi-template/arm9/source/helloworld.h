#ifndef _HELLO_WORLD_H_
#define _HELLO_WORLD_H_

#include "woopsi.h"
#include "alert.h"

using namespace WoopsiUI;

class HelloWorld : public Woopsi {
public:
	void startup();
	void shutdown();
private:
	Alert* _alert;
};

#endif
