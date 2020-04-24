#include "libbsp/intermediate.hh"

#include <iomanip>
#include <sstream>

BSPI::EntityArray::EntityArray(BSP::Reader::EntityArray const & ents_in) {
	reserve(ents_in.size());
	for (auto const & ent_in : ents_in) {
		auto & ent = emplace_back();
		for (auto const & kvp : ent_in) {
			ent[ meadow::istring {kvp.first} ] = meadow::istring {kvp.second};
		}
	}
}

std::string BSPI::EntityArray::stringify() const {
	std::ostringstream ss;
	for (auto const & ent : *this) {
		ss << "{\n";
		for (auto const & kvp : ent) {
			ss << "\"" << kvp.first << "\" \"" << kvp.second << "\"\n";
		}
		ss << "}\n";
	}
	return ss.str();
}
