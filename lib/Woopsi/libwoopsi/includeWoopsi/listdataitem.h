#ifndef _LIST_DATA_ITEM_H_
#define _LIST_DATA_ITEM_H_

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "woopsistring.h"

namespace WoopsiUI {

	/**
	 * Class representing a data item within a list.  Intended for use within the ListData
	 * class.
	 */
	class ListDataItem {
	public:

		/**
		 * Constructor.
		 * @param text The text to display in the item.
		 * @param value The value of the item.
		 */
		ListDataItem(const WoopsiString& text, const u32 value);
		
		/**
		 * Destructor.
		 */
		virtual ~ListDataItem();

		/**
		 * Get the item's text.
		 * @return The item's text.
		 */
		inline const WoopsiString& getText() const { return _text; };

		/**
		 * Get the item's value.
		 * @return The item's value.
		 */
		inline const u32 getValue() const { return _value; };

		/**
		 * Get the item's selection state.
		 * @return True if the item is selected; false if not.
		 */
		inline const bool isSelected() const { return _isSelected; };

		/**
		 * Set the item's selection state.
		 * @param selected True to select the item; false to deselect it.
		 */
		inline void setSelected(bool selected) { _isSelected = selected; };

		/**
		 * Compare the item with another.  Comparison is based on the text of
		 * the item.  Returns 0 if the text in the two items is the same,
		 * a value less than 0 if this item is less than the argument, and
		 * a value greater than 0 if this item is greater than the argument.
		 * @param item An item to compare this object with.
		 * @return 0 if the text in the two items is the same,
		 * a value less than 0 if this item is less than the argument, and
		 * a value greater than 0 if this item is greater than the argument.
		 */
		virtual s8 compareTo(const ListDataItem* item) const;

	private:
		WoopsiString _text;				/**< Text to display for option. */
		u32 _value;						/**< Option value. */
		bool _isSelected;				/**< True if the option is selected. */
	};
}

#endif
