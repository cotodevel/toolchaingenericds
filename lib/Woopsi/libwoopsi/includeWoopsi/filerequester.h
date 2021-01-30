#ifndef _FILE_REQUESTER_H_
#define _FILE_REQUESTER_H_

#include "amigawindow.h"
#include "listdata.h"
#include "filelistboxdataitem.h"
#include "filelistbox.h"
#include "gadgetstyle.h"

namespace WoopsiUI {

	class Button;
	class FilePath;

	/**
	 * Class providing a window containing a file listbox, an OK button and a
	 * Cancel button.
	 * Designed to allow users selection a file from the filesytem.  When a file
	 * is selected the requester will automatically close.
	 *
	 * To read the value of the selected option or options, you should listen
	 * for the value changed event.  This will fire when the user double-clicks
	 * an option or clicks the OK button.
	 *
	 * To add this class to a project:
	 * - Enable libfat in the makefile by changing the line that reads
	 *      LIBS	:= -lnds9
	 *   to
	 *      LIBS	:= -lfat -lnds9
	 * - Call "fatInitDefault();" somewhere in your setup code.
	 *
	 * Note that including libfat increases the ROM size by ~100K.  Also note
	 * that this code is not compatible with the SDL build of Woopsi.
	 */
	class FileRequester : public AmigaWindow {
	public:

		/**
		 * Constructor.
		 * @param x The x co-ordinate of the window.
		 * @param y The y co-ordinate of the window.
		 * @param width The width of the window.
		 * @param height The height of the window.
		 * @param title The title of the window.
		 * @param path The initial path that the requester will show.
		 * @param flags Standard flags.
		 * @param style Optional gadget style.
		 */
		FileRequester(s16 x, s16 y, u16 width, u16 height, const WoopsiString& title, const WoopsiString& path, u32 flags, GadgetStyle* style = NULL);

		/**
		 * Handles events raised by its sub-gadgets.
		 * @param e Event arguments.
		 */
		virtual void handleReleaseEvent(const GadgetEventArgs& e);
	
		/**
		 * Handles events raised by its sub-gadgets.
		 * @param e Event arguments.
		 */
		virtual void handleValueChangeEvent(const GadgetEventArgs& e);

		/**
		 * Add a new option to the gadget using default colours.  Does not redraw the gadget.
		 * @param text Text to show in the option.
		 * @param value The value of the option.
		 */
		virtual inline void addOption(const char* text, const u32 value) {
			_listbox->addOption(text, value);
		};

		/**
		 * Add a new option to the gadget.  Does not redraw the gadget.
		 * @param text Text to show in the option.
		 * @param value The value of the option.
		 * @param normalTextColour Colour to draw the text with when not selected.
		 * @param normalBackColour Colour to draw the background with when not selected.
		 * @param selectedTextColour Colour to draw the text with when selected.
		 * @param selectedBackColour Colour to draw the background with when selected.
		 */
		virtual inline void addOption(const char* text, const u32 value, const u16 normalTextColour, const u16 normalBackColour, const u16 selectedTextColour, const u16 selectedBackColour) {
			_listbox->addOption(text, value, normalTextColour, normalBackColour, selectedTextColour, selectedBackColour);
		};

		/**
		 * Remove an option from the gadget by its index.  Does not redraw the gadget.
		 * @param index The index of the option to remove.
		 */
		virtual inline void removeOption(const s32 index) {
			_listbox->removeOption(index);
		};

		/**
		 * Remove all options from the gadget.  Does not redraw the gadget.
		 */
		virtual inline void removeAllOptions() {
			_listbox->removeAllOptions();
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
		 * Get the selected option.  Returns NULL if nothing is selected.
		 * @return The selected option.
		 */
		virtual inline const FileListBoxDataItem* getSelectedOption() const {
			return _listbox->getSelectedOption();
		};

		/**
		 * Sets whether multiple selections are possible or not.
		 * Does not redraw the gadget.
		 * @param allowMultipleSelections True to allow multiple selections.
		 */
		virtual inline void setAllowMultipleSelections(const bool allowMultipleSelections) {
			_listbox->setAllowMultipleSelections(allowMultipleSelections);
		};

		/**
		 * Get the specified option.
		 * @return The specified option.
		 */
		virtual inline const FileListBoxDataItem* getOption(const s32 index) const {
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
		 * Expose the listBox object.
		 * @return the listBox object.
		 */
		virtual inline FileListBox* getInternalListBoxObject() const {
			return _listbox;
		};
		
		/**
		 * Set the displayed path.
		 * @param path The new path.
		 */
		virtual void setPath(const char* path);

		/**
		 * Append a new path component to the current path.  Automatically
		 * inserts trailing slash.
		 */
		virtual void appendPath(const char* path);

		/**
		 * Get the current path.
		 */
		virtual const FilePath* getPath() const;

	protected:
		Button* _okButton;					/**< Pointer to the OK button */
		Button* _cancelButton;				/**< Pointer to the cancel button */
		FileListBox* _listbox;				/**< Pointer to the list box */

		/**
		 * Resize the textbox to the new dimensions.
		 * @param width The new width.
		 * @param height The new height.
		 */
		virtual void onResize(u16 width, u16 height);
		
		/**
		 * Destructor.
		 */
		virtual ~FileRequester() { };
		
		/**
		 * Copy constructor is protected to prevent usage.
		 */
		inline FileRequester(const FileRequester& fileRequester) : AmigaWindow(fileRequester) { };
	};
}

#endif
