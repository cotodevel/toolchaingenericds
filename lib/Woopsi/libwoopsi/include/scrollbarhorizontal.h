#ifndef _SCROLLBAR_HORIZONTAL_H_
#define _SCROLLBAR_HORIZONTAL_H_

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "gadget.h"
#include "gadgeteventhandler.h"
#include "gadgetstyle.h"
#include "sliderbase.h"

namespace WoopsiUI {

	class SliderHorizontal;
	class Button;
	class WoopsiTimer;

	/**
	 * Container class that holds a slider gadget and two arrow buttons.
	 * The interface is presents is virtually identical to the SliderHorizontal
	 * gadget, which means the two are easily interchangeable.  All events
	 * raised by the internal slider gadget are re-raised by this gadget
	 * to this gadget's event handler, meaning its events are also identical
	 * to the SliderHorizontal's.
	 */
	class ScrollbarHorizontal : public SliderBase, public Gadget, public GadgetEventHandler {

	public:

		/**
		 * Constructor.
		 * @param x The x co-ord of the slider, relative to its parent.
		 * @param y The y co-ord of the slider, relative to its parent.
		 * @param width The width of the slider.
		 * @param height The height of the slider.
		 * @param style The style that the gadget should use.  If this is not
		 * specified, the gadget will use the values stored in the global
		 * defaultGadgetStyle object.  The gadget will copy the properties of
		 * the style into its own internal style object.
		 */
		ScrollbarHorizontal(s16 x, s16 y, u16 width, u16 height, GadgetStyle* style = NULL);

		/**
		 * Get the smallest value that the slider can represent.
		 * @return The smallest value.
		 */
		const s16 getMinimumValue() const;

		/**
		 * Get the largest value that the slider can represent.
		 * @return The largest value.
		 */
		const s16 getMaximumValue() const;

		/**
		 * Get the current value of the slider.
		 * return The current slider value.
		 */
		const s16 getValue() const;

		/**
		 * Get the value represented by the height of the grip.
		 * For sliders, this would typically be 1 (so each new
		 * grip position is worth 1).  For scrollbars, this
		 * would be the height of the scrolling gadget.
		 * @return The page size.
		 */
		const s16 getPageSize() const;

		/**
		 * Set the smallest value that the slider can represent.
		 * @param value The smallest value.
		 */
		void setMinimumValue(const s16 value);

		/**
		 * Set the largest value that the slider can represent.
		 * @param value The largest value.
		 */
		void setMaximumValue(const s16 value);

		/**
		 * Set the value that of the slider.  This will reposition
		 * and redraw the grip.
		 * @param value The new value.
		 */
		void setValue(const s16 value);

		/**
		 * Set the value that of the slider.  This will reposition and redraw
		 * the grip.  The supplied value should be bitshifted left 16 places.
		 * This ensures greater accuracy than the standard setValue() method if
		 * the slider is being used as a scrollbar.
		 * @param value The new value.
		 */
		void setValueWithBitshift(const s32 value);

		/**
		 * Set the page size represented by the grip.
		 * @param pageSize The page size.
		 * @see getPageSize().
		 */
		void setPageSize(const s16 pageSize);

		/**	
		 * Process events fired by the grip.
		 * @param e The event details.
		 */
		virtual void handleActionEvent(const GadgetEventArgs& e);

		/**
		 * Process events fired by the grip.
		 * @param e The event details.
		 */
		virtual void handleClickEvent(const GadgetEventArgs& e);

		/**
		 * Process events fired by the grip.
		 * @param e The event details.
		 */
		virtual void handleReleaseEvent(const GadgetEventArgs& e);

		/**
		 * Process events fired by the grip.
		 * @param e The event details.
		 */
		virtual void handleReleaseOutsideEvent(const GadgetEventArgs& e);

		/**
		 * Process events fired by the grip.
		 * @param e The event details.
		 */
		virtual void handleValueChangeEvent(const GadgetEventArgs& e);

	protected:
		SliderHorizontal* _slider;					/**< Pointer to the slider gadget */
		Button* _leftButton;						/**< Pointer to the left button */
		Button* _rightButton;						/**< Pointer to the right button */
		u8 _buttonWidth;							/**< Width of the buttons */
		u8 _scrollTimeout;							/**< VBLs needed until a button triggers another grip movement */
		WoopsiTimer* _timer;						/**< Controls slider button repeats */

		/**
		 * Resize the scrollbar to the new dimensions.
		 * @param width The new width.
		 * @param height The new height.
		 */
		virtual void onResize(u16 width, u16 height);
		
		/**
		 * Destructor.
		 */
		virtual inline ~ScrollbarHorizontal() { };

		/**
		 * Copy constructor is protected to prevent usage.
		 */
		inline ScrollbarHorizontal(const ScrollbarHorizontal& scrollbarHorizontal) : Gadget(scrollbarHorizontal) { };
	};
}

#endif
