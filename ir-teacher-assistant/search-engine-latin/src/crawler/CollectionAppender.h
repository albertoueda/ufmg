/**
 * Collection Appender
 * @author: ueda@dcc.ufmg.br
 */

#ifndef __COLLECTION_APPENDER_H__
#define __COLLECTION_APPENDER_H__

#include <stddef.h>
#include <stdint.h>
#include <cstdio>
#include <unordered_map>

#include "ContentFile.h"
#include "Document.h"

namespace RICPNS {

	class CollectionAppender {
		public:

			CollectionAppender(const std::string & inputDirectory,
							 const std::string & inputIndexFileName,
							 const std::string & outputDirectory,
							 const std::string & outputIndexFileName,
							 const std::string & outputPrefixFileName);

			virtual ~CollectionAppender();

			void append(unsigned first, unsigned last);

		private:

			typedef unsigned char * TUCharPtr;

			void initialize();			
			void readInputIndex();
			bool getNextContentFile(ContentFile& content, const std::string& filename);
			bool getNextDocument(ContentFile& content, Document& doc);
			int compressDocument(const Document & document, TUCharPtr & to, size_t & size);
			void appendDocument(const Document & document,
					TUCharPtr dPtr,
					const size_t & compressedDocSize);

			bool valid(const Document & doc);

			std::string inputDirectory_;
			std::string inputIndexFileName_;
			std::string outputDirectory_;
			std::string outputIndexFileName_;
			std::string outputPrefixFileName_;

			FILE * inputIndexFilePtr_;
			FILE * inputFile;
			FILE * outputIndexFilePtr_;
			FILE * outputContentFilePtr_;
			uint32_t outputContentIndex_;

			std::string inputContentFileName_;
			size_t outputCurrentOffset_;
			size_t filePos_;

			std::unordered_map<std::string, bool> urls;
	};
}

#endif

