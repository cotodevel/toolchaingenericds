#ifndef _CYCLE_BUTTON_H_
#define _CYCLE_BUTTON_H_

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "button.h"
#include "listdataeventhandler.h"
#include "listdata.h"
#include "listdataitem.h"
#include "gadgetstyle.h"
#include "woopsistring.h"

namespace WoopsiUI {

	/**
	 * Cycle button gadget.  Displays text within the button.  Clicking it cycles
	 * through its available options.
	 */
	class CycleButton : public Button, public ListDataEventHandler  {
	public:

		/**
		 * Constructor for cycle buttons.
		 * @param x The x co-ordinate of the button, relative to its parent.
		 * @param y The y co-ordinate of the button, relative to its parent.
		 * @param width The width of the button.
		 * @param height The height of the button.
		 * @param style The style that the button should use.  If this is not
		 * specified, the button will use the values stored in the global
		 * defaultGadgetStyle object.  The button will copy the properties of
		 * the style into its own internal style object.
		 */
		CycleButton(s16 x, s16 y, u16 width, u16 height, GadgetStyle* style = NULL);

		/**
		 * Add a new option to the gadget.
		 * @param text The text of the option.
		 * @param value The value of the option.
		 */
		void addOption(const WoopsiString& text, const u32 value);

		/**
		 * Remove an option from the gadget by its index.
		 * @param index The index of the option to remove.
		 */
		virtual void removeOption(const s32 index);

		/**
		 * Remove all options from the gadget.
		 */
		virtual void removeAllOptions();

		/**
		 * Select an option by its index.
		 * Redraws the gadget and raises a value changed event.
		 * @param index The index of the option to select.
		 */
		virtual void selectOption(const s32 index);

		/**
		 * Get the selected index.  Returns -1 if nothing is selected.  If more than one
		 * option is selected, the index of the first selected option is returned.
		 * @return The selected index.
		 */
		virtual const s32 getSelectedIndex() const;

		/**
		 * Sets the selected index.  Specify -1 to select nothing.  Resets any
		 * other selected options to deselected.
		 * Redraws the gadget and raises a value changed event.
		 * @param index The selected index.
		 */
		virtual void setSelectedIndex(const s32 index);

		/**
		 * Get the selected option.  Returns NULL if nothing is selected.
		 * @return The selected option.
		 */
		virtual const ListDataItem* getSelectedOption() const;

		/**
		 * Get the value of the current option.
		 * @return Value of the current option.
		 */
		inline const u32 getValue() const { return getSelectedOption()->getValue(); };

		/**
		 * Get the specified option.
		 * @return The specified option.
		 */
		virtual inline const ListDataItem* getOption(const s32 index) {
			return _options.getItem(index);
		};

		/**
		 * Sort the options alphabetically by the text of the options.
		 */
		virtual void sort();

		/**
		 * Get the total number of options.
		 * @return The number of options.
		 */
		virtual inline const s32 getOptionCount() const {
			return _options.getItemCount();
		};

		/**
		 * Sets whether or not items added to the list are automatically sorted on insert or not.
		 * @param sortInsertedItems True to enable sort on insertion.
		 */
		virtual inline void setSortInsertedItems(const bool sortInsertedItems) {
			_options.setSortInsertedItems(sortInsertedItems);
		};

		/**
		 * Handles list data changed events.
		 * @param e Event arguments.
		 */
		virtual void handleListDataChangedEvent(const ListDataEventArgs& e);

		/**
		 * Handles list selection changed events.
		 * @param e Event arguments.
		 */
		virtual void handleListDataSelectionChangedEvent(const ListDataEventArgs& e);

		/**
		 * Insert the dimensions that this gadget wants to have into the rect
		 * passed in as a parameter.  All co-ordinates are relative to the gadget's
		 * parent.  Value is based on the length of the largest string in the
		 * set of options.
		 * @param rect Reference to a rect to populate with data.
		 */
		virtual void getPreferredDimensions(Rect& rect) const;

	protected:
		ListData _options;							/**< Option storage. */

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
		 * Draws the outline of the button.
		 * @param port Graphics port to draw to.
		 */
		virtual void drawOutline(GraphicsPort* port);

		/**
		 * Selects the next option in the list and redraws the button.
		 * @param x The x co-ordinate of the stylus.
		 * @param y The y co-ordinate of the stylus.
		 */
		virtual void onRelease(s16 x, s16 y);
		
		/**
		 * Redraws the button.
		 * @param x The x co-ordinate of the stylus.
		 * @param y The y co-ordinate of the stylus.
		 */
		virtual void onReleaseOutside(s16 x, s16 y);

		/**
		 * Prevents the Button onResize() method from recalculating the text
		 * positions by overriding it.
		 * @param width The new width.
		 * @param height The new height.
		 */
		virtual inline void onResize(u16 width, u16 height) { };

		/**
		 * Override method in Label class to prevent recalculation of text positions.
		 */
		virtual inline void calculateTextPosition() { };

		/**
		 * Destructor.
		 */
		virtual ~CycleButton() { };

		/**
		 * Copy constructor is protected to prevent usage.
		 */
		inline CycleButton(const CycleButton& cycleButton) : Button(cycleButton) { };
	};
}

#endif
