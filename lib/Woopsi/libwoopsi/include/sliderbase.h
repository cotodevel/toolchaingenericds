#ifndef _SLIDER_BASE_H_
#define _SLIDER_BASE_H_

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

namespace WoopsiUI {

	/**
	 * Defines the interface for slider gadgets.
	 */
	class SliderBase {

	public:

		/**
		 * Get the smallest value that the slider can represent.
		 * @return The smallest value.
		 */
		virtual const s16 getMinimumValue() const = 0;

		/**
		 * Get the largest value that the slider can represent.
		 * @return The largest value.
		 */
		virtual const s16 getMaximumValue() const = 0;

		/**
		 * Get the current value of the slider.
		 * return The current slider value.
		 */
		virtual const s16 getValue() const = 0;

		/**
		 * Get the value represented by the height of the grip.
		 * For sliders, this would typically be 1 (so each new
		 * grip position is worth 1).  For scrollbars, this
		 * would be the height of the scrolling gadget.
		 * @return The page size.
		 */
		virtual const s16 getPageSize() const = 0;

		/**
		 * Set the smallest value that the slider can represent.
		 * @param value The smallest value.
		 */
		virtual void setMinimumValue(const s16 value) = 0;

		/**
		 * Set the largest value that the slider can represent.
		 * @param value The largest value.
		 */
		virtual void setMaximumValue(const s16 value) = 0;

		/**
		 * Set the value that of the slider.  This will reposition
		 * and redraw the grip.
		 * @param value The new value.
		 */
		virtual void setValue(const s16 value) = 0;

		/**
		 * Set the value that of the slider.  This will reposition and redraw
		 * the grip.  The supplied value should be bitshifted left 16 places.
		 * This ensures greater accuracy than the standard setValue() method if
		 * the slider is being used as a scrollbar.
		 * @param value The new value.
		 */
		virtual void setValueWithBitshift(const s32 value) = 0;

		/**
		 * Set the page size represented by the grip.
		 * @param pageSize The page size.
		 * @see getPageSize().
		 */
		virtual void setPageSize(const s16 pageSize) = 0;
	};
}

#endif
