#pragma once

#include <iostream>
#include <vector>
#include <variant>

typedef std::vector<uint8_t> byte_arr;
typedef struct event {
	uint32_t delta_time;
	uint8_t type;
	byte_arr event_data;
	event() : delta_time(0), type(0), event_data() {}
	event(uint32_t dt, uint8_t t, byte_arr ed) : delta_time(dt), type(t), event_data(ed) {}
} event;
/*typedef struct meta_event {
	uint32_t delta_time;
	uint8_t type;
	uint8_t meta_type;
	uint32_t length;
	byte_arr meta_data;
	meta_event() : delta_time(0), type(0), meta_type(0), length(0), meta_data() {}
	meta_event(uint32_t dt, uint8_t t, uint8_t mt, uint32_t len, byte_arr md) : delta_time(dt), type(t), meta_type(mt), length(len), meta_data(md) {}
} meta_event;*/

typedef struct chunk {
	std::string type;
	uint32_t length;
	std::vector<event> data;
	chunk() : type(""), length(0), data() {}
} chunk;

class MidiConverter {
private:
	FILE* open_file(const char* filename, const char* mode);
	void close_file(FILE* file);
	byte_arr get_word(byte_arr buffer, FILE* file, int bytes);
	chunk read_header(FILE* file);
	chunk read_track(FILE* file);
public:
	void convert(const char* input_filename, const char* output_filename);
};

void handle_error(int err);
bool compare_bytes(byte_arr buffer, byte_arr bytes, int length);
