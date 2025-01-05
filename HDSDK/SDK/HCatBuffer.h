

#ifndef __HCATBUFFER_H__
#define __HCATBUFFER_H__


#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <vector>


namespace cat
{


class HCatBuffer
{
public:
    typedef typename std::string::difference_type difference_type;
    typedef HCatBuffer value_type;
    typedef HCatBuffer * pointer;
    typedef HCatBuffer & reference;
    typedef typename std::string::iterator::iterator_category iterator_category;

public:
    HCatBuffer() : buff_(new std::string()) {}
    explicit HCatBuffer(std::size_t len) : buff_(new std::string(len, '\0')) {}
    HCatBuffer(std::size_t len, char c) : buff_(new std::string(len, c)) {}
    HCatBuffer(const char *buff, std::size_t len) : buff_(new std::string(buff, len)) {}
    HCatBuffer(const char *buff) : buff_(new std::string(buff ? buff : "")) {}
    HCatBuffer(const std::string &buff) : buff_(new std::string(buff)) {}
    HCatBuffer(std::string &&buff) : buff_(new std::string(std::move(buff))) {}
    HCatBuffer(const HCatBuffer &buff) : buff_(buff.buff_) {}

    std::string::iterator begin() { Detach(); return buff_->begin(); }
    std::string::const_iterator begin() const { return buff_->begin(); }
    std::string::iterator end() { Detach(); return buff_->end(); }
    std::string::const_iterator end() const { return buff_->end(); }

    static bool isUpperCaseAscii(char c) { return c >= 'A' && c <= 'Z'; }
    static bool isLowerCaseAscii(char c) { return c >= 'a' && c <= 'z'; }
    bool isUpper() const { return !std::any_of(buff_->begin(), buff_->end(), isLowerCaseAscii); }
    bool isLower() const { return !std::any_of(buff_->begin(), buff_->end(), isUpperCaseAscii); }

    static unsigned char asciiUpper(unsigned char c) { return c >= 'a' && c <= 'z' ? c & ~0x20 : c; }
    static unsigned char asciiLower(unsigned char c) { return c >= 'A' && c <= 'Z' ? c | 0x20 : c; }
    HCatBuffer toLower() const {
        HCatBuffer result(*this);
        std::transform(buff_->begin(), buff_->end(), result.begin(), asciiLower);
        return result;
    }
    HCatBuffer toUpper() const {
        HCatBuffer result(*this);
        std::transform(buff_->begin(), buff_->end(), result.begin(), asciiUpper);
        return result;
    }

    std::string GetString() const { return *buff_; }
    std::string GetString(std::size_t len) const { return buff_->substr(0, len); }
    const std::string &GetConstString() const { return *buff_; }
    std::string &GetRefString() { Detach(); return *buff_; }

    std::string::pointer Data() { Detach(); return &buff_->front(); }
    std::string::const_pointer Data() const { return &buff_->front(); }
    std::string::const_pointer ConstData() const { return buff_->data(); }
    std::size_t Size() const { return buff_->size(); }
    HCatBuffer &Swap(HCatBuffer &buff) { Detach(); buff_.swap(buff.buff_); return *this; }
    std::string::reference At(std::size_t index) {  Detach(); return buff_->at(index); }
    std::string::const_reference At(std::size_t index) const { return buff_->at(index); }
    bool Empty() const { return buff_->empty(); }
    HCatBuffer &Clear() { Detach(); std::string().swap(*buff_); return *this; }

    std::string::reference operator [](std::size_t index) { Detach(); return buff_->at(index); }
    std::string::const_reference operator [](std::size_t index) const { return buff_->at(index); }

    HCatBuffer &Insert(const char *buff, std::size_t index) { Detach(); buff_->insert(index, buff); return *this; }
    HCatBuffer &Insert(const char *buff, std::size_t index, std::size_t len) { Detach(); buff_->insert(index, buff, len); return *this; }
    HCatBuffer &Insert(const std::string &buff, std::size_t index) { Detach(); buff_->insert(index, buff); return *this; }
    HCatBuffer &Insert(const HCatBuffer &buff, std::size_t index) { Detach(); buff_->insert(index, *buff.buff_); return *this; }

    HCatBuffer &Remove(std::size_t index, std::size_t num = std::string::npos) {
        if (index > buff_->size()) {
            index = buff_->size();
        }
        Detach();
        buff_->erase(index, num);
        return *this;
    }

    HCatBuffer Mid(std::size_t index, std::size_t len = std::string::npos) const {
        if (index >= buff_->size()) {
            return HCatBuffer();
        }
        if (len > buff_->size()) {
            len = buff_->size();
        }
        if (len > buff_->size() - index) {
            len = buff_->size() - index;
        }
        if (index == 0 && len == buff_->size()) {
            return *this;
        }
        return HCatBuffer(buff_->substr(index, len));
    }

    std::string::size_type Find(char value, std::size_t pos = 0) const { return buff_->find(value, pos); }
    std::string::size_type Find(const char *value, std::size_t pos = 0) const { return buff_->find(value, pos); }
    std::string::size_type Find(const std::string &value, std::size_t pos = 0) const { return buff_->find(value, pos); }
    std::string::size_type IndexOf(char value, std::size_t pos = 0) const { return Find(value, pos); }
    std::string::size_type IndexOf(const char *value, std::size_t pos = 0) const { return Find(value, pos); }
    std::string::size_type IndexOf(const std::string &value, std::size_t pos = 0) const { return Find(value, pos); }

    std::string::size_type Rfind(char value, std::size_t pos = std::string::npos) const { return buff_->rfind(value, pos); }
    std::string::size_type Rfind(const char *value, std::size_t pos = std::string::npos) const { return buff_->rfind(value, pos); }
    std::string::size_type Rfind(const std::string &value, std::size_t pos = std::string::npos) const { return buff_->rfind(value, pos); }
    std::string::size_type LastIndexOf(char value, std::size_t pos = std::string::npos) const { return Rfind(value, pos); }
    std::string::size_type LastIndexOf(const char *value, std::size_t pos = std::string::npos) const { return Rfind(value, pos); }
    std::string::size_type LastIndexOf(const std::string &value, std::size_t pos = std::string::npos) const { return Rfind(value, pos); }

    bool Contains(char c) const { return IndexOf(c) != std::string::npos; }
    bool Contains(const char *c) const { return IndexOf(c) != std::string::npos; }
    bool Contains(const std::string &c) const { return IndexOf(c) != std::string::npos; }

    HCatBuffer &Append(const std::string &buff) { Detach(); buff_->append(buff); return *this; }
    HCatBuffer &Append(std::string &&buff) { Detach(); buff_->append(std::move(buff)); return *this; }
    HCatBuffer &Append(const HCatBuffer &buff) { Detach(); buff_->append(*buff.buff_); return *this; }
    HCatBuffer &Append(HCatBuffer &&buff) { Detach(); buff_->append(std::move(*buff.buff_)); return *this; }
    HCatBuffer &Append(const char *buff) { if (buff == nullptr) { return *this; } Detach(); buff_->append(buff); return *this; }
    HCatBuffer &Append(const char *buff, std::size_t size) { Detach(); buff_->append(buff, size); return *this; }
    HCatBuffer &Append(const unsigned char *buff) { Detach(); buff_->append(reinterpret_cast<const char *>(buff)); return *this; }
    HCatBuffer &Append(const unsigned char *buff, std::size_t size) { Detach(); buff_->append(reinterpret_cast<const char *>(buff), size); return *this; }
    HCatBuffer &Append(const char buff) { Detach(); buff_->push_back(buff); return *this; }
    HCatBuffer &Append(int len, const char buff) { Detach(); buff_->append(len, buff); return *this; }
    HCatBuffer &Append(const unsigned char buff) { Detach(); buff_->push_back(static_cast<char>(buff)); return *this; }
    HCatBuffer &Append(int len, const unsigned char buff) { Detach(); buff_->append(len, static_cast<char>(buff)); return *this; }

    HCatBuffer &Replace(const std::string &src, const std::string &dest) {
        std::string::size_type pos = buff_->find(src);
        if (pos == std::string::npos) {
            return *this;
        }

        Detach();
        buff_->replace(pos, src.size(), dest);
        return *this;
    }

    HCatBuffer Left(std::size_t len) const {
        if (len >= buff_->size()) {
            return *this;
        }
        return HCatBuffer(buff_->data(), len);
    }
    HCatBuffer Right(std::size_t len) const {
        if (len >= buff_->size()) {
            return *this;
        }
        return HCatBuffer((&(*buff_->end())) - len, len);
    }

    bool StartsWith(const cat::HCatBuffer &other) const {
        if (Size() < other.Size()) {
            return false;
        }
        if (buff_ == other.buff_ || other.Size() == 0) {
            return true;
        }
        return memcmp(ConstData(), other.ConstData(), other.Size()) == 0;
    }
    bool EndsWith(const cat::HCatBuffer &other) const {
        if (Size() < other.Size()) {
            return false;
        }
        if (buff_ == other.buff_ || other.Size() == 0) {
            return true;
        }
        return memcmp((ConstData() + Size() - other.Size()), other.ConstData(), other.Size()) == 0;
    }

    std::vector<HCatBuffer> Split(const std::string &sep, bool keepEmpty = true) const {
        std::vector<HCatBuffer> result;
        std::size_t startPos = 0;
        std::size_t endPos = 0;
        std::size_t extra = 0;
        while ((endPos = buff_->find(sep, startPos + extra)) != std::string::npos) {
            result.emplace_back(Mid(startPos, endPos - startPos));
            startPos = endPos + sep.size();
            extra = sep.size() == 0 ? 1 : 0;
        }
        if (startPos != buff_->size() || keepEmpty) {
            result.emplace_back(Mid(startPos));
        }
        return result;
    }
    std::vector<HCatBuffer> Split(char sep, bool keepEmpty = true) const { return Split(std::string(1, sep), keepEmpty); }
    static HCatBuffer Join(const std::vector<HCatBuffer> &that, const cat::HCatBuffer &sep) {
        std::size_t totalLen = 0;
        const std::size_t size = that.size();
        for (std::size_t i = 0; i < size; ++i) {
            totalLen += that.at(i).Size();
        }
        if (size > 0) {
            totalLen += sep.Size() * (size - 1);
        }

        HCatBuffer result;
        if (totalLen) {
            result.buff_->reserve(totalLen);
        }
        for (std::size_t i = 0; i < size; ++i) {
            if (i) {
                result.Append(sep);
            }
            result.Append(that.at(i));
        }
        return result;
    }
    static HCatBuffer Join(const std::vector<HCatBuffer> &that, char sep) { return Join(that, HCatBuffer(1, sep)); }

    HCatBuffer Simplified() const {
        if (buff_->empty()) {
            return *this;
        }
        const char *srcPos = ConstData();
        const char *endPos = ConstData() + Size();
        cat::HCatBuffer result(Size());
        char *dst = result.Data();
        char *ptr = dst;
        for (;;) {
            while (srcPos != endPos && std::isspace(static_cast<unsigned char>(*srcPos))) {
                ++srcPos;
            }
            while (srcPos != endPos && !std::isspace(static_cast<unsigned char>(*srcPos))) {
                *ptr++ = *srcPos++;
            }
            if (srcPos == endPos) {
                break;
            }
            *ptr++ = ' ';
        }
        if (ptr != dst && std::isspace(static_cast<unsigned char>(ptr[-1]))) {
            --ptr;
        }

        std::size_t newLen = ptr - dst;
        result.buff_->resize(newLen);
        return result;
    }

    static char ToHexLower(int value) noexcept { return "0123456789abcdef"[value & 0xF]; }
    static char toHexUpper(int value) noexcept { return "0123456789ABCDEF"[value & 0xF]; }
    HCatBuffer ToHex(char separator, bool upper = false) const {
        if (buff_->empty()) {
            return HCatBuffer();
        }

        const std::size_t length = separator ? (buff_->size() * 3 - 1) : (buff_->size() * 2);
        HCatBuffer hex(length);
        char *hexData = hex.Data();
        const unsigned char *data = reinterpret_cast<const unsigned char *>(this->ConstData());
        for (std::size_t i = 0, o = 0; i < buff_->size(); ++i) {
            if (upper) {
                hexData[o++] = toHexUpper(data[i] >> 4);
                hexData[o++] = toHexUpper(data[i] & 0xf);
            } else {
                hexData[o++] = ToHexLower(data[i] >> 4);
                hexData[o++] = ToHexLower(data[i] & 0xf);
            }

            if ((separator) && (o < length))
                hexData[o++] = separator;
        }

        return hex;
    }

    static int FromHex(int c) {
        return ((c >= '0') && (c <= '9')) ? int(c - '0') :
               ((c >= 'A') && (c <= 'F')) ? int(c - 'A' + 10) :
               ((c >= 'a') && (c <= 'f')) ? int(c - 'a' + 10) :
               /* otherwise */              -1;
    }
    static HCatBuffer FromHex(const HCatBuffer &hexEncoded) {
        HCatBuffer res((hexEncoded.Size() + 1) / 2);
        unsigned char *result = reinterpret_cast<unsigned char *>(res.Data() + res.Size());

        bool odd_digit = true;
        for (long i = hexEncoded.Size() - 1; i >= 0; --i) {
            unsigned char ch = static_cast<unsigned char>(hexEncoded.At(i));
            int tmp = FromHex(ch);
            if (tmp == -1)
                continue;
            if (odd_digit) {
                --result;
                *result = tmp;
                odd_digit = false;
            } else {
                *result |= tmp << 4;
                odd_digit = true;
            }
        }

        res.Remove(0, result - reinterpret_cast<const unsigned char *>(res.ConstData()));
        return res;
    }

    HCatBuffer ToBase64() const {
        if (this->Empty()) {
            return *this;
        }

        const char alphabet_base64[] = "ABCDEFGH" "IJKLMNOP" "QRSTUVWX" "YZabcdef"
                                       "ghijklmn" "opqrstuv" "wxyz0123" "456789+/";
        const char *const alphabet = alphabet_base64;
        const char padchar = '=';
        int padlen = 0;
        const std::size_t sz = Size();
        HCatBuffer tmp((sz + 2) / 3 * 4);

        std::size_t i = 0;
        char *out = tmp.Data();
        while (i < sz) {
            int chunk = 0;
            chunk |= int(static_cast<unsigned char>(ConstData()[i++])) << 16;
            if (i == sz) {
                padlen = 2;
            } else {
                chunk |= int(static_cast<unsigned char>(ConstData()[i++])) << 8;
                if (i == sz) {
                    padlen = 1;
                } else {
                    chunk |= int(static_cast<unsigned char>(ConstData()[i++]));
                }
            }

            int j = (chunk & 0x00fc0000) >> 18;
            int k = (chunk & 0x0003f000) >> 12;
            int l = (chunk & 0x00000fc0) >> 6;
            int m = (chunk & 0x0000003f);
            *out++ = alphabet[j];
            *out++ = alphabet[k];

            if (padlen > 1) {
                *out++ = padchar;
            } else {
                *out++ = alphabet[l];
            }
            if (padlen > 0) {
                *out++ = padchar;
            } else {
                *out++ = alphabet[m];
            }
        }

        std::size_t diffSize = out - tmp.ConstData();
        if (diffSize < tmp.Size()) {
            tmp.buff_->resize(diffSize);
        }
        return tmp;
    }

    static HCatBuffer FromBase64(const HCatBuffer &base64) {
        if (base64.Empty()) {
            return base64;
        }
        const auto inputSize = base64.Size();
        HCatBuffer result((inputSize * 3) / 4);
        enum Base64Option {
            Base64Encoding = 0,
            Base64UrlEncoding = 1,
            IgnoreBase64DecodingErrors = 0,
            AbortOnBase64DecodingErrors = 4,
        };
        Base64Option options = Base64Encoding;

        const char *input = base64.ConstData();
        char *output = result.Data();
        unsigned int buf = 0;
        int nbits = 0;

        std::size_t offset = 0;
        for (std::size_t i = 0; i < inputSize; ++i) {
            int ch = input[i];
            int d;

            if (ch >= 'A' && ch <= 'Z') {
                d = ch - 'A';
            } else if (ch >= 'a' && ch <= 'z') {
                d = ch - 'a' + 26;
            } else if (ch >= '0' && ch <= '9') {
                d = ch - '0' + 52;
            } else if (ch == '+' && (options & Base64UrlEncoding) == 0) {
                d = 62;
            } else if (ch == '-' && (options & Base64UrlEncoding) != 0) {
                d = 62;
            } else if (ch == '/' && (options & Base64UrlEncoding) == 0) {
                d = 63;
            } else if (ch == '_' && (options & Base64UrlEncoding) != 0) {
                d = 63;
            } else {
                if (options & AbortOnBase64DecodingErrors) {
                    if (ch == '=') {
                        // can have 1 or 2 '=' signs, in both cases padding base64Size to
                        // a multiple of 4. Any other case is illegal.
                        if ((inputSize % 4) != 0) {
                            result.Clear();
                            return result;
                        } else if ((i == inputSize - 1) ||
                            (i == inputSize - 2 && input[++i] == '=')) {
                            d = -1; // ... and exit the loop, normally
                        } else {
                            result.Clear();
                            return result;
                        }
                    } else {
                        result.Clear();
                        return result;
                    }
                } else {
                    d = -1;
                }
            }

            if (d != -1) {
                buf = (buf << 6) | d;
                nbits += 6;
                if (nbits >= 8) {
                    nbits -= 8;
                    if (offset >= i) {
                        result.Clear();
                        return result;
                    }
                    output[offset++] = buf >> nbits;
                    buf &= (1 << nbits) - 1;
                }
            }
        }

        if (offset < result.Size()) {
            result.buff_->resize(offset);
        }
        return result;
    }

    int ToInt(int base = 10) const {
        if (buff_->empty()) {
            return 0;
        }

        int result = 0;
        try {
#if __cplusplus >= 201402L
            result = std::stoi(buff_->data(), nullptr, base);
#else
            result = strtol(buff_->data(), nullptr, base);
#endif
        } catch(...) {
            return result;
        }
        return result;
    }
    long long ToLongLong(int base = 10) const {
        if (buff_->empty()) {
            return 0;
        }

        long long result = 0;
        try {
#if __cplusplus >= 201402L
            result = std::stoll(buff_->data(), nullptr, base);
#else
            result = strtoll(buff_->data(), nullptr, base);
#endif
        } catch(...) {
            return result;
        }
        return result;
    }
    float ToFloat() const {
        if (buff_->empty()) {
            return 0.0;
        }

        float result = 0.0;
        try {
#if __cplusplus >= 201402L
            result = std::stof(buff_->data());
#else
            result = strtof(buff_->data(), nullptr);
#endif
        } catch(...) {
            return result;
        }
        return result;
    }

    HCatBuffer &SetNum(long long n, int base = 10) {
        // big enough for MAX_ULLONG in base 2
        const int buffsize = 66;
        char buff[buffsize];
        char *p = nullptr;
        if (n < 0) {
            p = LToS(buff + buffsize, static_cast<unsigned long long>(-(1 + n) + 1), base);
            *--p = '-';
        } else {
            p = LToS(buff + buffsize, static_cast<unsigned long long>(n), base);
        }
        Clear();
        return Append(p, buffsize - (p - buff));
    }
    HCatBuffer &SetNum(unsigned long long n, int base = 10) {
        // big enough for MAX_ULLONG in base 2
        const int buffsize = 66;
        char buff[buffsize];
        char *p = LToS(buff + buffsize, n, base);
        Clear();
        return Append(p, buffsize - (p - buff));
    }

    static HCatBuffer Number(int value, int base = 10) { HCatBuffer result; return result.SetNum(static_cast<unsigned long long>(value), base); }
    static HCatBuffer Number(long long value, int base = 10) { HCatBuffer result; return result.SetNum(value, base); }


    HCatBuffer operator +(const HCatBuffer &buff) const { return *buff_ + *buff.buff_; }
    HCatBuffer operator +(const std::string &buff) const { return *buff_ + buff; }
    HCatBuffer operator +(const char *buff) const { return *buff_ + buff; }
    HCatBuffer operator +(const char buff) const { return *buff_ + buff; }

    HCatBuffer &operator +=(const HCatBuffer &buff) { return Append(buff); }
    HCatBuffer &operator +=(const std::string &buff) { return Append(buff); }
    HCatBuffer &operator +=(const char *buff) { return Append(buff); }
    HCatBuffer &operator +=(const char buff) { return Append(buff); }

    HCatBuffer &operator =(const HCatBuffer &buff) { Detach(); buff_->assign(*buff.buff_); return *this; }
    HCatBuffer &operator =(const std::string &buff) { Detach(); buff_->assign(buff); return *this; }
    HCatBuffer &operator =(std::string &&buff) { Detach(); buff_->assign(std::move(buff)); return *this; }
    HCatBuffer &operator =(const char *buff) { if (buff == nullptr) { return *this; } Detach(); buff_->assign(buff); return *this; }

    bool operator==(const HCatBuffer &buff) const { return *buff_ == *buff.buff_; }
    bool operator==(const std::string &buff) const { return *buff_ == buff; }
    bool operator==(const char *buff) const { return *buff_ == buff; }

    bool operator!=(const HCatBuffer &buff) const { return *buff_ != *buff.buff_; }
    bool operator!=(const std::string &buff) const { return *buff_ != buff; }
    bool operator!=(const char *buff) const { return *buff_ != buff; }

    bool operator<(const HCatBuffer &buff) const { return *buff_ < *buff.buff_; }
    bool operator<=(const HCatBuffer &buff) const { return *buff_ < *buff.buff_; }
    bool operator>(const HCatBuffer &buff) const { return *buff_ > *buff.buff_; }
    bool operator>=(const HCatBuffer &buff) const { return *buff_ > *buff.buff_; }

private:
    void Detach() {
        if (buff_.use_count() <= 1) {
            return ;
        }

        buff_.reset(new std::string(*buff_));
    }

    // Convert number to string
    static char *LToS(char *p , unsigned long long n, int base) {
        if (base < 2 || base > 36) {
            base = 10;
        }
        const char b = 'a' - 10;
        do {
            const int c = n % base;
            n /= base;
            *--p = c + (c < 10 ? '0' : b);
        } while (n);

        return p;
    }

private:
    std::shared_ptr<std::string> buff_;
};


}


#endif // __HCATBUFFER_H__
