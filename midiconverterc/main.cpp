#include "midiconverterc.h"
int main(int argc, char* argv[]) {
	std::string options = "", input_filename, output_filename = "output.txt";
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <input_midi_file>" << std::endl;
		return 1;
	}
	for(int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "--help" || std::string(argv[i]) == "-h") {
			std::cout << "Usage: " << argv[0] << " <input_midi_file> <options>\n";
			std::cout << "Converts a MIDI file to a text representation.\n";
			std::cout << "Options:\n";
			std::cout << "  -h, --help        Show this help message and exit\n";
			std::cout << "  -R                Read only mode\n";
			std::cout << "  -W                Read and write mode (default)\n";
			std::cout << "  -o <output_file>  Specify output file name (default: output.txt)\n";
			return 0;
		}
		if (argv[i][0] == '-') {
			if(std::string(argv[i])== "-o" && i + 1 < argc) {
				output_filename = argv[++i];
				continue;
			}
			options += std::string(argv[i]);
		}
		else 
			input_filename = argv[i];
	}
	MidiConverter mc;
	mc.convert(input_filename.c_str(), output_filename.c_str(), options.c_str());
	return 0;
}