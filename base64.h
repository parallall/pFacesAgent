#ifndef _BASE64_H_
#define _BASE64_H_

#include <vector>
#include <string>

class Base64 {
	typedef unsigned char BYTE;

	static inline bool is_base64(BYTE c);

public:
	static std::string encode(BYTE const* buf, unsigned int bufLen);
	static std::vector<BYTE> decode(std::string const&);
};

#endif