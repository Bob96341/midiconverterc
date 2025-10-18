#include "midiconverterc.h"
int main(int argc, char* argv[]) {
	std::cout << "Hello World!\n";
	std::string filename = argv[1];
	MidiConverter mc;
	mc.convert(filename.c_str(), "output.txt");
	return 0;
}