#ifndef _CHECKBOX_H_
#define _CHECKBOX_H_

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "button.h"
#include "gadgetstyle.h"

namespace WoopsiUI {

	/**
	 * Class representing a radio button.  Like radio buttons, checkboxes
	 * are tri-state - off, on and "mu".  The mu state cannot be enabled by
	 * a user - it can only be set by the developer.
	 */
	class CheckBox : public Button {

	public:
		/**
		 * Enum listing all possible checkbox states.
		 */
		enum CheckBoxState {
			CHECK_BOX_STATE_OFF = 0,			/**< Checkbox is unticked */
			CHECK_BOX_STATE_ON = 1,				/**< Checkbox is ticked */
			CHECK_BOX_STATE_MU = 2				/**< Checkbox is in the third state */
		};

		/**
		 * Constructor.
		 * @param x The x co-ordinate of the checkbox, relative to its parent.
		 * @param y The y co-ordinate of the checkbox, relative to its parent.
		 * @param width The width of the checkbox.
		 * @param height The height of the checkbox.
		 * @param style The style that the gadget should use.  If this is not
		 * specified, the gadget will use the values stored in the global
		 * defaultGadgetStyle object.  The gadget will copy the properties of
		 * the style into its own internal style object.
		 */
		CheckBox(s16 x, s16 y, u16 width, u16 height, GadgetStyle* style = NULL);

		/**
		 * Get the current state of the checkbox.
		 * @return The state of the checkbox.
		 */
		virtual inline const CheckBoxState getState() const { return _state; };

		/**
		 * Set the state of the checkbox.
		 * @param state The new checkbox state.
		 */
		virtual void setState(CheckBoxState state);

	protected:
		CheckBoxState _state;				/**< The state of the checkbox */

		/**
		 * Draw the area of this gadget that falls within the clipping region.
		 * Called by the redraw() function to draw all visible regions.
		 * @param port The GraphicsPort to draw to.
		 * @see redraw()
		 */
		virtual void drawContents(GraphicsPort* port);

		/**
		 * Draw the area of this gadget that falls within the clipping region.
		 * Called by the redraw() function to draw all visible regions.
		 * @param port The GraphicsPort to draw to.
		 * @see redraw()
		 */
		virtual void drawBorder(GraphicsPort* port);

		/**
		 * Toggles the state of the checkbox.
		 * @param x The x co-ordinate of the click.
		 * @param y The y co-ordinate of the click.
		 */
		virtual void onClick(s16 x, s16 y);

		/**
		 * Destructor.
		 */
		virtual inline ~CheckBox() { };

		/**
		 * Copy constructor is protected to prevent usage.
		 */
		inline CheckBox(const CheckBox& checkBox) : Button(checkBox) { };
	};
}

#endif
