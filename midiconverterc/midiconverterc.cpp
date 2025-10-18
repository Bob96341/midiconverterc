#include <iostream>
#include <vector>
typedef std::vector<unsigned char> byte_arr;
typedef struct event {
	unsigned int delta_time;
	unsigned char type;
	byte_arr event_data;
} event;;
typedef struct chunk {
	std::string type;
	unsigned int length;
	std::vector<event> data;
} chunk;

void handle_error(errno_t err) {
	switch (err) {
	case ENOENT:
		std::cerr << "File not found" << std::endl;
		break;
	case EINVAL:
		std::cerr << "Invalid parameter" << std::endl;
		break;
	case 0:
		return;
	default:
		std::cerr << "Unknown error code: "  << strerror(err) << std::endl;
		break;
	}
	exit(1);
}

FILE* open_file(const char* filename, const char* mode) {
	FILE *file = fopen(filename,mode);
	if(!file) 
		handle_error(errno);
	return file;
}

void close_file(FILE* file) {
	if (fclose(file) != 0) {
		std::cerr << "Error closing file: " << strerror(errno) << std::endl;
		//handle_error(errno);
		exit(1);
	}
}

bool compare_bytes(byte_arr buffer, byte_arr bytes, int length) {
	for (int i = 0; i < length; i++) 
		if (buffer[i] != bytes[i]) return false;
	return true;
}

/*unsigned int get_byte(FILE* file) {
	unsigned int byte = getc(file);
	if (byte == EOF) {
		std::cerr << "End of file reached" << std::endl;
		return EOF;
	}
	return byte;
}*/

byte_arr get_word(byte_arr buffer, FILE* file, int bytes) {
	buffer.clear();
	for (int i = 0; i < bytes; i++) {
		int c = getc(file);
		if (c == EOF) {
			std::cerr << "Unexpected end of file\n";
			break; 
		}
		buffer.push_back(static_cast<unsigned char>(c));
	}
	return buffer;
}

chunk read_header(FILE* file) {
	chunk header;
	byte_arr buffer = get_word(buffer, file, 4);//MThd
	header.type = std::string(buffer.begin(), buffer.end());
	buffer = get_word(buffer, file, 4);//length
	header.length = (uint32_t)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
	uint32_t len = header.length;
	while (len > 0) {
		buffer = get_word(buffer, file, 2);//format, ntrks, division
		header.data.push_back({ 0, 0, buffer });
		len -= 2;
	}
	if (header.type != "MThd") {
		std::cerr << "Invalid MIDI file" << std::endl;
		close_file(file);
		exit(1);
	}
	if ((header.data[0].event_data[0] << 8 | header.data[0].event_data[1]) != 0 || (header.data[1].event_data[0] << 8 | header.data[1].event_data[1]) != 1) {
		std::cerr << "Unsupported MIDI format or number of tracks" << std::endl;
		//add format 1 later
		close_file(file);
		exit(1);
	}
	return header;
}

int main(int argc, char* argv[]){
    std::cout << "Hello World!\n";
	std::string filename = argv[1];
	FILE *f = open_file(filename.c_str(), "rb");
	std::cout << "Opened file: " << filename << std::endl;
	chunk header = read_header(f);

	std::cout << header.type << std::endl;
	for(unsigned int i = 0; i < header.data.size(); i++)
		std::cout << std::hex << (int)(header.data[i].event_data[0] << 8 | header.data[i].event_data[1]) << " ";

	close_file(f);
	return 0;
}

/*
values needed to find in file:
0x4D 0x54 0x68 0x64 - header chunk
0x00 0x00 0x00 0x06 - header length
0x00 0x00 - format type (just check if it's 0)
0x00 0x01 - number of tracks (just check if it's 1)
0x00 0xXX - time division
0x4D 0x54 0x72 0x6B - track chunk
0xXX 0xXX 0xXX 0xXX - track length
0x00 0xFF 0x51 0x03 0xXX 0xXX 0xXX - set tempo (assume 120 bpm if not found)
0x00 0xFF 0x58 0x04 0xXX 0xXX 0xXX 0xXX - time signature (assume 4/4 with defaults if not found)
0x00 0xFF 0x59 0x02 0xXX 0xXX - key signature (assume c major if not found)

figure out how to find notes

when reach delta time
	if msb is 1
		variable length value - read next byte and add to value
	if msb is 0
		add to value and stop

read event code
if event code 0xC0
	program change - ignore
if event code 0xB0
	control change - ignore
if event code 0xE0
	pitch bend - ignore
if event code 0xA0
	aftertouch - ignore
if event code 0xD0
	channel aftertouch - ignore
if event code 0x90 or 0x80
	read note and velocity
	if velocity is 0
		note off
	else
		note on

stack for notes
if event code 0x90
	push note on stack
if event code 0x80
	pop note from stack

instead of cycles use delta time from next note in file
when reach end of cycle
	play first note in stack
	set new cycle to delta time of next note in file

if event code 0xFF 0x2F 0x00
	end of track
*/