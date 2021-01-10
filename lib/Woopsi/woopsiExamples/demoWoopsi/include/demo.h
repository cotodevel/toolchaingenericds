#ifndef _DEMO_H_
#define _DEMO_H_

#include "woopsi.h"
#include "alert.h"

#define DEMO_VERSION "Woopsi Demo V0.99.3 Beta"

using namespace WoopsiUI;

class PacMan;
class Calculator;
class Pong;

class Demo : public Woopsi {
public:
	void startup();
	void shutdown();

private:
	PacMan* _pacMan;
	Calculator* _calculator;
	Pong* _pong;
	Alert* _alert;
};

#endif
