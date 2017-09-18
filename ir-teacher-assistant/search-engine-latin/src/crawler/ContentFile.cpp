/**
 * ContentFile class implementation
 * @author: ueda@dcc.ufmg.br
 */

#include "ContentFile.h"

using namespace std;
using namespace RICPNS;

ContentFile::ContentFile()
	: text_(),
	  length_(0) {}

ContentFile::ContentFile(const std::string & text,
						 const size_t & length)
	: text_(text),
	  length_(0) {
}

ContentFile::~ContentFile() {}

void ContentFile::setText(const std::string & text) {
	text_ = text;
}

string ContentFile::getText() const {
	return text_;
}

void ContentFile::setLength(const size_t & length) {
	length_ = length;
}

size_t ContentFile::getLength() const {
	return length_;
}

void ContentFile::clear() {
	setText("");
	setLength(0);
}

