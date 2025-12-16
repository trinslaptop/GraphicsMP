#ifndef JSON_HPP
#define JSON_HPP

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <any>
#include <stdexcept>
#include <stdint.h>

/// A standalone JSON parser
/// (c) Trin Wasinger 2025
///
/// NOTE: Currently missing numeric exponent support and serializing of unicode escapes,
/// also slightly more forgiving than it technically should be (e.g. allows trailing commas)
///
/// Based off of:
/// https://github.com/kishore-ganesh/smolJSON
/// https://www.rfc-editor.org/rfc/rfc8259
/// https://www.json.org/json-en.html
namespace json {
	/// A `null` value
	struct NullT {
		private:
			const unsigned char _ = 0;
	};

	using ValueT = std::any;
	using ObjectT = std::map<std::string, std::any>;
	using ListT = std::vector<std::any>;
	using BooleanT = bool;
	using NumberT = double;
	using StringT = std::string;

	namespace is {
		inline bool string(const std::any& value) {
			return value.type() == typeid(std::string);
		}

		inline bool number(const std::any& value) {
			return value.type() == typeid(double);
		}

		inline bool boolean(const std::any& value) {
			return value.type() == typeid(bool);
		}

		inline bool null(const std::any& value) {
			return value.type() == typeid(NullT);
		}

		/// Returns true if `value` does not exists (possible via map indexing with nonexistent key, this is not the same as null)
		inline bool empty(const std::any& value) {
			return !value.has_value();
		}

		inline bool object(const std::any& value) {
			return value.type() == typeid(std::map<std::string, std::any>);
		}

		inline bool list(const std::any& value) {
			return value.type() == typeid(std::vector<std::any>);
		}
	}

	namespace cast {
		inline std::string string(const std::any& value) {
			return std::any_cast<std::string>(value);
		}

		inline double number(const std::any& value) {
			return std::any_cast<double>(value);
		}

		inline bool boolean(const std::any& value) {
			return std::any_cast<bool>(value);
		}

		inline NullT null(const std::any& value) {
			return std::any_cast<NullT>(value);
		}

		inline std::map<std::string, std::any> object(const std::any& value) {
			return std::any_cast<std::map<std::string, std::any>>(value);
		}

		inline std::vector<std::any> list(const std::any& value) {
			return std::any_cast<std::vector<std::any>>(value);
		}
	}

	namespace {
		/// Tokenizes a JSON input stream
		class Tokenizer final {
			private:
				std::istream& _stream;
				std::streampos _lpos;

				static constexpr bool _is_whitespace(const char c) noexcept {
					return c == ' ' || c == '\t' || c == '\n' || c == '\r';
				}

				/// Gets next non whitespace character
				char _next_char() {
					char c = 0;
					do {
						this->_stream.get(c);
						if(!_stream.good()) {
							return this->_is_whitespace(c) ? 0 : c;
						}
					} while(this->_is_whitespace(c));
					return c;
				}
			public:
				// TODO: track position for error messages?
				struct Token final {
					const enum class TokenT {
						TT_LCURL, TT_RCURL, // {}
						TT_LSQR, TT_RSQR, // []
						TT_COLON, // :
						TT_COMMA, // ,
						TT_STRING,
						TT_NUMBER,
						TT_BOOLEAN,
						TT_NULL,
					} type;
					const std::string value;

					std::string get_name() const noexcept {
						switch(this->type) {
							case Tokenizer::Token::TokenT::TT_LCURL:
							case Tokenizer::Token::TokenT::TT_RCURL:
							case Tokenizer::Token::TokenT::TT_LSQR:
							case Tokenizer::Token::TokenT::TT_RSQR:
							case Tokenizer::Token::TokenT::TT_COLON:
							case Tokenizer::Token::TokenT::TT_COMMA:
								return this->value;
							case Tokenizer::Token::TokenT::TT_STRING:
								return "string literal";
							case Tokenizer::Token::TokenT::TT_NUMBER:
								return "number literal";
							case Tokenizer::Token::TokenT::TT_BOOLEAN:
								return "boolean literal";
							case Tokenizer::Token::TokenT::TT_NULL:
								return "null literal";
							default:
								return "unknown";
						}
					}
				};

				Tokenizer(std::istream& stream) : _stream(stream) {}

				// No copying
				Tokenizer (const Tokenizer&) = delete;
				// No copying
				Tokenizer& operator= (const Tokenizer&) = delete;

				/// Steps back a token
				void back() {
					if(this->_stream.eof()) {
						this->_stream.clear();
					}
					this->_stream.seekg(this->_lpos);
				}

				/// Returns true if there are more (possibly invalid) tokens to read
				bool has_more() {
					while(this->_is_whitespace(this->_stream.peek()) && !this->_stream.eof()) this->_stream.get();
					return !this->_stream.eof();
				}

				/// Reads next token
				Token next() {
					char c;
					if(!this->_stream.eof()) {
						this->_lpos = this->_stream.tellg();
						switch(c = this->_next_char()) {
							case '{': return {Token::TokenT::TT_LCURL, "{"};
							case '}': return {Token::TokenT::TT_RCURL, "}"};
							case '[': return {Token::TokenT::TT_LSQR, "["};
							case ']': return {Token::TokenT::TT_RSQR, "]"};
							case ':': return {Token::TokenT::TT_COLON, ":"};
							case ',': return {Token::TokenT::TT_COMMA, ","};
							case 'n':
								this->_stream.seekg(3, std::ios_base::cur);
								return {Token::TokenT::TT_NULL, "null"};
							case 't':
								this->_stream.seekg(3, std::ios_base::cur);
								return {Token::TokenT::TT_BOOLEAN, "true"};
							case 'f':
								this->_stream.seekg(4, std::ios_base::cur);
								return {Token::TokenT::TT_BOOLEAN, "false"};
							// Parse string
							case '"': {
								std::string value;
								while(true) {
									this->_stream.get(c);
									if(c == '\\' && !this->_stream.eof()) {
										this->_stream.get(c);
										switch(c) {
											case '"':
											case '\\':
											case '/':
												value += '"';
												break;
											case 'b':
												value += '\b';
												break;
											case 'f':
												value += '\f';
												break;
											case 'n':
												value += '\n';
												break;
											case 'r':
												value += '\r';
												break;
											case 't':
												value += '\t';
												break;
											case 'u': {
													// Parse unicode code point
													uint16_t code = 0;
													for(size_t n = 0; n < 4; n++) {
														if(this->_stream.eof()) {
															throw std::runtime_error("Unexpected end of unicode escape");
														}

														this->_stream.get(c);
														switch(c) {
															case '0':
															case '1':
															case '2':
															case '3':
															case '4':
															case '5':
															case '6':
															case '7':
															case '8':
															case '9':
																code += (c - '0') << (4*(3-n));
																break;
															case 'a':
															case 'b':
															case 'c':
															case 'd':
															case 'e':
															case 'f':
																code += (0xa + c - 'a') << (4*(3-n));
																break;
															case 'A':
															case 'B':
															case 'C':
															case 'D':
															case 'E':
															case 'F':
																code += (0xa + c - 'A') << (4*(3-n));
																break;
														}
													}

													// Convert to chars (https://en.wikipedia.org/wiki/UTF-8)
													if(code < 0x007f) {
														value += {(char) (code & 0b01111111)};
													} else if(code < 0x07ff) {
														value += {(char)(0b11000000 | (code >> 6)), (char)(0b10000000 | (code & 0b00111111))};
													} else {
														value += {(char)(0b11100000 | (code >> 12)), (char)(0b10000000 | ((code >> 6) & 0b00111111)), (char)(0b10000000 | (code & 0b00111111))};
													}
												}
												break;
											default:
												throw std::runtime_error("Unknown escape sequence '\\" + std::to_string(c) + "'");
										}
									} else if(c == '"') {
										break;
									} else if(this->_stream.eof()) {
										throw std::runtime_error("Unexpected end of string");
									} else {
										value += c;
									}
								}
								return {Token::TokenT::TT_STRING, value};
							}
							// Parse number
							case '-':
							case '0':
							case '1':
							case '2':
							case '3':
							case '4':
							case '5':
							case '6':
							case '7':
							case '8':
							case '9': {
								std::string value = {c};
								std::streampos t;
								while(true) {
									t = this->_stream.tellg();
									this->_stream.get(c);
									if(this->_stream.eof()) {
										break;
									} else if((c >= '0' && c <= '9') || c == '.') {
										value += c;
									} else {
										this->_stream.seekg(t);
										break;
									}
								}
								return {Token::TokenT::TT_NUMBER, value};
							}
						}
					}
					throw std::runtime_error("Unexpected character '" + (c == 0 ? std::string("EOF") : std::to_string(c)) + "'");
				}
		};

		/// Parses an arbitrary set of JSON tokens into a value
		std::any _parse(Tokenizer& tokenizer);
		
		/// Parses a JSON list after the opening '[', permits trailing commas
		std::vector<std::any> _parse_list(Tokenizer& tokenizer);
		
		/// Parses a JSON object after the opening '{', permits trailing commas
		std::map<std::string, std::any> _parse_object(Tokenizer& tokenizer);

		/// Parses a JSON object after the opening '{', permits trailing commas
		std::map<std::string, std::any> _parse_object(Tokenizer& tokenizer) {
			std::map<std::string, std::any> object;
			while(tokenizer.has_more()) {
				// TT_LCURL already consumed

				const Tokenizer::Token first = tokenizer.next();
				switch(first.type) {
					case Tokenizer::Token::TokenT::TT_RCURL: return object;
					case Tokenizer::Token::TokenT::TT_STRING: break;
					default: throw std::runtime_error("Got unexpected token '" + first.get_name() + "' ('" + first.value + "'), expected 'string literal' or '}'");
				}
				
				const Tokenizer::Token colonToken = tokenizer.next();
				switch(colonToken.type) {
					case Tokenizer::Token::TokenT::TT_COLON: break;
					default: throw std::runtime_error("Got unexpected token '" + colonToken.get_name() + "' ('" + colonToken.value + "'), expected ':'");
				}

				object[first.value] = _parse(tokenizer);

				const Tokenizer::Token nextToken = tokenizer.next();
				switch(nextToken.type) {
					case Tokenizer::Token::TokenT::TT_COMMA: break;
					case Tokenizer::Token::TokenT::TT_RCURL: return object;
					default: throw std::runtime_error("Got unexpected token '" + nextToken.get_name() + "' ('" + nextToken.value + "'), expected ',' or '}'");
				}
			}
			throw std::runtime_error("Unexpected end of object");
		}

		/// Parses a JSON list after the opening '[', permits trailing commas
		std::vector<std::any> _parse_list(Tokenizer& tokenizer) {
			std::vector<std::any> list;
			while(tokenizer.has_more()) {
				// TT_LSQR already consumed
				
				const Tokenizer::Token first = tokenizer.next();
				switch(first.type) {
					case Tokenizer::Token::TokenT::TT_RSQR: return list;
					default:
						tokenizer.back();
						list.push_back(_parse(tokenizer));
				}

				const Tokenizer::Token last = tokenizer.next();
				switch(last.type) {
					case Tokenizer::Token::TokenT::TT_COMMA: break;
					case Tokenizer::Token::TokenT::TT_RSQR: return list;
					default: throw std::runtime_error("Got unexpected token '" + last.get_name() + "' ('" + last.value + "'), expected ',' or ']'");
				}
			}
			throw std::runtime_error("Unexpected end of array");
		}

		/// Parses an arbitrary set of JSON tokens into a value
		std::any _parse(Tokenizer& tokenizer) {
			while(tokenizer.has_more()) {
				const Tokenizer::Token token = tokenizer.next();
				switch(token.type) {
					case Tokenizer::Token::TokenT::TT_LCURL: return _parse_object(tokenizer);
					case Tokenizer::Token::TokenT::TT_LSQR: return _parse_list(tokenizer);
					case Tokenizer::Token::TokenT::TT_STRING: return token.value;
					case Tokenizer::Token::TokenT::TT_NUMBER: return std::stod(token.value);
					case Tokenizer::Token::TokenT::TT_BOOLEAN: return token.value == "true";
					case Tokenizer::Token::TokenT::TT_NULL: return NullT {};
					default: throw std::runtime_error("Got unexpected token '" + token.get_name() + "' ('" + token.value + "')");
				}
			}
			return NullT {};
		}

		/// Stringifies a value to JSON, unsupported types are replaced with `null`
		std::string _stringify(const std::any& value, const size_t indent = 1) {
			if(value.type() == typeid(double)) {
				return std::to_string(std::any_cast<double>(value));
			} else if(value.type() == typeid(bool)) {
				return std::any_cast<bool>(value) ? "true" : "false";
			} else if(value.type() == typeid(std::string)) {
				std::string str = "\"";
				for(const char c : std::any_cast<std::string>(value)) {
					switch(c) {
						case '"':
						case '\\':
						case '/':
							str += "\\" + c;
							break;
						case '\b':
							str += "\\b";
							break;
						case '\f':
							str += "\\f";
							break;
						case '\n':
							str += "\\n";
							break;
						case '\r':
							str += "\\r";
							break;
						case '\t':
							str += "\\t";
							break;
						// TODO: Unicode escapes
						default:
							str += c;
							break;
					}
				}
				return str + "\"";
			} else if(value.type() == typeid(NullT)) {
				return "null";
			} else if(value.type() == typeid(std::vector<std::any>)) {
				std::string str = "[";
				for(const std::any& item : std::any_cast<std::vector<std::any>>(value)) {
					str += (indent ? "\n" + std::string(4*indent, ' ') : "") + _stringify(item, indent ? indent + 1 : indent) + ",";
				}
				if(std::any_cast<std::vector<std::any>>(value).size()) {
					str.pop_back();
					str += (indent ? "\n" : "");
				}
				return str + std::string(indent ? 4*(indent-1) : 0, ' ') + "]";
			} else if(value.type() == typeid(std::map<std::string, std::any>)) {
				std::string str = "{";
				for(const auto& entry : std::any_cast<std::map<std::string, std::any>>(value)) {
					str += (indent ? "\n" + std::string(4*indent, ' ') : "") + _stringify(entry.first) + ":" + (indent ? " " : "") + _stringify(entry.second, indent ? indent + 1 : indent) + ",";
				}
				if(std::any_cast<std::map<std::string, std::any>>(value).size()) {
					str.pop_back();
					str += (indent ? "\n" : "");
				}
				return str + std::string(indent ? 4*(indent-1) : 0, ' ') + "}";
			} else {
				return "null";
			}
		}
	}

	/// Parse a JSON input stream into a value
	inline std::any parse(std::istream& stream) {
		Tokenizer tokenizer(stream);
		return _parse(tokenizer);
	}

	/// Parse a JSON string into a value
	inline std::any parse(const std::string& text) {
		std::istringstream stream(text);
		return parse(stream);
	}

	/// Stringifies a value to JSON, unsupported types are replaced with `null`
	inline std::string stringify(const std::any& value, const bool indented = true) {
		return _stringify(value, indented);
	}
}
#endif