#ifndef _PIPE_BUTTON_L3_H_
#define _PIPE_BUTTON_L3_H_

#include "pipebuttonstandard.h"
#include "graphicsport.h"
#include "constants.h"

using namespace WoopsiUI;

class PipeButtonL3 : public PipeButtonStandard {
public:
	PipeButtonL3(s16 x, s16 y, u16 width, u16 height) : PipeButtonStandard(x, y, width, height, false, false, true, true) { };
	
protected:
	virtual ~PipeButtonL3() { };
	
	void drawContents(GraphicsPort* port) {

		// Do not draw if contents not revealed
		if (!isRevealed()) return;
		
		Rect rect;
		getClientRect(rect);
		
		u16 colour;
		
		if (isEnabled()) {
			colour = getShadowColour();
		} else {
			colour = getDarkColour();
		}
		
		// Draw flow
		if (!hasAvailableTopConnector()) {
		
			// Flow is moving from bottom to right
			if (getFlowLevel() > MAX_PIPE_BUTTON_FLOW / 2) {
			
				// Fill entire vertical section
				port->drawFilledRect(((rect.width - FLOW_SIZE) / 2) + 1, 0, FLOW_SIZE, (rect.height - FLOW_SIZE) / 2 + FLOW_SIZE, woopsiRGB(0, 0, 20));
				
				// Draw horizontal section
				port->drawFilledRect(((rect.width - FLOW_SIZE) / 2) + 1, ((rect.height - FLOW_SIZE) / 2) + 1, getFlowLevel() - ((rect.width - FLOW_SIZE) / 2) + 1, FLOW_SIZE, woopsiRGB(0, 0, 20));
			} else {
			
				// Draw part of vertical section
				port->drawFilledRect(((rect.width - FLOW_SIZE) / 2) + 1, 0, FLOW_SIZE, getFlowLevel(), woopsiRGB(0, 0, 20));
			}
		} else {
		
			// Flow is moving from right to bottom
			if (getFlowLevel() > MAX_PIPE_BUTTON_FLOW / 2) {
			
				// Fill entire horizontal section
				port->drawFilledRect(_width - getFlowLevel() - 1, 3, getFlowLevel(), rect.height - 7, woopsiRGB(0, 0, 20));
			} else {
			
				// Draw part of horizontal section
			}
		}
		
		port->drawVertLine((rect.width + FLOW_SIZE) / 2, (rect.height - FLOW_SIZE) / 2, (rect.height + FLOW_SIZE) / 2, colour);
		port->drawVertLine((rect.width - FLOW_SIZE) / 2, (rect.height + FLOW_SIZE) / 2, (rect.height - FLOW_SIZE) / 2, colour);

		port->drawHorizLine(0, (rect.height - FLOW_SIZE) / 2, (rect.width + FLOW_SIZE) / 2, colour);
		port->drawHorizLine(0, (rect.height + FLOW_SIZE) / 2, (rect.width - FLOW_SIZE) / 2, colour);
	};
};

#endif