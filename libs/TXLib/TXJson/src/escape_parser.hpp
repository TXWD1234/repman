#pragma once

#include <string>
#include <string_view>
#include <stdexcept>
#include <cstdint>
#include <array>
#include <algorithm>

// Effort of ChatGPT, Gemini, Claude, Deepseek
class EscapeParser {
public:
	/// Parse a string containing backslash escape sequences and return
	/// the decoded result.  Supported escapes:
	///   \n \r \t \b \f \a \v \\ \" \'  \e
	///   \xHH   \uHHHH   \UHHHHHHHH
	///   \0 – \377  (1–3 octal digits, max value 0377 / 255)
	static std::string apply(std::string_view input) {
		return EscapeParser(input).parse();
	}

	EscapeParser(const EscapeParser&) = delete;
	EscapeParser& operator=(const EscapeParser&) = delete;

private:
	std::string_view input_;
	std::size_t pos_ = 0;
	std::string result_;

	explicit EscapeParser(std::string_view in) : input_(in) {
		result_.reserve(in.size());
	}

	// -----------------------------------------------------------------------
	// Error helpers
	// -----------------------------------------------------------------------

	[[noreturn]] void throwError(const std::string& msg) const {
		throw std::runtime_error(
		    "EscapeParser at position " + std::to_string(pos_) + ": " + msg);
	}

	// -----------------------------------------------------------------------
	// Input helpers
	// -----------------------------------------------------------------------

	bool hasMore() const noexcept { return pos_ < input_.size(); }

	char peek() const {
		if (!hasMore()) throwError("unexpected end of input");
		return input_[pos_];
	}

	char advance() {
		if (!hasMore()) throwError("unexpected end of input");
		return input_[pos_++];
	}

	// -----------------------------------------------------------------------
	// Formatting helpers
	// -----------------------------------------------------------------------

	static std::string toHex(uint32_t val) {
		static constexpr char digits[] = "0123456789ABCDEF";
		if (val == 0) return "0";

		std::array<char, 8> buf{};
		int len = 0;
		while (val && len < 8) {
			buf[static_cast<std::size_t>(len++)] = digits[val & 0xF];
			val >>= 4;
		}
		std::reverse(buf.begin(), buf.begin() + len);
		return std::string(buf.data(), static_cast<std::size_t>(len));
	}

	static std::string toOctal(uint32_t val) {
		if (val == 0) return "0";

		std::array<char, 11> buf{};
		int len = 0;
		while (val && len < 11) {
			buf[static_cast<std::size_t>(len++)] =
			    static_cast<char>('0' + (val & 7));
			val >>= 3;
		}
		std::reverse(buf.begin(), buf.begin() + len);
		return std::string(buf.data(), static_cast<std::size_t>(len));
	}

	// -----------------------------------------------------------------------
	// Hex reading
	// -----------------------------------------------------------------------

	static int hexDigitValue(char c) noexcept {
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'a' && c <= 'f') return c - 'a' + 10;
		if (c >= 'A' && c <= 'F') return c - 'A' + 10;
		return -1;
	}

	uint32_t readHexDigits(std::size_t count) {
		if (pos_ + count > input_.size()) {
			throwError("expected " + std::to_string(count) +
			           " hex digits but input ends early");
		}

		uint32_t value = 0;
		for (std::size_t i = 0; i < count; ++i) {
			char c = input_[pos_++];
			int dig = hexDigitValue(c);
			if (dig < 0) {
				throwError(std::string("invalid hex digit '") + c + "'");
			}
			value = (value << 4) | static_cast<uint32_t>(dig);
		}
		return value;
	}

	// -----------------------------------------------------------------------
	// UTF-8 encoding
	// -----------------------------------------------------------------------

	void appendCodepoint(uint32_t cp) {
		if (cp > 0x10FFFF) {
			throwError("codepoint U+" + toHex(cp) +
			           " exceeds Unicode maximum (U+10FFFF)");
		}
		if (cp >= 0xD800 && cp <= 0xDFFF) {
			throwError("U+" + toHex(cp) +
			           " is a surrogate and cannot be encoded in UTF-8");
		}

		if (cp <= 0x7F) {
			result_ += static_cast<char>(cp);
		} else if (cp <= 0x7FF) {
			result_ += static_cast<char>(0xC0 | (cp >> 6));
			result_ += static_cast<char>(0x80 | (cp & 0x3F));
		} else if (cp <= 0xFFFF) {
			result_ += static_cast<char>(0xE0 | (cp >> 12));
			result_ += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
			result_ += static_cast<char>(0x80 | (cp & 0x3F));
		} else {
			result_ += static_cast<char>(0xF0 | (cp >> 18));
			result_ += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
			result_ += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
			result_ += static_cast<char>(0x80 | (cp & 0x3F));
		}
	}

	// -----------------------------------------------------------------------
	// Escape-sequence handlers
	// -----------------------------------------------------------------------

	void handleSimple(char ch) { result_ += ch; }
	void handleHex2() { appendCodepoint(readHexDigits(2)); }
	void handleHex4() { appendCodepoint(readHexDigits(4)); }
	void handleHex8() { appendCodepoint(readHexDigits(8)); }

	void handleOctal(char firstDigit) {
		uint32_t val = static_cast<uint32_t>(firstDigit - '0');

		// Consume up to 2 more octal digits
		for (int i = 0; i < 2 && hasMore(); ++i) {
			char next = peek();
			if (next < '0' || next > '7') break;
			val = (val << 3) | static_cast<uint32_t>(next - '0');
			++pos_;
		}

		if (val > 0xFF) {
			throwError("octal escape \\" + toOctal(val) +
			           " exceeds single-byte range (max \\377)");
		}
		appendCodepoint(val);
	}

	// -----------------------------------------------------------------------
	// Dispatch
	// -----------------------------------------------------------------------

	/// O(1) lookup table mapping a char after '\' to an action.
	/// Returns false only for unrecognised escape characters.
	bool dispatchEscape(char ch) {
		switch (ch) {
		// Simple single-character escapes
		case 'n': handleSimple('\n'); return true;
		case 'r': handleSimple('\r'); return true;
		case 't': handleSimple('\t'); return true;
		case 'b': handleSimple('\b'); return true;
		case 'f': handleSimple('\f'); return true;
		case 'a': handleSimple('\a'); return true;
		case 'v': handleSimple('\v'); return true;
		case 'e': handleSimple('\x1B'); return true; // ESC
		case '\\': handleSimple('\\'); return true;
		case '"': handleSimple('"'); return true;
		case '\'': handleSimple('\''); return true;

		// Hex escapes
		case 'x': handleHex2(); return true;
		case 'u': handleHex4(); return true;
		case 'U': handleHex8(); return true;

		// Octal escapes
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			handleOctal(ch);
			return true;

		default:
			return false;
		}
	}

	// -----------------------------------------------------------------------
	// Main parse loop
	// -----------------------------------------------------------------------

	std::string parse() {
		while (hasMore()) {
			char c = advance();

			if (c != '\\') {
				result_ += c;
				continue;
			}

			// Will throw on a trailing backslash
			char esc = advance();

			if (!dispatchEscape(esc)) {
				throwError(std::string("unknown escape sequence '\\") +
				           esc + "'");
			}
		}

		return result_; // NRVO applies
	}
};
