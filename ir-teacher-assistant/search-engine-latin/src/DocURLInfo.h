/*
 * DocURLInfo.h
 *
 *  Created on: Jun 4, 2013
 *      
 */

#ifndef DOCURLINFO_H_
#define DOCURLINFO_H_

#include "common.h"
#include <unordered_map>
#include <vector>

class DocURLInfo {
public:
	DocURLInfo(ui pdocId=MAX_VALID_DOC_ID);
	~DocURLInfo();

	static const ui MAX_VALID_DOC_ID;

	ui docId;
	ui numberOfOutLinks;
	vector<DocURLInfo*> listInLinks;
	float rank;
	unordered_map<ui, ui> mapWords; //wordId, frequence
	string url;//retirar
};

#endif /* DOCURLINFO_H_ */
