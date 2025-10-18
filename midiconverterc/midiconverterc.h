#pragma once
#ifndef MIDICONVERTERC_H
#define MIDICONVERTERC_H

#include <iostream>
#include <vector>

typedef std::vector<unsigned char> byte_arr;
typedef struct event {
	unsigned int delta_time;
	unsigned char type;
	byte_arr event_data;
} event;
typedef struct chunk {
	std::string type;
	unsigned int length;
	std::vector<event> data;
} chunk;

class MidiConverter {
private:
	FILE* open_file(const char* filename, const char* mode);
	void close_file(FILE* file);
	byte_arr get_word(byte_arr buffer, FILE* file, int bytes);
	chunk read_header(FILE* file);
public:
	void convert(const char* input_filename, const char* output_filename);
};

void handle_error(errno_t err);
bool compare_bytes(byte_arr buffer, byte_arr bytes, int length);
#endif