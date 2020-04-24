#include "libbsp.hh"

#include <iostream>
#include <sstream>

using namespace BSP;

static inline bool is_end(char const * & cur, std::string_view const & str) {
	if (!*cur) return true;
	if (cur == str.end()) return true;
	return false;
}

static inline bool char_is_whitespace(char c) {
	switch (c) {
		case ' ':
		case '\n':
		case '\r':
			return true;
		default: return false;
	}
}

static bool skip_whitespace(char const * & cur, std::string_view const & str) {
	while (!is_end(cur, str) && char_is_whitespace(*cur)) cur++;
	return !is_end(cur, str);
}

template <typename T = meadow::istring_view>
static T parse_string(char const * & cur, std::string_view const & str) {
	if (!skip_whitespace(cur, str) || *cur != '\"') throw Reader::ReadException { "expected '\"'" };
	cur++;
	char const * start = cur;
	while (!is_end(cur, str)) {
		switch(*cur) {
			default: 
				cur++;
				break;
			case '\"':
				return T { start, static_cast<size_t>(cur++ - start) }; // FIXME -- better string_view constructor in C++20
		}
	}
	throw Reader::ReadException { "unexpected end of string" };
}

static Reader::Entity parse_entity(char const * & cur, std::string_view const & str) {
	if (!skip_whitespace(cur, str) || *cur != '{') throw Reader::ReadException { "expected '{'" };
	cur++;
	Reader::Entity ent;
	while (skip_whitespace(cur, str) && *cur != '}') {
		// seem to have to separate these out or else they execute in the wrong order????????????
		meadow::istring_view key = parse_string<meadow::istring_view>(cur, str);
		ent[key] = parse_string(cur, str);
	}
	cur++;
	return ent;
}

Reader::EntityArray Reader::parse_entities(std::string_view const & ent_str) {
	
	EntityArray ret;
	
	char const * cur = ent_str.begin();
	while (skip_whitespace(cur, ent_str)) {
		ret.emplace_back(parse_entity(cur, ent_str));
	}
	
	return ret;
}

Reader::EntityArray Reader::entities_parsed() const {
	return parse_entities(entities());
}
