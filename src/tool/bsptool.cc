#include "libbsp.hh"
#include "argagg.hh"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <filesystem>

static int32_t add_shader(BSPI::ShaderArray & shaders, meadow::istring_view name) {
		
		int32_t shdst = 0;
		static constexpr int32_t content_flags_default = 32769;
		static constexpr int32_t surface_flags_default = 0;
		
		auto shiter = std::find_if(shaders.begin(), shaders.end(), [&](BSPI::Shader const & v){ return name == v.path; });
		if (shiter == shaders.end()) {
			shdst = shaders.size();
			shaders.emplace_back( BSPI::Shader { name.data(), surface_flags_default, content_flags_default } );
		}
		else
			shdst = std::distance(shaders.begin(), shiter);
		
		return shdst;
}

int main(int argc, char * * argv) {
	
	argagg::parser argp {{
		{ "help",      { "-h", "--help" }, "Show this help message", 0 },
		{ "info",      { "-i", "--info" }, "Print general info", 0 },
		{ "info+",     { "-I", "--info-extra" }, "Print general+advanced info", 0 },
		{ "entstr",    { "-e", "--entstring" }, "Print the entity string", 0 },
		{ "ents",      { "-E", "--ents" }, "Print information about the entities", 0 },
		{ "shaders",   { "-s", "--shaders" }, "Print the shaders used", 0 },
		{ "shaders+",  { "-S", "--shaders-extra" }, "Print the shaders used plus extra information", 0 },
		{ "reprocess", { "-r", "--reprocess" }, "Load the BSP and resave it", 0 },
		{ "lmdump",    { "-L", "--lmdump" }, "Dump all lightmaps", 0 },	
		
		{ "shsurfs",   { "--shader-surfaces" }, "<shader>", 0 },
		{ "remap",     { "--remap" }, "requires (--src or --idx), --dst, and -o to be specified", 0 },
		{ "uvbound",   { "--uvbound" }, "requires --idx and -o to be specified", 0 },		
		{ "lmsecret",  { "--lmsecret" }, "requires --idx, --src, and -o to be specified", 0 },
		{ "lmsecret2", { "--lmsecret2" }, "requires --idx, --src, and -o to be specified, parameter is flipping bitfield", 1 },
		{ "lmsecret3", { "--lmsecret3" }, "requires --src, -x, -y, -z, --size, and -o to be specified, parameter is which plane (x, y, z) to create the secret on", 1 },
		{ "nsbrush",   { "--nsbrush" }, "sets a brush to be non-solid, requires --idx and -o to be specified", 0 },
		{ "rmsurf",    { "--rmsurf" }, "removes a surface (sets the vertex count to zero, does not permanently remove data), requires --idx and -o to be specified", 0 }, // TODO
		
		{ "output",    { "-o", "--output" }, "Output path for saving operations", 1 },
		{ "src",       { "--src" }, "<source shader name>", 1 },
		{ "dst",       { "--dst" }, "<dest shader name>", 1 },
		{ "idx",       { "--idx" }, "<index>", 1 },
		{ "size",      { "--size" }, "<size>", 1 },
		{ "model",     { "--model" }, "<model index>", 1 },
		{ "x",		   { "--x" }, "<x>", 1 },
		{ "y",	       { "--y" }, "<y>", 1 },
		{ "z",	       { "--z" }, "<z>", 1 },
	}};
	
	argagg::parser_results args;
	
	try {
		args = argp.parse(argc, argv);
	} catch (std::exception const & e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	
	// ================================
	// HELP
	// ================================
	
	if (args["help"] || !args.count()) {
		std::cerr << "Usage: bsptool [options] <path to bsp>" << std::endl << argp;
		return 0;
	}
	
	// ================================
	// SETUP
	// ================================
	
	std::string bsp_path = args.as<std::string>(0);
	
	std::string output_path;
	if (args["output"])
		output_path = args["output"].as<std::string>();
	else
		output_path = bsp_path;
	
	auto fdr = open( bsp_path.c_str(), O_RDONLY );
	
	if (fdr == -1) {
		std::cerr << "File not found!" << std::endl;
		return 1;
	}
	
	struct stat sb;
	fstat(fdr, &sb);
	
	if (sb.st_size < (ssize_t)sizeof(BSP::Header)) {
		std::cerr << "File too small to be a BSP file!" << std::endl;
		return 1;
	}

	BSP::Reader bspr;
	std::vector<uint8_t> rdat;
	
	if (!std::filesystem::equivalent(bsp_path, output_path))
		bspr.rebase(reinterpret_cast<uint8_t const *> (mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fdr, 0)));
	else {
		rdat.resize(sb.st_size);
		read(fdr, rdat.data(), sb.st_size);
		close(fdr);
		bspr.rebase(rdat.data());
	}
	
	if (bspr.header().ident != BSP::IDENT) {
		std::cerr << "File does not appear to be a BSP file!" << std::endl;
		return 1;
	}
	
	// ================================
	// INFO
	// ================================
	
	if (args["info"]) {
		
		std::unordered_map<BSP::SurfaceType, uint_fast32_t> surface_type_uage;
		for (auto const & surf : bspr.surfaces()) surface_type_uage[surf.type]++;
		
		std::cout
			<< sb.st_size << " byte BSP file" << std::endl
			<< bspr.entities().size() << " entity string bytes" << std::endl
			
			<< bspr.shaders().size() << " shaders"
			<< " (" << bspr.shaders().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.planes().size() << " planes"
			<< " (" << bspr.planes().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.nodes().size() << " nodes" 
			<< " (" << bspr.nodes().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.leafs().size() << " leafs"
			<< " (" << bspr.leafs().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.leafsurfaces().size() << " leafsurfaces"
			<< " (" << bspr.leafsurfaces().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.leafbrushes().size() << " leafbrushes"
			<< " (" << bspr.leafbrushes().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.models().size() << " models"
			<< " (" << bspr.models().size_bytes() << " bytes" << ")" 
			<< std::endl;
			
		uint64_t total_side_refs = 0;
		int32_t highest_side_count = 0;
		
		for (auto b : bspr.brushes()) {
			total_side_refs += b.num_sides;
			if (b.num_sides > highest_side_count) highest_side_count = b.num_sides;
		}
		
		std::cout
			<< bspr.brushes().size() << " brushes"
			<< " (" << bspr.brushes().size_bytes() << " bytes" << ")" 
			<< std::endl
			<< "    "
			<< "highest number of sides: "
			<< highest_side_count
			<< ", average side count: "
			<< std::setprecision(4)
			<< static_cast<double>(total_side_refs) / bspr.brushes().size()
			<< std::endl;
			
		std::cout
			<< bspr.brushsides().size() << " brushsides"
			<< " (" << bspr.brushsides().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.drawverts().size() << " drawverts"
			<< " (" << bspr.drawverts().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.drawindices().size() << " drawindexes"
			<< " (" << bspr.drawindices().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.fogs().size() << " fogs"
			<< " (" << bspr.fogs().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.surfaces().size() << " surfaces"
			<< " (" << bspr.surfaces().size_bytes() << " bytes" << ")" 
			<< std::endl
			<< "    "
			<< surface_type_uage[BSP::SurfaceType::PLANAR] << " planars, "
			<< surface_type_uage[BSP::SurfaceType::PATCH] << " patches, "
			<< surface_type_uage[BSP::SurfaceType::TRISOUP] << " trisoups, "
			<< surface_type_uage[BSP::SurfaceType::FLARE] << " flares"
			<< std::endl
			
			<< bspr.lightmaps().size() << " lightmaps"
			<< " (" << bspr.lightmaps().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.lightgrids().size() << " lightgrid elements"
			<< " (" << bspr.lightgrids().size_bytes() << " bytes" << ")" 
			<< std::endl
		;
		
		if (bspr.has_visibility()) {
			std::cout
				<< bspr.visibility().data.size() << " bytes of visibility data"
				<< std::endl
				<< "    "
				<< bspr.visibility().header.clusters << " clusters, "
				<< bspr.visibility().header.cluster_bytes << " bytes per cluster"
				<< std::endl
			;
		} else {
			std::cout << "no visibility data" << std::endl;
		}
			
		std::cout
			<< bspr.lightarray().size() << " lightarray elements"
			<< " (" << bspr.lightarray().size_bytes() << " bytes" << ")" 
			<< std::endl
		;
	}
	
	// ================================
	// INFO+
	// ================================
	
	if (args["info+"]) {
		
		std::unordered_map<BSP::SurfaceType, uint_fast32_t> surface_type_uage;
		for (auto const & surf : bspr.surfaces()) surface_type_uage[surf.type]++;
		
		std::cout
			<< sb.st_size << " byte BSP file" << std::endl
			<< bspr.entities().size() << " entity string bytes" << std::endl
			
			<< bspr.shaders().size() << " shaders"
			<< " (" << bspr.shaders().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.planes().size() << " planes"
			<< " (" << bspr.planes().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.nodes().size() << " nodes" 
			<< " (" << bspr.nodes().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.leafs().size() << " leafs"
			<< " (" << bspr.leafs().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.leafsurfaces().size() << " leafsurfaces"
			<< " (" << bspr.leafsurfaces().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.leafbrushes().size() << " leafbrushes"
			<< " (" << bspr.leafbrushes().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.models().size() << " models"
			<< " (" << bspr.models().size_bytes() << " bytes" << ")" 
			<< std::endl;
			
		uint64_t total_side_refs = 0;
		int32_t highest_side_count = 0;
		
		for (auto b : bspr.brushes()) {
			total_side_refs += b.num_sides;
			if (b.num_sides > highest_side_count) highest_side_count = b.num_sides;
		}
		
		std::cout
			<< bspr.brushes().size() << " brushes"
			<< " (" << bspr.brushes().size_bytes() << " bytes" << ")" 
			<< std::endl
			<< "    "
			<< "highest number of sides: "
			<< highest_side_count
			<< ", average side count: "
			<< std::setprecision(4)
			<< static_cast<double>(total_side_refs) / bspr.brushes().size()
			<< std::endl;
			
		std::cout
			<< bspr.brushsides().size() << " brushsides"
			<< " (" << bspr.brushsides().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.drawverts().size() << " drawverts"
			<< " (" << bspr.drawverts().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.drawindices().size() << " drawindexes"
			<< " (" << bspr.drawindices().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.fogs().size() << " fogs"
			<< " (" << bspr.fogs().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.surfaces().size() << " surfaces"
			<< " (" << bspr.surfaces().size_bytes() << " bytes" << ")" 
			<< std::endl
			<< "    "
			<< surface_type_uage[BSP::SurfaceType::PLANAR] << " planars, "
			<< surface_type_uage[BSP::SurfaceType::PATCH] << " patches, "
			<< surface_type_uage[BSP::SurfaceType::TRISOUP] << " trisoups, "
			<< surface_type_uage[BSP::SurfaceType::FLARE] << " flares"
			<< std::endl
			
			<< bspr.lightmaps().size() << " lightmaps"
			<< " (" << bspr.lightmaps().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.lightgrids().size() << " lightgrid elements"
			<< " (" << bspr.lightgrids().size_bytes() << " bytes" << ")" 
			<< std::endl
		;
		
		if (bspr.has_visibility()) {
			std::cout
				<< bspr.visibility().data.size() << " bytes of visibility data"
				<< std::endl
				<< "    "
				<< bspr.visibility().header.clusters << " clusters, "
				<< bspr.visibility().header.cluster_bytes << " bytes per cluster"
				<< std::endl
			;
		} else {
			std::cout << "no visibility data" << std::endl;
		}
			
		std::cout
			<< bspr.lightarray().size() << " lightarray elements"
			<< " (" << bspr.lightarray().size_bytes() << " bytes" << ")" 
			<< std::endl
		;
		
		std::cout << std::endl << std::endl;
		
		std::array<int32_t, 3> leafs_mins, leafs_maxs;
		BSP::Reader::LeafArray leafs = bspr.leafs();
		leafs_mins = { leafs[0].mins[0], leafs[0].mins[1], leafs[0].mins[2] };
		leafs_maxs = { leafs[0].maxs[0], leafs[0].maxs[1], leafs[0].maxs[2] };
		for (size_t i = 1; i < leafs.size(); i++) {
			for (size_t j = 0; j < 3; j++) {
				if (leafs[i].mins[j] < leafs_mins[j]) leafs_mins[j] = leafs[i].mins[j];
				if (leafs[i].maxs[j] > leafs_maxs[j]) leafs_maxs[j] = leafs[i].maxs[j];
			}
		}
		std::cout
			<< "Overall Minimum Bounds:"
			<< std::endl
			<< "    " << leafs_mins[0] << ", " << leafs_mins[1] << ", " << leafs_mins[2]
			<< std::endl
			<< "Overall Maximum Bounds:"
			<< std::endl
			<< "    " << leafs_maxs[0] << ", " << leafs_maxs[1] << ", " << leafs_maxs[2]
			<< std::endl
		;
	}
	
	// ================================
	// ENTSTR
	// ================================
	
	if (args["entstr"]) {
		std::cout << bspr.entities();
		std::flush(std::cout);
	}
	
	// ================================
	// ENTS
	// ================================
	
	if (args["ents"]) {
		auto ents = bspr.entities_parsed();
		
		std::cout << ents.size() << " entities" << std::endl;
		
		size_t classless = 0;
		std::map<meadow::istring_view, std::vector<BSP::Reader::Entity const *>> classnames;
		for (auto const & ent : ents) {
			auto iter = ent.find(meadow::istring_view("classname"));
			if (iter == ent.end()) {
				classless++;
				continue;
			} else {
				classnames[iter->second].emplace_back(&ent);
			}
		}
		
		std::cout << "classes:" << std::endl;
		for (auto const & cl : classnames) std::cout << "    " << cl.first << ": " << cl.second.size() << std::endl;
		if (classless) {
			std::cout << "    " << classless << " classless entities" << std::endl;
		}
	}
	
	// ================================
	// SHADERS
	// ================================
	
	if (args["shaders"]) {
		for (auto const & shad : bspr.shaders()) std::cout << shad.shader << std::endl;
	}
	
	// ================================
	// SHADERS+
	// ================================
	
	if (args["shaders+"]) {
		
		auto shads = bspr.shaders();
		
		std::unordered_map<int32_t, uint32_t> brush_usage;
		for (auto const & v : bspr.brushes()) brush_usage[v.shader]++;
		
		std::unordered_map<int32_t, uint32_t> brushside_usage;
		for (auto const & v : bspr.brushsides()) brushside_usage[v.shader]++;
		
		std::unordered_map<int32_t, uint32_t> surface_usage;
		for (auto const & v : bspr.surfaces()) surface_usage[v.shader]++;
		
		for (size_t i = 0; i < shads.size(); i++) {
			auto const & shad = shads[i];
			std::cout 
				<< "Shader Path:     " << shad.shader 
				<< std::endl
				<< "Brush Usage:     " << brush_usage[i]
				<< std::endl
				<< "Brushside Usage: " << brushside_usage[i]
				<< std::endl
				<< "Surface Usage:   " << surface_usage[i]
				<< std::endl
				<< "Content Flags:   " 
				<< std::bitset<32> {static_cast<size_t>(shad.content_flags)} 
				<< " ( " << static_cast<size_t>(shad.content_flags) << " )"
				<< std::endl
				<< "Surface Flags:   "
				<< std::bitset<32> {static_cast<size_t>(shad.surface_flags)}
				<< " ( " << static_cast<size_t>(shad.surface_flags) << " )"
				<< std::endl
				<< std::endl;
		}
	}
	
	// ================================
	// REPROCESS
	// ================================
	
	if (args["reprocess"]) {
		BSP::LumpProviderPtr pprov = std::make_shared<BSP::BSPReaderLumpProvider>(bspr);
		BSP::Assembler bspa { pprov };
		auto bytes = bspa.assemble();
		
		std::ofstream f { (std::string) args["reprocess"][0], std::ios_base::binary | std::ios_base::out };
		if (!f.good()) {
			// TODO error
			return 1;
		}
		f.write( reinterpret_cast<char const *>(bytes.data()), bytes.size());
		f.close();
	}
	
	// ================================
	// LMDUMP
	// ================================
	
	if (args["lmdump"]) {
		for (size_t i = 0; i < bspr.lightmaps().size(); i++) {
			auto const & lm = bspr.lightmaps()[i];
			std::stringstream ss;
			ss << "lm_";
			ss << i;
			ss << ".png";
			stbi_write_png(ss.str().data(), 128, 128, 3, &lm.pixels[0], 3 * 128);
		}
	}
	
	// ================================
	// SHSURFS
	// ================================
	
	if (args["shsurfs"]) {
		meadow::istring src = meadow::s2i(args["src"].as<std::string>());
		std::vector<BSP::Surface const *> matchsurfs;
		for (BSP::Surface const & surf : bspr.surfaces())
			if (bspr.shaders()[surf.shader].shader == src) matchsurfs.emplace_back(&surf);
		for (size_t i = 0; i < matchsurfs.size(); i++) {
			BSP::Surface const & surf = *matchsurfs[i];
			std::cout
				<< "[" << i << "] "
				<< "Vertices: " << surf.vert_count
			<< std::endl;
		}
	}
	
	// ================================
	// REMAP
	// ================================
	
	if (args["remap"]) {
		
		meadow::istring src;
		meadow::istring dst = meadow::s2i(args["dst"].as<std::string>());
		int32_t idx;
		
		if (args["idx"]) {
			idx = args["idx"].as<int32_t>();
		} else if (args["src"]) {
			src = meadow::s2i(args["src"].as<std::string>());
			idx = -1;
		} else {
			return 1;
		}
		
		if (!output_path.size()) {
			return 1;
		}
		
		if (idx < -1) return 1;
		
		BSP::LumpProviderPtr pprov = std::make_shared<BSP::BSPReaderLumpProvider>(bspr);
		BSP::Assembler bspa { pprov };
		
		std::shared_ptr<BSPI::ShaderArray> shaders = std::make_shared<BSPI::ShaderArray>(bspr.shaders());
		bspa[BSP::LumpIndex::SHADERS] = std::make_shared<BSP::BSPIShaderArrayLumpProvider>(shaders);
		
		if (idx == -1) {
			auto shiter = std::find_if(shaders->begin(), shaders->end(), [&](BSPI::Shader const & v){ return src == v.path; });
			if (shiter == shaders->end()) {
				std::cerr << "source shader not found in BSP" << std::endl;
				return 1;
			}
			shiter->path = dst;
			
		} else {
			
			std::shared_ptr<BSPI::SurfaceArray> surfaces = std::make_shared<BSPI::SurfaceArray>(bspr.surfaces());
			bspa[BSP::LumpIndex::SURFACES] = std::make_shared<BSP::BSPISurfaceArrayLumpProvider>(surfaces);
			
			int32_t shdst = add_shader(*shaders, dst);
			
			if ((size_t)idx >= surfaces->size()) {
				std::cerr << "remap index greater than surfaces" << std::endl;
				return 1;
			}
			
			surfaces->at(idx).shader = shdst;
		}
		
		auto bytes = bspa.assemble();
		
		std::ofstream fout { output_path, std::ios_base::binary | std::ios_base::out };
		if (!fout.good()) {
			std::cerr << "failed to open output for writing" << std::endl;
			return 1;
		}
		fout.write( reinterpret_cast<char const *>(bytes.data()), bytes.size());
		fout.close();
	}
	
	// ================================
	// UVBOUND
	// ================================
	
	if (args["uvbound"]) {
		
		if (!args["idx"]) return -1;
		int32_t idx = args["idx"].as<int32_t>();
		
		BSP::LumpProviderPtr pprov = std::make_shared<BSP::BSPReaderLumpProvider>(bspr);
		BSP::Assembler bspa { pprov };
		std::shared_ptr<BSPI::VertexArray> vertices = std::make_shared<BSPI::VertexArray>(bspr.drawverts());
		bspa[BSP::LumpIndex::DRAWVERTS] = std::make_shared<BSP::BSPIVertexArrayLumpProvider>(vertices);
		
		auto const & surf = bspr.surfaces()[idx];
		
		std::array<float, 2> uv_min, uv_max;
		
		BSPI::VertexArray::value_type * vert = &vertices->at(surf.vert_idx);
		uv_max[0] = uv_min[0] = vert->uv[0];
		uv_max[1] = uv_min[1] = vert->uv[1];
		vert++;
		for (int32_t i = 1; i < surf.vert_count; i++, vert++) {
			if      (vert->uv[0] < uv_min[0]) uv_min[0] = vert->uv[0];
			else if (vert->uv[0] > uv_max[0]) uv_max[0] = vert->uv[0];
			if      (vert->uv[1] < uv_min[1]) uv_min[1] = vert->uv[1];
			else if (vert->uv[1] > uv_max[1]) uv_max[1] = vert->uv[1];
		}
		
		std::array<float, 2> diff = { uv_max[0] - uv_min[0], uv_max[1] - uv_min[1] };
		vert = &vertices->at(surf.vert_idx);
		for (int32_t i = 0; i < surf.vert_count; i++, vert++) {
			vert->uv[0] = (vert->uv[0] - uv_min[0]) / diff[0];
			vert->uv[1] = (vert->uv[1] - uv_min[1]) / diff[1];
		}
		
		auto bytes = bspa.assemble();
		
		std::ofstream fout { output_path, std::ios_base::binary | std::ios_base::out };
		if (!fout.good()) {
			std::cerr << "failed to open output for writing" << std::endl;
			return 1;
		}
		fout.write( reinterpret_cast<char const *>(bytes.data()), bytes.size());
		fout.close();
	}
	
	// ================================
	// LMSECRET
	// ================================
	
	if (args["lmsecret"] || args["lmsecret2"]) {
		
		bool flip_x = false, flip_y = false;
		if (args["lmsecret2"]) {
			int parm = args["lmsecret2"].as<int>();
			flip_x = parm & 1;
			flip_y = parm & 2;
		}
		
		if (!args["idx"]) return -1;
		if (!args["src"]) return -1;
		std::string imgf = args["src"].as<std::string>();
		int32_t idx = args["idx"].as<int32_t>();
		
		int32_t xmult = 1;
		if (args["x"]) xmult = args["x"].as<int32_t>();
		
		BSP::LumpProviderPtr pprov = std::make_shared<BSP::BSPReaderLumpProvider>(bspr);
		BSP::Assembler bspa { pprov };
		auto shaders = std::make_shared<BSPI::ShaderArray>(bspr.shaders());
		auto surfaces = std::make_shared<BSPI::SurfaceArray>(bspr.surfaces());
		auto brushsides = std::make_shared<BSPI::BrushSideArray>(bspr.brushsides());
		auto vertices = std::make_shared<BSPI::VertexArray>(bspr.drawverts());
		auto lightmaps = std::make_shared<BSPI::LightmapArray>(bspr.lightmaps());
		bspa[BSP::LumpIndex::SHADERS] = std::make_shared<BSP::BSPIShaderArrayLumpProvider>(shaders);
		bspa[BSP::LumpIndex::SURFACES] = std::make_shared<BSP::BSPISurfaceArrayLumpProvider>(surfaces);
		bspa[BSP::LumpIndex::BRUSHSIDES] = std::make_shared<BSP::BSPIBrushSidesArrayLumpProvider>(brushsides);
		bspa[BSP::LumpIndex::DRAWVERTS] = std::make_shared<BSP::BSPIVertexArrayLumpProvider>(vertices);
		bspa[BSP::LumpIndex::LIGHTMAPS] = std::make_shared<BSP::BSPILightmapArrayLumpProvider>(lightmaps);
		
		auto & surf = surfaces->at(idx);
		surf.shader = add_shader(*shaders, "textures/colors/white2");
		
		for (auto & bs : *brushsides) {
			if (bs.surface == idx) {
				bs.shader = surf.shader;
				break;
			}
		}
		
		BSPI::VertexArray::value_type * vert;
		std::array<float, 2> uv_min, uv_max, diff;
		
		while (true) {
			
			BSPI::VertexArray::value_type * vert = &vertices->at(surf.vert_idx);
			uv_max[0] = uv_min[0] = vert->lightmap[0][0];
			uv_max[1] = uv_min[1] = vert->lightmap[0][1];
			vert++;
			for (int32_t i = 1; i < surf.vert_count; i++, vert++) {
				if      (vert->lightmap[0][0] < uv_min[0]) uv_min[0] = vert->lightmap[0][0];
				else if (vert->lightmap[0][0] > uv_max[0]) uv_max[0] = vert->lightmap[0][0];
				if      (vert->lightmap[0][1] < uv_min[1]) uv_min[1] = vert->lightmap[0][1];
				else if (vert->lightmap[0][1] > uv_max[1]) uv_max[1] = vert->lightmap[0][1];
			}
			
			diff = { uv_max[0] - uv_min[0], uv_max[1] - uv_min[1] };
			
			if (diff[0] < 0.001 || diff[1] < 0.001) { // probably no lightmap UVs, hack it from the position and redo
				vert = &vertices->at(surf.vert_idx);
				for (int32_t i = 0; i < surf.vert_count; i++, vert++) {
					vert->lightmap[0][0] = vert->pos[0] + vert->pos[1];
					vert->lightmap[0][1] = vert->pos[2];
				}
				continue;
			}
			break;
		}
		
		vert = &vertices->at(surf.vert_idx);
		for (int32_t i = 0; i < surf.vert_count; i++, vert++) {
			vert->lightmap[0][0] = (vert->lightmap[0][0] - uv_min[0]) / diff[0];
			vert->lightmap[0][1] = 1 - (vert->lightmap[0][1] - uv_min[1]) / diff[1];
			
			if (xmult > 1) vert->lightmap[0][0] *= xmult;
			if (flip_x) vert->lightmap[0][0] = 1 - vert->lightmap[0][0];
			if (flip_y) vert->lightmap[0][1] = 1 - vert->lightmap[0][1];
		}
		
		surf.lightmap[0] = lightmaps->size();
		auto & lm = lightmaps->emplace_back();
		
		int img_w, img_h, img_ch;
		auto * img_data = stbi_load(imgf.data(), &img_w, &img_h, &img_ch, 3);
		if (!img_data || img_w != 128 || img_h != 128 || img_ch != 3) {
			std::cerr << "src image MUST be 128x128 RGB" << std::endl;
			return 1;
		}
		
		auto * img_ptr = img_data;
		for (size_t y = 0; y < 128; y++) for (size_t x = 0; x < 128; x++) {
			lm.pixels[y][x].r = img_ptr[0];
			lm.pixels[y][x].g = img_ptr[1];
			lm.pixels[y][x].b = img_ptr[2];
			img_ptr += 3;
		}
		
		stbi_image_free(img_data);
		
		auto bytes = bspa.assemble();
		
		std::ofstream fout { output_path, std::ios_base::binary | std::ios_base::out };
		if (!fout.good()) {
			std::cerr << "failed to open output for writing" << std::endl;
			return 1;
		}
		fout.write( reinterpret_cast<char const *>(bytes.data()), bytes.size());
		fout.close();
	}
	
	// ================================
	// LMSECRET3
	// ================================
	
	if (args["lmsecret3"]) {
		
		if (!args["src"] || !args["x"] || !args["y"] || !args["z"] || !args["size"]) return -1;
		std::string imgf = args["src"].as<std::string>();
		int32_t x = args["x"].as<int32_t>();
		int32_t y = args["y"].as<int32_t>();
		int32_t z = args["z"].as<int32_t>();
		int32_t size = args["size"].as<int32_t>();
		std::string dir = args["lmsecret3"].as<std::string>();
		
		int32_t midx = 0;
		
		if (args["model"])
			midx = args["model"].as<int32_t>();
		
		BSP::LumpProviderPtr pprov = std::make_shared<BSP::BSPReaderLumpProvider>(bspr);
		BSP::Assembler bspa { pprov };
		auto shaders = std::make_shared<BSPI::ShaderArray>(bspr.shaders());
		auto surfaces = std::make_shared<BSPI::SurfaceArray>(bspr.surfaces());
		auto leafs = std::make_shared<BSPI::LeafArray>(bspr.leafs());
		auto leafsurfs = std::make_shared<BSPI::LeafSurfaceArray>(bspr.leafsurfaces());
		auto models = std::make_shared<BSPI::ModelArray>(bspr.models());
		auto brushsides = std::make_shared<BSPI::BrushSideArray>(bspr.brushsides());
		auto vertices = std::make_shared<BSPI::VertexArray>(bspr.drawverts());
		auto indices = std::make_shared<BSPI::IndexArray>(bspr.drawindices());
		auto lightmaps = std::make_shared<BSPI::LightmapArray>(bspr.lightmaps());
		bspa[BSP::LumpIndex::SHADERS] = std::make_shared<BSP::BSPIShaderArrayLumpProvider>(shaders);
		bspa[BSP::LumpIndex::SURFACES] = std::make_shared<BSP::BSPISurfaceArrayLumpProvider>(surfaces);
		bspa[BSP::LumpIndex::LEAFS] = std::make_shared<BSP::BSPILeafArrayLumpProvider>(leafs);
		bspa[BSP::LumpIndex::LEAFSURFACES] = std::make_shared<BSP::BSPILeafSurfacesArrayLumpProvider>(leafsurfs);
		bspa[BSP::LumpIndex::MODELS] = std::make_shared<BSP::BSPIModelArrayLumpProvider>(models);
		bspa[BSP::LumpIndex::BRUSHSIDES] = std::make_shared<BSP::BSPIBrushSidesArrayLumpProvider>(brushsides);
		bspa[BSP::LumpIndex::DRAWVERTS] = std::make_shared<BSP::BSPIVertexArrayLumpProvider>(vertices);
		bspa[BSP::LumpIndex::DRAWINDEXES] = std::make_shared<BSP::BSPIIndexArrayLumpProvider>(indices);
		bspa[BSP::LumpIndex::LIGHTMAPS] = std::make_shared<BSP::BSPILightmapArrayLumpProvider>(lightmaps);
		
		// SURFACE
		
		auto & world = models->at(midx);
		int32_t ipos = world.first_surface + world.num_surfaces;
		world.num_surfaces++;
		
		auto & surf = *surfaces->emplace( surfaces->begin() + ipos );
		surf = {};
		surf.type = BSP::SurfaceType::PLANAR;
		surf.shader = add_shader(*shaders, "textures/colors/white2");
		surf.fog = -1;
		
		surf.vert_count = 4;
		surf.vert_idx = vertices->size();
		
		surf.index_count = 12;
		surf.index_idx = indices->size();
		
		for (auto & m : *models)
			if (m.first_surface >= ipos) m.first_surface++;
			
		for (auto & bs : *brushsides)
			if (bs.surface >= ipos) bs.surface++;
		
		// VERTEX
		
		static constexpr std::array<std::array<float, 2>, 4> pos_base {{
			{0, 0},
			{0, 1},
			{1, 0},
			{1, 1}
		}};
		
		for (size_t i = 0; i < 4; i++) {
			auto & vert = vertices->emplace_back();
			vert = {};
			
			if (dir == "x") {
				vert.pos[0] = x;
				vert.pos[1] = y + pos_base[i][0] * size;
				vert.pos[2] = z + pos_base[i][1] * size;
			} else if (dir == "y") {
				vert.pos[0] = x + pos_base[i][0] * size;
				vert.pos[1] = y;
				vert.pos[2] = z + pos_base[i][1] * size;
			} else {
				vert.pos[0] = x + pos_base[i][0] * size;
				vert.pos[1] = y + pos_base[i][1] * size;
				vert.pos[2] = z;
			}
			
			printf("pos: %f, %f, %f\n", vert.pos[0], vert.pos[1], vert.pos[2]);
			
			vert.lightmap[0][0] = pos_base[i][0];
			vert.lightmap[0][1] = 1 - pos_base[i][1];
		}
		
		// INDEX
		
		indices->emplace_back(0);
		indices->emplace_back(1);
		indices->emplace_back(2);
		indices->emplace_back(2);
		indices->emplace_back(1);
		indices->emplace_back(3);
		indices->emplace_back(1);
		indices->emplace_back(0);
		indices->emplace_back(2);
		indices->emplace_back(1);
		indices->emplace_back(2);
		indices->emplace_back(3);
		
		// LEAF
		
		auto is_in_leaf = [&](int32_t const * mins, int32_t const * maxs){
			if (x < mins[0]) return false;
			if (y < mins[1]) return false;
			if (z < mins[2]) return false;
			if (x > maxs[0]) return false;
			if (y > maxs[1]) return false;
			if (z > maxs[2]) return false;
			return true;
		};
		
		if (!midx) {
			
			bool leaf_found = false;
			std::vector<int32_t> leaf_spos;
			for (size_t i = 0; i < leafs->size(); i++) {
				auto & leaf = leafs->at(i);
				if (!is_in_leaf(leaf.mins, leaf.maxs)) continue;
				leaf_found = true;
				leaf.num_surfaces++;
				leaf_spos.push_back(leaf.first_surface);
				leafsurfs->emplace(leafsurfs->begin() + leaf_spos.back(), ipos);
			}
			
			if (!leaf_found) {
				std::cerr << "failed to find a suitable leaf" << std::endl;
				return 1;
			}
		
			for (auto & leaf : *leafs) {
				for (int32_t spos : leaf_spos) if (leaf.first_surface > spos) leaf.first_surface++;
			}
		}
		
		// LIGHTMAP
		
		surf.lightmap[0] = lightmaps->size();
		surf.lightmap[1] = -1;
		surf.lightmap[2] = -1;
		surf.lightmap[3] = -1;
		surf.lightmap_styles[0] = 0;
		surf.lightmap_styles[1] = 255;
		surf.lightmap_styles[2] = 255;
		surf.lightmap_styles[3] = 255;
		auto & lm = lightmaps->emplace_back();
		
		int img_w, img_h, img_ch;
		auto * img_data = stbi_load(imgf.data(), &img_w, &img_h, &img_ch, 3);
		if (!img_data || img_w != 128 || img_h != 128 || img_ch != 3) {
			std::cerr << "src image MUST be 128x128 RGB" << std::endl;
			return 1;
		}
		
		auto * img_ptr = img_data;
		for (size_t y = 0; y < 128; y++) for (size_t x = 0; x < 128; x++) {
			lm.pixels[y][x].r = img_ptr[0];
			lm.pixels[y][x].g = img_ptr[1];
			lm.pixels[y][x].b = img_ptr[2];
			img_ptr += 3;
		}
		
		stbi_image_free(img_data);
		
		auto bytes = bspa.assemble();
		
		std::ofstream fout { output_path, std::ios_base::binary | std::ios_base::out };
		if (!fout.good()) {
			std::cerr << "failed to open output for writing" << std::endl;
			return 1;
		}
		fout.write( reinterpret_cast<char const *>(bytes.data()), bytes.size());
		fout.close();
	}
	
	// ================================
	// NSBRUSH
	// ================================
	
	if (args["nsbrush"]) {
		
		if (!args["idx"]) return -1;
		int32_t idx = args["idx"].as<int32_t>();
		
		BSP::LumpProviderPtr pprov = std::make_shared<BSP::BSPReaderLumpProvider>(bspr);
		BSP::Assembler bspa { pprov };
		
		std::shared_ptr<BSPI::ShaderArray> shaders = std::make_shared<BSPI::ShaderArray>(bspr.shaders());
		std::shared_ptr<BSPI::BrushArray> brushes = std::make_shared<BSPI::BrushArray>(bspr.brushes());
		bspa[BSP::LumpIndex::SHADERS] = std::make_shared<BSP::BSPIShaderArrayLumpProvider>(shaders);
		bspa[BSP::LumpIndex::BRUSHES] = std::make_shared<BSP::BSPIBrushArrayLumpProvider>(brushes);
		
		auto & brush = brushes->at(idx);
		auto & shin = shaders->at(brush.shader);
		if (!shin.content_flags) {
			std::cerr << "brush already non-solid" << std::endl;
			return 1;
		}
		
		int32_t shidx = -1;
		for (size_t i = 0; i < shaders->size(); i++) {
			auto const & sh = shaders->at(i);
			if (sh.path == shin.path && ! sh.content_flags) {
				shidx = i;
				break;
			}
		}
		
		if (shidx >= 0) {
			brush.shader = shidx;
		} else {
			brush.shader = shaders->size();
			auto & shnew = shaders->emplace_back(shin);
			shnew.content_flags = 0;
		}
		
		auto bytes = bspa.assemble();
		
		std::ofstream fout { output_path, std::ios_base::binary | std::ios_base::out };
		if (!fout.good()) {
			std::cerr << "failed to open output for writing" << std::endl;
			return 1;
		}
		fout.write( reinterpret_cast<char const *>(bytes.data()), bytes.size());
		fout.close();
	}
	
	return 0;
}
