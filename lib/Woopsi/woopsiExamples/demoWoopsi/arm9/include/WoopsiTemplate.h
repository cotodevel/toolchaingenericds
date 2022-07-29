#ifndef _DEMO_H_
#define _DEMO_H_

#ifdef __cplusplus
#include "alert.h"
#include "woopsi.h"
#include "woopsiheaders.h"
#include "filerequester.h"
#include "textbox.h"
#include "soundTGDS.h"

#include <string>
using namespace std;
using namespace WoopsiUI;

class PacMan;
class Calculator;
class Pong;

#define DEMO_VERSION "Woopsi Demo V0.99.3 Beta"
class WoopsiTemplate : public Woopsi {
public:
	void startup(int argc, char **argv);
	void shutdown();
	void handleLidClosed();
	void handleLidOpen();
	void ApplicationMainLoop();
private:
	PacMan* _pacMan;
	Calculator* _calculator;
	Pong* _pong;
	Alert* _alert;
};
#endif

#define TGDSPROJECTNAME ((char*) "demoWoopsi")

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern WoopsiTemplate * WoopsiTemplateProc;

#ifdef __cplusplus
}
#endif
