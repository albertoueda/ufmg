/**
 * ContentFile representation
 * author: ueda@dcc.ufmg.br
 */

#ifndef __CONTENT_FILE_H__
#define __CONTENT_FILE_H__

#include <string>

namespace RICPNS
{
	class ContentFile
	{
		public:

			ContentFile();
			ContentFile(const std::string & text,
						const size_t & length);
			virtual ~ContentFile();

			void setText(const std::string & text);
			std::string getText() const;
			void setLength(const size_t & length);
			size_t getLength() const;

			void clear();

		private:
			std::string url_;
			std::string text_;
			size_t length_;
	};
}

#endif

