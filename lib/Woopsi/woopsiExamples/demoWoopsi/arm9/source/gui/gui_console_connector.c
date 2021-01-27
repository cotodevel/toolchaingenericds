/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA
*/

//This file abstracts specific TGDS console code which allows for easy DS console setup.

#include "gui_console_connector.h"

////////[For custom Console implementation]:////////
//You need to override :
	//vramSetup * getProjectSpecificVRAMSetup()
	//Which provides a proper custom 2D VRAM setup

//Then override :
	//bool InitProjectSpecificConsole()
	//Which provides the console init code, example not available here, checkout projects that support Custom console implementation.

//After that you can call :
	//bool project_specific_console = true;
	//GUI_init(project_specific_console);


////////[For default Console implementation simply call]:////////
	//bool project_specific_console = false;
	//GUI_init(project_specific_console);





	////////[Default Console implementation is selected, thus stubs are implemented here]////////


//Definition that overrides the weaksymbol expected from toolchain to init console video subsystem
vramSetup * getProjectSpecificVRAMSetup(){
	return NULL;
}


//2) Uses subEngine: VRAM Layout -> Console Setup
bool InitProjectSpecificConsole(){
	DefaultSessionConsole = (ConsoleInstance *)(&CustomConsole);
	InitializeConsole(DefaultSessionConsole);
	return true;
}