#include <string.h>
#include "woopsistring.h"
#include "stringiterator.h"

using namespace WoopsiUI;

WoopsiString::WoopsiString() {
	_text = NULL;
	_dataLength = 0;
	_stringLength = 0;
	_allocatedSize = 0;
	_growAmount = 32;
}

WoopsiString::WoopsiString(const char* text) {
	_text = NULL;
	_dataLength = 0;
	_stringLength = 0;
	_allocatedSize = 0;
	_growAmount = 32;

	setText(text);
}

WoopsiString::WoopsiString(const u32 text) {
	_text = NULL;
	_dataLength = 0;
	_stringLength = 0;
	_allocatedSize = 0;
	_growAmount = 32;

	setText(text);
}

WoopsiString::WoopsiString(const WoopsiString& string) {
	_text = NULL;
	_dataLength = 0;
	_stringLength = 0;
	_allocatedSize = 0;
	_growAmount = 32;

	setText(string);
};

WoopsiString& WoopsiString::operator=(const WoopsiString& string) {
	if (&string != this) {
		setText(string);
	}
	
	return *this;
}

WoopsiString& WoopsiString::operator=(const char* string) {
	setText(string);
	return *this;
}

WoopsiString& WoopsiString::operator=(const u32 letter) {
	setText(letter);
	return *this;
}

StringIterator* WoopsiString::newStringIterator() const {
	return new StringIterator(this);
}

void WoopsiString::setText(const WoopsiString& text) {

	// Ensure we've got enough memory available
	allocateMemory(text.getByteCount(), false);

	memcpy(_text, text.getCharArray(), text.getByteCount());

	_dataLength = text.getByteCount();
	_stringLength = text.getLength();
}

void WoopsiString::setText(const char* text) {

	s32 length = strlen(text);

	// Ensure we've got enough memory available
	allocateMemory(length, false);

	// Copy/filter the valid UTF-8 tokens into _text and cache the length
	s32 unicodeChars = 0;
	_dataLength = filterString(_text, text, length, &unicodeChars);
	_stringLength = unicodeChars;
}

void WoopsiString::setText(const u32 codePoint) {

	// Encode the character
	u8 numBytes = 0;
	const char* encoded = encodeCodePoint(codePoint, &numBytes);

	// Is the codepoint valid?
	if (encoded != NULL) {

		// Ensure we've got enough memory available
		allocateMemory(numBytes, false);
	
		// Copy the valid UTF-8 token into _text and cache the length
		memcpy(_text, encoded, numBytes);
		
		_dataLength = numBytes;
		_stringLength = 1;
	
		delete[] encoded;
	} else {
		
		// Codepoint is invalid; we just need to truncate the string
		_dataLength = 0;
		_stringLength = 0;
	}
}

void WoopsiString::append(const WoopsiString& text) {

	// Ensure we've got enough memory available
	allocateMemory(_dataLength + text.getByteCount(), true);

	memcpy(_text + _dataLength, text.getCharArray(), text.getByteCount());

	_dataLength += text.getByteCount();
	_stringLength += text.getLength();
}

char* WoopsiString::getToken(s32 index) const {
        
	// Early exit if the string is empty
	if (!hasData()) return NULL;

	// Early exit if we want the entire string
	if (index == 0) return _text;

	// Early exit if the index is greater than the length of the string
	if (index >= _stringLength) return NULL;

	char token;
	char* pos = _text;

	while (index > 0) {

		pos++;
		token = *pos;

		if ((token < 0x80) || ((token > 0xC2) && (token < 0xFE))) {
			if (index <= 1) return pos;
			index--;
		}
	}

	// No corresponding token found
	return NULL;
}

void WoopsiString::insert(const WoopsiString& text, s32 index) { 

	// Early exit if the string is empty
	if (!hasData()) {
		WoopsiString::setText(text);
		return;
	}

	// Early exit if we're just appending
	if (index >= _stringLength) {
		WoopsiString::append(text);
		return;
	}

	// Locate the point at which we can cut the existing string 
	s32 insertPoint = getToken(index) - _text;

	// Get the total size of the string that we need
	s32 newSize = _dataLength + text.getByteCount();

	// Reallocate memory if the existing memory isn't large enough
	if (_allocatedSize < newSize) {
		
		newSize += _growAmount;

		// Allocate new string large enough to contain additional data
		char* newText = new char[newSize];

		// Copy the start of the existing text to the newly allocated string
		if (insertPoint > 0) memcpy(newText, _text, insertPoint);

		// Insert the additional text into the new string
		u32 size = text.getByteCount();
		
		memcpy(newText + insertPoint, text.getCharArray(), size);
		_stringLength += text.getLength();

		// Copy the end of the existing text the the newly allocated string
		if (_dataLength > insertPoint) memcpy(newText + insertPoint + size, _text + insertPoint, _dataLength - insertPoint);

		_allocatedSize = newSize;

		// Delete existing string
		delete[] _text;

		// Swap pointers 
		_text = newText;				
		_dataLength += size;
	} else {

		// Existing size large enough, so make space in string for insert
		for (s32 i = 0; i < _dataLength - insertPoint; ++i) {
			_text[newSize - i - 1] = _text[_dataLength - i - 1];
		}

		// Insert the additional text into the new string
		memcpy(_text + insertPoint, text.getCharArray(), text.getByteCount());

		_dataLength += text.getByteCount();
		_stringLength += text.getLength();
	}
}

void WoopsiString::remove(const s32 startIndex) {

	// Reject if requested operation makes no sense
	if (!hasData()) return;
	if (startIndex >= _stringLength) return;
	if (_dataLength == 0) return;

	// Find the UTF-8 token corresponding to startIndex        
	char* pos = getToken(startIndex);

	// Abort if unable to find index
	if (pos == NULL) return;
        
	// Removing characters from the end of the string is trivial - simply
	// decrease the length
	_dataLength = pos - _text;

	_stringLength -= (_stringLength - startIndex);
}

void WoopsiString::remove(const s32 startIndex, const s32 count) {

	// Reject if requested operation makes no sense
	if (!hasData()) return;
	if (startIndex >= _stringLength) return;
	if (_dataLength == 0) return;

	// Use alternative remove method if end point matches or exceeds the
	// size of the string.  The other method is much faster
	if (startIndex + count >= _stringLength) {
		remove(startIndex);
		return;
	}

	// Find the utf-8 token corresponding to startIndex
	char* startPos = getToken(startIndex);

	// Abort if unable to start point
	if (startPos == NULL) return;

	// Find the utf-8 token corresponding to startIndex + count
	char* endPos = getToken(startIndex + count);

	// Abort if unable to find end point
	if (endPos == NULL) while(1) { };//return;

	// Copy characters from a point after the area to be deleted into the space created
	// by the deletion
	if (startPos < endPos) memmove(startPos, endPos, _dataLength - u32(endPos - _text));
	
	// Decrease length
	_dataLength -= (endPos - startPos);

	_stringLength -= count;
}

const u32 WoopsiString::getCharAt(s32 index) const {
	const char* token = getToken(index);
	return getCodePoint(token, NULL);
}

const s32 WoopsiString::indexOf(u32 letter) const {
	return indexOf(letter, 0, getLength());
}

const s32 WoopsiString::indexOf(u32 letter, s32 startIndex) const {
	return indexOf(letter, startIndex, getLength() - startIndex);
}

const s32 WoopsiString::indexOf(u32 letter, s32 startIndex, s32 count) const {

	// Exit if no data available
	if (!hasData()) return -1;

	s32 index = -1;
	s32 charsExamined = 0;

	StringIterator* iterator = newStringIterator();
	if (!iterator->moveTo(startIndex)) return -1;

	do {
		if (iterator->getCodePoint() == letter) {
			index = iterator->getIndex();
			break;
		}

		charsExamined++;
	} while (iterator->moveToNext() && (charsExamined < count));

	delete iterator;

	return index;
}

const s32 WoopsiString::lastIndexOf(u32 letter) const {
	return lastIndexOf(letter, getLength() - 1, getLength());
}

const s32 WoopsiString::lastIndexOf(u32 letter, s32 startIndex) const {
	return lastIndexOf(letter, startIndex, getLength() - (getLength() - startIndex));
}

const s32 WoopsiString::lastIndexOf(u32 letter, s32 startIndex, s32 count) const {

	// Exit if no data available
	if (!hasData()) return -1;

	s32 index = -1;
	s32 charsExamined = 0;

	StringIterator* iterator = newStringIterator();
	if (!iterator->moveTo(startIndex)) return -1;

	do {
		if (iterator->getCodePoint() == letter) {
			index = iterator->getIndex();
			break;
		}

		charsExamined++;
	} while (iterator->moveToPrevious() && (charsExamined <= count));

	delete iterator;

	return index;
}

WoopsiString* WoopsiString::subString(s32 startIndex) const {
	return subString(startIndex, getLength() - startIndex);
}

WoopsiString* WoopsiString::subString(s32 startIndex, s32 length) const {
	WoopsiString* subString = new WoopsiString();
	StringIterator* iterator = newStringIterator();
	if (!iterator->moveTo(startIndex)) return NULL;

	// Build up the string character by character.  This is slower than
	// a straightforward memcpy(), but as we don't know how many bytes
	// are in the requested substring we can't pre-allocate a buffer for
	// the memcpy().  The only possible way to achieve this is to use a
	// two-pass algorithm that firstly gets the number of bytes in the
	// substring, then performs the memcpy().  However, the method used
	// here is probably just as fast, as the string always over-allocates.
	s32 count = 0;
	while (count < length) {
		subString->append(iterator->getCodePoint());
		iterator->moveToNext();
		count++;
	}

	delete iterator;

	return subString;
}

void WoopsiString::allocateMemory(s32 chars, bool preserve) {

	// Do we already have enough memory allocated to contain this new size?
	// If so, we can avoid deallocating and allocating new memory by re-using the old
	
	if (chars > _allocatedSize) {

		// Not enough space in existing memory; allocate new memory
		char* newText = new char[chars + _growAmount];

		// Free old memory if necessary
		if (_text != NULL) {

			// Preserve existing data if required
			if (preserve) memcpy(newText, _text, _dataLength);

			delete[] _text;
		}

		// Swap pointer to new memory
		_text = newText;

		// Remember how much memory we've allocated
		_allocatedSize = chars + _growAmount;
	}
}

void WoopsiString::copyToCharArray(char* buffer) const {
	memcpy(buffer, _text, _dataLength);
	buffer[_dataLength] = '\0';
}

s32 WoopsiString::filterString(char* dest, const char* src, s32 sourceBytes, s32* totalUnicodeChars) const {
	u8 bytes;
	s32 totalBytes = 0;

	while (totalBytes < sourceBytes) {
		getCodePoint(src, &bytes);

		if (bytes == 0) {

			// This utf-8 token is corrupt; ignore the first char and find a new utf-8 token
			src++;
		} else {

			// Copy the valid utf-8 tokens
			memcpy(dest, src, bytes);
			totalBytes += bytes;
			*totalUnicodeChars += 1;
			src += bytes;
			dest += bytes;
		}
	}

	return totalBytes;
}

u32 WoopsiString::getCodePoint(const char* string, u8* numChars) const {
	char char0 = *string;

	// 0xxxxxxx ASCII char
	if (char0 < 0x80) {
		if (numChars) *numChars = 1;
		return char0;
	}

	// 1xxxxxxx 10xxxxxx
	if ((string[1] < 0x80) || (string[1] >= 0xC0)) return 0; 

	// 110xxxxx 10xxxxxx
	if (char0 < 0xE0) {
		if (char0 < 0xC2) return 0; // 10xxxxxx (invalid leading char) or  1100000x 10xxxxxx (invalid representation : should have been coded with just 1 char)
		if (numChars) *numChars = 2;
		return ((char0 - 0xC0) << 6) | (string[1] - 0x80); 
	}

	// 111yyyyy 10xxxxxx 10xxxxxx
	if ((string[2]<0x80) || (string[2] >= 0xC0)) return 0; 
	
	// 1110xxxx 10xxxxxx 10xxxxxx
	if (char0 < 0xF0) {
		if ((char0 == 0xE0) && (string[1] < 0xA0)) return 0; // 11100000 100xxxxx 10xxxxxx (invalid representation : should have been coded with at most 2 chars)
		if (numChars) *numChars = 3;
		return ((char0 - 0xE0) << 12) | ((string[1] - 0x80) << 6) | (string[2] - 0x80); 
	}

	// There shouldn't be many utf-8 tokens beyond this point

	// 1111yyyy 10xxxxxx 10xxxxxx 10xxxxxx
	if ((string[3] < 0x80) || (string[3] >= 0xC0)) return 0; 

	// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	if (char0 < 0xF8) {
		if ((char0 == 0xF0) && (string[1]<0x90)) return 0; // 11110000 1000xxxx 10xxxxxx 10xxxxxx  (invalid representation : should have been coded with at most 3 chars)
		if (numChars) *numChars = 4;
		return ((char0 - 0xF0) << 18) | ((string[1] - 0x80) << 12) | ((string[2] - 0x80) << 6) | (string[3] - 0x80);
	}            

	// 11111yyy 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	if ((string[4] < 0x80) || (string[4] >= 0xC0)) return 0; 

	// 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	if (char0 < 0xFC) {
		if ((char0 == 0xF8) && (string[1]<0x88)) return 0; // 11111000 10000xxx 10xxxxxx 10xxxxxx 10xxxxxx (invalid representation : should have been coded with at most 4 chars)
		if (numChars) *numChars = 5;
		return ((char0-0xF8)<<24) | ((string[1]-0x80)<<18) | ((string[2]-0x80)<<12) | ((string[3]-0x80)<<6) | (string[4] -0x80);
	}   

	// 111111yy 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	if ((string[5] < 0x80) || (string[5] >= 0xC0)) return 0; 

	// 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	if (char0 < 0xFE) {
		if ((char0 == 0xFC) && (string[1]<0x84)) return 0; // 11111100 100000xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx (invalid representation : should have been coded with at most 5 chars)
		if (numChars) *numChars = 6;
		return ((char0-0xFC)<<30) | ((string[1]-0x80)<<24) | ((string[2]-0x80)<<18) | ((string[3]-0x80)<<12) | ((string[4]-0x80)<<6) | (string[5] -0x80);
	}   

	// 11111110 and 11111111 are invalid
	return 0;
}

const char* WoopsiString::encodeCodePoint(u32 codepoint, u8* numBytes) const {
	
	*numBytes = 0;
	
	if (codepoint < 0x80) {
		*numBytes = 1;
		char* buffer = new char[1];
		buffer[0] = codepoint;
		return buffer;
	}

	if (codepoint < 0x0800) {
		*numBytes = 2;
		char* buffer = new char[2];
		buffer[0] = (codepoint >> 6) + 0xC0;
		buffer[1] = (codepoint & 0x1F) + 0x80;
		return buffer;
	}

	if (codepoint < 0x10000) {
		*numBytes = 3;
		char* buffer = new char[3];
		buffer[0] = (codepoint >> 12) + 0xE0;
		buffer[1] = ((codepoint >> 6) & 0x3F) + 0x80;
		buffer[2] = (codepoint & 0x3F) + 0x80;
		return buffer;
	}
	
	if (codepoint < 0x10FFFF) {
		*numBytes = 4;
		char* buffer = new char[4];
		buffer[0] = (codepoint >> 18) + 0xF0;
		buffer[1] = ((codepoint >> 12) & 0x3F) + 0x80;
		buffer[2] = ((codepoint >> 6) & 0x3F) + 0x80;
		buffer[3] = (codepoint & 0x3F) + 0x80;
		return buffer;
	}

	// No legal codepoints after this point
	return NULL;
}

s32 WoopsiString::compareTo(const WoopsiString& string) const {
	return strncmp(_text, string.getCharArray(), getLength());
}
