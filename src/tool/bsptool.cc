#include "libbsp.hh"
#include "argagg.hh"

#include <unistd.h>

#include <bitset>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char * * argv) {
	
	argagg::parser argp {{
		{ "help",    { "-h", "--help" }, "Show this help message", 0 },
		{ "verbose", { "-v", "--verbose" }, "Print additional info if available", 0 },
		{ "ents",    { "-e", "--ents" }, "Print the entity string", 0 },
		{ "shaders", { "-s", "--shaders" }, "Print the surface shaders", 0 },
	}};
	
	argagg::parser_results args;
	
	try {
		args = argp.parse(argc, argv);
	} catch (std::exception const & e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	
	if (args["help"] || !args.count()) {
		std::cerr << "Usage: bsptool [options] <path to bsp>" << std::endl << argp;
		return 0;
	}
	
	std::filesystem::path bsp_path = args.as<std::string>(0);
	
	if (!std::filesystem::exists(bsp_path)) {
		std::cerr << "File not found!" << std::endl;
		return 1;
	}
	
	auto length = std::filesystem::file_size(bsp_path);
	
	if (!length) {
		std::cerr << "Empty file!" << std::endl;
		return 1;
	}
	
	std::ifstream file { bsp_path, std::ios::binary };
	std::vector<uint8_t> data (length);
	
	file.read( reinterpret_cast<char *>(data.data()), length );
	if(!file.good()) {
		std::cerr << "Failed to read file!" << std::endl;
		return 1;
	}
	
	libbsp::BSP_Reader bspr {data.data()};
	
	if (args["ents"]) {
		std::cout << bspr.entities();
		std::flush(std::cout);
	}
	
	if (args["shaders"]) {
		auto shads = bspr.shaders();
		for (auto const & shad : shads) {
			std::cout << shad.shader << std::endl;
			if (args["verbose"]) {
				std::cout 
					<< "Content Flags: " 
					<< std::bitset<32> {static_cast<size_t>(shad.content_flags)} 
					<< " ( "
					<< static_cast<size_t>(shad.content_flags)
					<< " )"
					<< std::endl
					<< "Surface Flags: " 
					<< std::bitset<32> {static_cast<size_t>(shad.surface_flags)} 
					<< " ( "
					<< static_cast<size_t>(shad.surface_flags)
					<< " )"
					<< std::endl
					<< std::endl;
			}
		}
	}
	
	return 0;
}
