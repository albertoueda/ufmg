/*
 * DocURLInfo.cpp
 *
 *  Created on: Jun 4, 2013
 *      
 */

#include "DocURLInfo.h"

const ui DocURLInfo::MAX_VALID_DOC_ID = 1e7;

DocURLInfo::DocURLInfo(ui pdocId) :
		docId(pdocId), numberOfOutLinks(0u), rank(1.0f)
{

}

DocURLInfo::~DocURLInfo() {

}

