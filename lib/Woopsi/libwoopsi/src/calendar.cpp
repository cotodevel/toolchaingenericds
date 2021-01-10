#include <stdio.h>
#include <stdlib.h>
#include "calendar.h"
#include "date.h"
#include "button.h"
#include "graphicsport.h"
#include "label.h"

using namespace WoopsiUI;

Calendar::Calendar(s16 x, s16 y, u16 width, u16 height, u8 day, u8 month, u16 year, u32 flags, GadgetStyle* style) : Gadget(x, y, width, height, flags, style) {
	_date = NULL;
	_visibleDate = NULL;
	_selectedDayButton = NULL;

	buildGUI();

	setDate(day, month, year);
}

Calendar::~Calendar() {
	delete _date;
}

void Calendar::drawContents(GraphicsPort* port) {
	port->drawFilledRect(0, 0, getWidth(), getHeight(), getBackColour());
}

void Calendar::drawBorder(GraphicsPort* port) {

	// Stop drawing if the gadget indicates it should not have an outline
	if (isBorderless()) return;

	port->drawBevelledRect(0, 0, getWidth(), getHeight(), getShineColour(), getShadowColour());
}

void Calendar::handleReleaseEvent(const GadgetEventArgs& e) {

	if (e.getSource() != NULL) {

		if (e.getSource() == _leftArrow) {

			// Handle left arrow release
			_visibleDate->addMonths(-1);
			populateGUI();

		} else if (e.getSource() == _rightArrow) {

			// Handle right arrow release
			_visibleDate->addMonths(1);
			populateGUI();

		} else if (e.getSource()->getRefcon() > 0) {

			// Handle day button release

			// Calculate the new date

			// Extract raw char data; this could be much more efficient
			char string[((Button*)e.getSource())->getText().getByteCount()];
			((Button*)e.getSource())->getText().copyToCharArray(string);

			u8 day = atoi(string);
			Date* newDate = new Date(day, _visibleDate->getMonth(), _visibleDate->getYear());

			// Prevent changes if new date is same as old
			if (*_date != *newDate) {

				_date->setDate(day, _visibleDate->getMonth(), _visibleDate->getYear());

				// Select the new gadget and deselect the old
				StickyButton* dayButton = (StickyButton*)e.getSource();
				dayButton->setStuckDown(true);
				dayButton->redraw();

				if (_selectedDayButton != NULL) {
					_selectedDayButton->setStuckDown(false);
					_selectedDayButton->redraw();
				}

				_selectedDayButton = dayButton;

				// Raise an action event
				_gadgetEventHandlers->raiseActionEvent();
			}

			delete newDate;
		}
	}
}

void Calendar::setDate(u8 day, u8 month, u16 year) {
	
	// Initialise date class
	if (_date == NULL) {
		_date = new Date(day, month, year);
		_visibleDate = new Date(day, month, year);
	} else {
		_date->setDate(day, month, year);
		_visibleDate->setDate(day, month, year);
	}

	populateGUI();
}

void Calendar::populateGUI() {

	// Buffer for int-to-string conversions
	char buffer[6];

	// Set month label
	_monthLabel->disableDrawing();
	_monthLabel->setText(_visibleDate->getMonthName());

	// Append year to month name label
	sprintf(buffer, " %d", _visibleDate->getYear());
	_monthLabel->enableDrawing();
	_monthLabel->appendText(buffer);

	u8 buttonIndex = 0;
	StickyButton* button;

	// Reset the selected day button
	_selectedDayButton = NULL;

	// Locate the first button in the gadget array
	for (u8 i = _decorationCount; i < _gadgets.size(); i++) {
		if (_gadgets[i]->getRefcon() > 0) {
			buttonIndex = i;
			break;
		}
	}

	// Get date object for start of this month
	Date* thisMonth = new Date(1, _visibleDate->getMonth(), _visibleDate->getYear());

	// Prepare to update grid of days
	u8 updatedDays = 0;
	u8 maxDays = CALENDAR_BODY_ROWS * CALENDAR_COLS;
	u8 startDay = 1;

	if (thisMonth->getWeekDay() != 1) {

		// Get start date of previous month
		Date* previousMonth = new Date(1, _visibleDate->getMonth(), _visibleDate->getYear());
		previousMonth->addMonths(-1);

		// Calculate calendar start date
		startDay = (previousMonth->getMonthDays() - thisMonth->getWeekDay()) + 2;

		// Set buttons
		while (startDay <= previousMonth->getMonthDays()) {
		
			sprintf(buffer, "%d", startDay);

			button = (StickyButton*)_gadgets[buttonIndex];
			button->disable();
			button->setStuckDown(false);
			button->setText(buffer);

			startDay++;
			updatedDays++;
			buttonIndex++;
		}

		// Reset start day
		startDay = 1;

		delete previousMonth;
	}

	// Set all buttons for this month
	while (startDay <= thisMonth->getMonthDays()) {
		sprintf(buffer, "%d", startDay);

		button = (StickyButton*)_gadgets[buttonIndex];
		button->enable();
		button->setStuckDown(false);

		// Select the day if necessary
		if (_visibleDate->getYear() == _date->getYear()) {
			if (_visibleDate->getMonth() == _date->getMonth()) {
				if (_date->getDay() == startDay) {
					button->setStuckDown(true);
					_selectedDayButton = button;
				}
			}
		}

		button->setText(buffer);

		startDay++;
		updatedDays++;
		buttonIndex++;
	}

	// Set all buttons for following month
	startDay = 1;
	while (updatedDays < maxDays) {
		sprintf(buffer, "%d", startDay);

		button = (StickyButton*)_gadgets[buttonIndex];
		button->disable();
		button->setStuckDown(false);
		button->setText(buffer);

		startDay++;
		updatedDays++;
		buttonIndex++;
	}

	delete thisMonth;
}

void Calendar::calculateColumnWidths(s32 spaceWidth, u8* columnWidths) {

	// Calculate the width of a single column, disregarding remainder
	u8 columnWidth = spaceWidth / CALENDAR_COLS;

	// Calculate sizes of each column - automatically assigns any
	// remainder in integer division of rect width 1px at a time to
	// columns in first come, first served fashion
	s32 remainderWidth = spaceWidth % CALENDAR_COLS;
	for (u8 i = 0; i < CALENDAR_COLS; ++i) {
		columnWidths[i] = columnWidth;

		if (remainderWidth > 0) {
			columnWidths[i]++;
			remainderWidth--;
		}
	}
}

void Calendar::calculateColumnHeights(s32 spaceHeight, u8* columnHeights) {

	// Calculate the height of a single column, disregarding remainder
	u8 columnHeight = spaceHeight / CALENDAR_ROWS;

	// Calculate sizes of each column - automatically assigns any
	// remainder in integer division of rect height 1px at a time to
	// columns in first come, first served fashion
	s32 remainderHeight = spaceHeight % CALENDAR_ROWS;
	for (u8 i = 0; i < CALENDAR_ROWS; ++i) {
		columnHeights[i] = columnHeight;

		if (remainderHeight > 0) {
			columnHeights[i]++;
			remainderHeight--;
		}
	}
}

void Calendar::buildGUI() {

	Rect rect;
	getClientRect(rect);

	// Get the sizes of the columns
	u8 columnWidths[CALENDAR_COLS];
	u8 columnHeights[CALENDAR_ROWS];
	calculateColumnWidths(rect.width, columnWidths);
	calculateColumnHeights(rect.height, columnHeights);

	// Add arrows and month label
	_leftArrow = new Button(rect.x, rect.y, columnWidths[0], columnHeights[0], GLYPH_ARROW_LEFT);
	_leftArrow->setFont(getGlyphFont());
	_leftArrow->addGadgetEventHandler(this);
	addGadget(_leftArrow);

	_rightArrow = new Button((rect.width - columnWidths[CALENDAR_COLS - 1]) + 1, rect.y, columnWidths[CALENDAR_COLS - 1], columnHeights[0], GLYPH_ARROW_RIGHT);
	_rightArrow->setFont(getGlyphFont());
	_rightArrow->addGadgetEventHandler(this);
	addGadget(_rightArrow);

	// Month name
	_monthLabel = new Label(rect.x + columnWidths[0], rect.y, rect.width - (columnWidths[0] + columnWidths[CALENDAR_COLS - 1]), columnHeights[0], "");
	_monthLabel->setBorderless(true);
	addGadget(_monthLabel);

	// Add day labels
	Label* label;
	WoopsiString dayInitials[7] = {"M", "T", "W", "T", "F", "S", "S"};
	s16 labelX = rect.x;

	for (u8 i = 0; i < CALENDAR_COLS; ++i) {
		label = new Label(labelX, rect.y + columnHeights[0], columnWidths[i], columnHeights[1], dayInitials[i]);
		label->setBorderless(true);
		addGadget(label);
		
		labelX += columnWidths[i];
	}

	// Prepare to build grid of days
	u8 allocatedDays = 0;
	u8 maxDays = CALENDAR_BODY_ROWS * CALENDAR_COLS;

	// Create all boxes for this month
	StickyButton* button;
	u16 buttonX = rect.x;
	u16 buttonY = rect.y + columnHeights[0] + columnHeights[1];
	u8 buttonRow = 2;
	while (allocatedDays < maxDays) {
		button = new StickyButton(buttonX, buttonY, columnWidths[allocatedDays % CALENDAR_COLS], columnHeights[buttonRow], "");
		button->addGadgetEventHandler(this);
		button->setRefcon(allocatedDays + 1);

		// Calculate x pos of next button
		buttonX += columnWidths[allocatedDays % CALENDAR_COLS];

		addGadget(button);
		allocatedDays++;

		// Reset x pos of next button if moving to next row
		if (allocatedDays % CALENDAR_COLS == 0) {
			buttonX = rect.x;
			buttonY += columnHeights[buttonRow];
			buttonRow++;
		}
	}
}

const u8 Calendar::getDay() const { return _date->getDay(); }

const u8 Calendar::getMonth() const { return _date->getMonth(); }

const u16 Calendar::getYear() const { return _date->getYear(); }

void Calendar::onResize(u16 width, u16 height) {

	// Get a rect describing the gadget
	Rect rect;
	getClientRect(rect);

	// Get the sizes of the columns
	u8 columnWidths[CALENDAR_COLS];
	u8 columnHeights[CALENDAR_ROWS];
	calculateColumnWidths(rect.width, columnWidths);
	calculateColumnHeights(rect.height, columnHeights);

	// Resize arrows
	_leftArrow->changeDimensions(rect.x, rect.y, columnWidths[0], columnHeights[0]);
	_rightArrow->changeDimensions((rect.width - columnWidths[CALENDAR_COLS - 1]) + 1, rect.y, columnWidths[CALENDAR_COLS - 1], columnHeights[0]);

	// Resize month name
	_monthLabel->changeDimensions(rect.x + columnWidths[0], rect.y, rect.width - (columnWidths[0] + columnWidths[CALENDAR_COLS - 1]), columnHeights[0]);

	// Resize day labels

	// Locate first day label - work on the assumption that this will always
	// be the first gadget after the month label
	s32 gadgetIndex = getGadgetIndex(_monthLabel) + 1;
	s16 labelX = rect.x;

	for (u8 i = 0; i < CALENDAR_COLS; ++i) {
		_gadgets[gadgetIndex]->changeDimensions(labelX, rect.y + columnHeights[0], columnWidths[i], columnHeights[1]);
		gadgetIndex++;

		labelX += columnWidths[i];
	}

	// Resize day buttons
	u8 allocatedDays = 0;
	u8 maxDays = CALENDAR_BODY_ROWS * CALENDAR_COLS;

	// Resize all boxes for this month
	u16 buttonX = rect.x;
	u16 buttonY = rect.y + columnHeights[0] + columnHeights[1];
	u8 buttonRow = 2;
	while (allocatedDays < maxDays) {
		_gadgets[gadgetIndex]->changeDimensions(buttonX, buttonY, columnWidths[allocatedDays % CALENDAR_COLS], columnHeights[buttonRow]);

		// Calculate x pos of next button
		buttonX += columnWidths[allocatedDays % CALENDAR_COLS];

		allocatedDays++;

		// Reset x pos of next button if moving to next row
		if (allocatedDays % CALENDAR_COLS == 0) {
			buttonX = rect.x;
			buttonY += columnHeights[buttonRow];
			buttonRow++;
		}

		gadgetIndex++;
	}
}

// Get the preferred dimensions of the gadget
void Calendar::getPreferredDimensions(Rect& rect) const {
	rect.x = _rect.getX();
	rect.y = _rect.getY();
	rect.width = 0;
	rect.height = 0;

	if (!_flags.borderless) {
		rect.width = _borderSize.left + _borderSize.right;
		rect.height = _borderSize.top + _borderSize.bottom;
	}

	rect.width += 160;
	rect.height += 106;
}
