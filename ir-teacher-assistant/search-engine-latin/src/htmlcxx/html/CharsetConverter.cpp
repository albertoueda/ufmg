#include <cstdio>
#include <cstring>
#include <cerrno>
#include <iostream>
#include "CharsetConverter.h"

using namespace std;
using namespace htmlcxx;

CharsetConverter::CharsetConverter(const string &from, const string &to) throw (Exception)
{
	mIconvDescriptor = iconv_open(to.c_str(), from.c_str());
	if (mIconvDescriptor == (iconv_t)(-1))
	{
		const char *error_str = strerror(errno);
		int size = strlen(error_str) + from.length() + to.length() + 26;
		char error[size];
		snprintf(error, size, "Can't convert from %s to %s: %s", from.c_str(), to.c_str(), error_str);
		throw Exception(error);
	}
//	bufferSize = 1024 * 1024;
//	sbuffer = new char[bufferSize];
}

CharsetConverter::~CharsetConverter()
{
	iconv_close(mIconvDescriptor);
//	delete [] sbuffer;
}

//void CharsetConverter::convert(const std::string &input, std::string &out){
//	//out = convert(input);
////	return;
//	const char *inbuf = input.c_str();
//	size_t inbytesleft = input.length();
//
//	size_t outbuf_len = input.length();
//	if(outbuf_len > bufferSize){
//		bufferSize = outbuf_len;
//		delete [] sbuffer;
//		sbuffer = new char[bufferSize];
//	}
//	char *outbuf_start = sbuffer;
//	char *outbuf = outbuf_start;
//	size_t outbytesleft = outbuf_len;
//
//	size_t ret;
//	while (1) {
//		ret = iconv(mIconvDescriptor, const_cast<char**>(&inbuf), &inbytesleft, &outbuf, &outbytesleft);
//
//		if (ret == 0 || inbytesleft == 0) break;
//		if (ret == (size_t)-1 && errno == E2BIG){
//				fprintf(stderr, "invalid byte: %ld\n", inbuf - input.c_str());
//				out.assign("");
//				//std::cerr << "END convert\n" ;
//
//				return;
//		}
//
//		inbuf++; inbytesleft--;
//	}
//	out.assign(outbuf_start, outbuf_len - outbytesleft);
//
//	//delete [] outbuf_start;
//}


string CharsetConverter::convert(const string &input)
{
	const char *inbuf = input.c_str();
	size_t inbytesleft = input.length();

	size_t outbuf_len = input.length();
	char *outbuf_start = new char[outbuf_len];
	char *outbuf = outbuf_start;
	size_t outbytesleft = outbuf_len;

	size_t ret;
	while (1) {
		ret = iconv(mIconvDescriptor, const_cast<char**>(&inbuf), &inbytesleft, &outbuf, &outbytesleft);
		if (ret == 0 || inbytesleft == 0) break;
		if (ret == (size_t)-1 && errno == E2BIG){
				fprintf(stderr, "invalid byte: %ld\n", inbuf - input.c_str());
				return string();
		}

		inbuf++; inbytesleft--;
	}

	string out(outbuf_start, outbuf_len - outbytesleft);

	delete [] outbuf_start;

	return out;
}
