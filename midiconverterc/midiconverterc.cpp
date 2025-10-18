#include <iostream>
//#include <errno.h>

typedef struct chunk {
	char* type;
	unsigned int length;
	unsigned int** meta;
	unsigned int** data;
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
		std::cerr << "Unknown error code: "  << err << std::endl;
		break;
	}
	exit(1);
}

FILE* open_file(const char* filename, const char* mode) {
	FILE* file = nullptr;
	errno_t err = fopen_s(&file,filename,mode);
	if(err != 0) 
		handle_error(err);
	return file;
}

void close_file(FILE* file) {
	if (fclose(file) != 0) {
		std::cerr << "Error closing file" << std::endl;
		exit(1);
	}
}

bool compare_bytes(int* buffer, int* bytes, int length) {
	for (int i = 0; i < length; i++) 
		if (buffer[i] != bytes[i]) return false;
	return true;
}

unsigned int get_byte(FILE* file) {
	unsigned int byte = getc(file);
	/*if (byte == EOF) {
		std::cerr << "End of file reached" << std::endl;
		return EOF;
	}*/
	return byte;
}

unsigned int *get_word(unsigned int *buffer, FILE* file, int bytes) {
	if (buffer) free(buffer);
	buffer = nullptr;
	unsigned int c = get_byte(file);
	/*for (int i = 0; c != EOF; c = get_byte(file), i++) {
		buffer = buffer ? (int*)realloc(buffer, sizeof(int) * (i+1)) : (int*)malloc(sizeof(int));
		buffer[i] = c;
		//figure out how to do this dumbassssssss
		if (compare_bytes(buffer, std::begin({ 0x4D, 0x54, 0x68, 0x64 }), sizeof(buffer) / sizeof(buffer[0]))) {
			return;
		}
	}*/
	for (int i = 0; i < bytes; i++) {
		buffer = buffer ? (unsigned int*)realloc(buffer, sizeof(unsigned int) * (i+1)) : (unsigned int*)malloc(sizeof(unsigned int));
		buffer[i] = c;
		c = get_byte(file);
	}
	return buffer;
}

chunk read_header(FILE* file) {
	chunk header;
	unsigned int* buffer = nullptr; get_word(buffer, file, 4);
	unsigned int** words = (unsigned int**)malloc(sizeof(unsigned int*));
	words[0] = get_word(words[0], file, 4);
	//figure out how to parse the header
	return header;
}

int main(int argc, char* argv[]){
    std::cout << "Hello World!\n";
	std::string filename = argv[1];
	FILE *f = open_file(filename.c_str(), "rb");
	unsigned int* word = nullptr; get_word(word, f, 4);
	std::cout << std::hex <<  << std::endl;
	return 0;
}

//just make a fucking word struct/class




/*
values needed to find in file:
0x4D 0x54 0x68 0x64 - header chunk
0x00 0x00 0x00 0x06 - header length
0x00 0x00 - format type (just check if it's 0)
0x00 0x01 - number of tracks (just check if it's 1)
0x00 0xXX - time division (96 ticks per quarter note)
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