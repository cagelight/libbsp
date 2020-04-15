#include "libbsp.hh"
#include "argagg.hh"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <bitset>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char * * argv) {
	
	argagg::parser argp {{
		{ "help",    { "-h", "--help" }, "Show this help message", 0 },
		{ "verbose", { "-v", "--verbose" }, "Print additional info if available", 0 },
		{ "ents",    { "-e", "--ents" }, "Print the entity string", 0 },
		{ "shaders", { "-s", "--shaders" }, "Print the surface shaders", 0 },
		{ "info",    { "-i", "--info" }, "Print general info", 0 },
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
	
	std::string bsp_path = args.as<std::string>(0);
	auto fd = open( bsp_path.c_str(), O_RDONLY );
	
	if (fd == -1) {
		std::cerr << "File not found!" << std::endl;
		return 1;
	}
	
	struct stat sb;
	fstat(fd, &sb);
	
	if (sb.st_size < (ssize_t)sizeof(libbsp::fmt::BSP_Header)) {
		std::cerr << "File too small to be a BSP file!" << std::endl;
		return 1;
	}

	auto ptr = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	
	libbsp::BSP_Reader bspr { reinterpret_cast<uint8_t const *> (ptr) };
	
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
	
	if (args["info"]) {
		std::cout
			<< bspr.entities().size() << " entity string bytes" << std::endl
			<< bspr.shaders().size() << " shaders" << std::endl
			<< bspr.planes().size() << " planes" << std::endl
		;
	}
	
	return 0;
}
