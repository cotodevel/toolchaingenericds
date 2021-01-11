#ifndef _REQUESTER_H_
#define _REQUESTER_H_

#include "amigawindow.h"
#include "scrollinglistbox.h"
#include "listdata.h"
#include "gadgetstyle.h"

namespace WoopsiUI {

	class Button;

	/**
	 * Class providing a window containing a listbox, an OK button and a Cancel button.
	 * Designed to allow users to make a selection from a number of options.  When an
	 * option is selected the requester will automatically close.
	 * To read the value of the selected option or options, you should listen for the
	 * value changed event.  This will fire when the user double-clicks an option or
	 * clicks the OK button.
	 */
	class Requester : public AmigaWindow {
	public:
		/**
		 * Constructor.
		 * @param x The x co-ordinate of the window.
		 * @param y The y co-ordinate of the window.
		 * @param width The width of the window.
		 * @param height The height of the window.
		 * @param title The title of the window.
		 * @param style The style that the gadget should use.  If this is not
		 * specified, the gadget will use the values stored in the global
		 * defaultGadgetStyle object.  The gadget will copy the properties of
		 * the style into its own internal style object.
		 */
		Requester(s16 x, s16 y, u16 width, u16 height, const WoopsiString& title, GadgetStyle* style = NULL);

		/**
		 * Handles events raised by its sub-gadgets.
		 * @param e Event arguments.
		 */
		virtual void handleReleaseEvent(const GadgetEventArgs& e);

		/**
		 * Handles events raised by its sub-gadgets.
		 * @param e Event arguments.
		 */
		virtual void handleDoubleClickEvent(const GadgetEventArgs& e);

		/**
		 * Add a new option to the gadget using default colours.
		 * @param text Text to show in the option.
		 * @param value The value of the option.
		 */
		virtual inline void addOption(const WoopsiString& text, const u32 value) {
			_listbox->addOption(text, value);
		};

		/**
		 * Add a new option to the gadget.
		 * @param text Text to show in the option.
		 * @param value The value of the option.
		 * @param normalTextColour Colour to draw the text with when not selected.
		 * @param normalBackColour Colour to draw the background with when not selected.
		 * @param selectedTextColour Colour to draw the text with when selected.
		 * @param selectedBackColour Colour to draw the background with when selected.
		 */
		virtual inline void addOption(const WoopsiString& text, const u32 value, const u16 normalTextColour, const u16 normalBackColour, const u16 selectedTextColour, const u16 selectedBackColour) {
			_listbox->addOption(text, value, normalTextColour, normalBackColour, selectedTextColour, selectedBackColour);
		};

		/**
		 * Remove an option from the gadget by its index.
		 * @param index The index of the option to remove.
		 */
		virtual inline void removeOption(const s32 index) {
			_listbox->removeOption(index);
		};

		/**
		 * Remove all options from the gadget.
		 */
		virtual inline void removeAllOptions() {
			_listbox->removeAllOptions();
		};

		/**
		 * Get the selected index.  Returns -1 if nothing is selected.  If more than one
		 * option is selected, the index of the first selected option is returned.
		 * @return The selected index.
		 */
		virtual const s32 getSelectedIndex() const {
			return _listbox->getSelectedIndex();
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

	protected:
		Button* _okButton;					/**< Pointer to the OK button. */
		Button* _cancelButton;				/**< Pointer to the cancel button. */
		ScrollingListBox* _listbox;			/**< Pointer to the list box. */

		/**
		 * Resize the requester to the new dimensions.
		 * @param width The new width.
		 * @param height The new height.
		 */
		virtual void onResize(u16 width, u16 height);
		
		/**
		 * Destructor.
		 */
		virtual inline ~Requester() { };

		/**
		 * Copy constructor is protected to prevent usage.
		 */
		inline Requester(const Requester& requester) : AmigaWindow(requester) { };
	};
}

#endif
