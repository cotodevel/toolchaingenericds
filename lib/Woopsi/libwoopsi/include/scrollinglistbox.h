#ifndef _SCROLLING_LISTBOX_H_
#define _SCROLLING_LISTBOX_H_

#include "gadget.h"
#include "listbox.h"
#include "gadgeteventhandler.h"
#include "listdata.h"
#include "listboxdataitem.h"
#include "gadgetstyle.h"
#include "listboxbase.h"

namespace WoopsiUI {

	class ScrollbarVertical;

	/**
	 * Gadget containing a ListBox and a vertical scrollbar.  Exposed
	 * methods are more or less identical to the methods exposed by the ListBox
	 * to ensure that the two are interchangeable.
	 */
	class ScrollingListBox : public ListBoxBase, public Gadget, public GadgetEventHandler {
	public:

		/**
		 * Constructor.
		 * @param x The x co-ordinate of the gadget.
		 * @param y The y co-ordinate of the gadget.
		 * @param width The width of the gadget.
		 * @param height The height of the gadget.
		 * @param style The style that the gadget should use.  If this is not
		 * specified, the gadget will use the values stored in the global
		 * defaultGadgetStyle object.  The gadget will copy the properties of
		 * the style into its own internal style object.
		 */
		ScrollingListBox(s16 x, s16 y, u16 width, u16 height, GadgetStyle* style = NULL);

		/**
		 * Add a new option to the gadget using default colours.
		 * @param text Text to show in the option.
		 * @param value The value of the option.
		 */
		virtual void addOption(const WoopsiString& text, const u32 value);

		/**
		 * Add an option to the gadget.
		 * @param option The option to add.
		 */
		virtual void addOption(ListBoxDataItem* option);

		/**
		 * Add a new option to the gadget.
		 * @param text Text to show in the option.
		 * @param value The value of the option.
		 * @param normalTextColour Colour to draw the text with when not selected.
		 * @param normalBackColour Colour to draw the background with when not selected.
		 * @param selectedTextColour Colour to draw the text with when selected.
		 * @param selectedBackColour Colour to draw the background with when selected.
		 */
		virtual void addOption(const WoopsiString& text, const u32 value, const u16 normalTextColour, const u16 normalBackColour, const u16 selectedTextColour, const u16 selectedBackColour);

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
		 * Select an option by its index.  Does not deselect any other selected options.
		 * Redraws the gadget and raises a value changed event.
		 * @param index The index of the option to select.
		 */
		virtual inline void selectOption(const s32 index) {
			_listbox->selectOption(index);
		};

		/**
		 * Select an option by its index.  Does not deselect any other selected options.
		 * Redraws the gadget and raises a value changed event.
		 * @param index The index of the option to select.
		 */
		virtual inline void deselectOption(const s32 index) {
			_listbox->deselectOption(index);
		};

		/**
		 * Select all options.  Does nothing if the listbox does not allow multiple selections.
		 * Redraws the gadget and raises a value changed event.
		 */
		virtual inline void selectAllOptions() {
			_listbox->selectAllOptions();
		};

		/**
		 * Deselect all options.
		 * Redraws the gadget and raises a value changed event.
		 */
		virtual inline void deselectAllOptions() {
			_listbox->deselectAllOptions();
		};

		/**
		 * Get the selected index.  Returns -1 if nothing is selected.  If more than one
		 * option is selected, the index of the first selected option is returned.
		 * @return The selected index.
		 */
		virtual inline const s32 getSelectedIndex() const {
			return _listbox->getSelectedIndex();
		};

		/**
		 * Sets the selected index.  Specify -1 to select nothing.  Resets any
		 * other selected items to deselected.
		 * Redraws the gadget and raises a value changed event.
		 * @param index The selected index.
		 */
		virtual inline void setSelectedIndex(const s32 index) {
			_listbox->setSelectedIndex(index);
		};

		/**
		 * Get the selected option.  Returns NULL if nothing is selected.
		 * @return The selected option.
		 */
		virtual inline const ListBoxDataItem* getSelectedOption() const {
			return _listbox->getSelectedOption();
		};
		
		/**
		 * Sets whether multiple selections are possible or not.
		 * @param allowMultipleSelections True to allow multiple selections.
		 */
		virtual inline void setAllowMultipleSelections(const bool allowMultipleSelections) {
			_listbox->setAllowMultipleSelections(allowMultipleSelections);
		};

		/**
		 * Sets whether multiple selections are possible or not.
		 * @return True if multiple selections are allowed.
		 */
		virtual inline const bool allowsMultipleSelections() const {
			return _listbox->allowsMultipleSelections();
		};

		/**
		 * Resize the scrolling canvas to encompass all options.
		 */
		virtual inline void resizeCanvas() {
			_listbox->resizeCanvas();
		};

		/**
		 * Get the specified option.
		 * @return The specified option.
		 */
		virtual inline const ListBoxDataItem* getOption(const s32 index) {
			return _listbox->getOption(index);
		};

		/**
		 * Get the selected index.  Returns -1 if nothing is selected.
		 * @return The selected index.
		 */
		virtual inline const ListBoxDataItem* getOption(const s32 index) const {
			return _listbox->getOption(index);
		};

		/**
		 * Sort the options alphabetically by the text of the options.
		 */
		virtual inline void sort() {
			_listbox->sort();
		};

		/**
		 * Get the total number of options.
		 * @return The number of options.
		 */
		virtual inline const s32 getOptionCount() const {
			return _listbox->getOptionCount();
		};

		/**
		 * Get the height of a single option.
		 * @return The height of an option.
		 */
		virtual inline const u16 getOptionHeight() const {
			return _listbox->getOptionHeight();
		};

		/**
		 * Handles events raised by its sub-gadgets.
		 * @param e Event arguments.
		 */
		virtual void handleValueChangeEvent(const GadgetEventArgs& e);

		/**
		 * Handle a gadget action event.
		 * @param e The event data.
		 */
		virtual void handleActionEvent(const GadgetEventArgs& e);

		/**
		 * Handles events raised by its sub-gadgets.
		 * @param e Event arguments.
		 */
		virtual void handleScrollEvent(const GadgetEventArgs& e);

		/**
		 * Handle a stylus click event.
		 * @param e The event data.
		 */
		virtual void handleClickEvent(const GadgetEventArgs& e);

		/**
		 * Handles events raised by its sub-gadgets.
		 * @param e Event arguments.
		 */
		virtual void handleDoubleClickEvent(const GadgetEventArgs& e);

		/**
		 * Handle a stylus release event that occurred within the bounds of
		 * the source gadget.
		 * @param e The event data.
		 */
		virtual void handleReleaseEvent(const GadgetEventArgs& e);

		/**
		 * Handle a stylus release event that occurred outside the bounds of
		 * the source gadget.
		 * @param e The event data.
		 */
		virtual void handleReleaseOutsideEvent(const GadgetEventArgs& e);

		/**
		 * Set the font used in the textbox.
		 * @param font Pointer to the new font.
		 */
		virtual void setFont(FontBase* font);

		/**
		 * Sets whether or not items added to the list are automatically sorted on insert or not.
		 * @param sortInsertedItems True to enable sort on insertion.
		 */
		virtual inline void setSortInsertedItems(const bool sortInsertedItems) { _listbox->setSortInsertedItems(sortInsertedItems); };

		/**
		 * Insert the dimensions that this gadget wants to have into the rect
		 * passed in as a parameter.  All co-ordinates are relative to the gadget's
		 * parent.  Value is based on the length of the largest string in the
		 * set of options.
		 * @param rect Reference to a rect to populate with data.
		 */
		virtual void getPreferredDimensions(Rect& rect) const;

	protected:
		ListBox* _listbox;									/**< Pointer to the list box. */
		ScrollbarVertical* _scrollbar;						/**< Pointer to the scrollbar. */
		u8 _scrollbarWidth;									/**< Width of the scrollbar. */

		/**
		 * Draw the area of this gadget that falls within the clipping region.
		 * Called by the redraw() function to draw all visible regions.
		 * @param port The GraphicsPort to draw to.
		 * @see redraw()
		 */
		virtual void drawContents(GraphicsPort* port);

		/**
		 * Resize the listbox to the new dimensions.
		 * @param width The new width.
		 * @param height The new height.
		 */
		virtual void onResize(u16 width, u16 height);
		
		/**
		 * Destructor.
		 */
		virtual inline ~ScrollingListBox() { };

		/**
		 * Copy constructor is protected to prevent usage.
		 */
		inline ScrollingListBox(const ScrollingListBox& scrollingListBox) : Gadget(scrollingListBox) { };
	};
}

#endif
