#pragma once

#include <iostream>
#include <vector>
#include <variant>
#include <cstdint>

typedef std::vector<uint8_t> byte_arr;
typedef struct event {
	uint32_t length;
	uint32_t delta_time;
	uint8_t type;
	byte_arr event_data;
	event() : length(0), delta_time(0), type(0), event_data() {}
	event(uint32_t l, uint32_t dt, uint8_t t, byte_arr ed) : length(l), delta_time(dt), type(t), event_data(ed) {}
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

inline constexpr const char* NOTE_NAMES[12] = {
	"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

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
	event  read_event(FILE* file);
	chunk read_track(FILE* file); //large memory usage possible here
	int read_write_track(FILE* file, FILE* output_file); //use instead of storing all track data in memory
	uint32_t time_division;
	uint32_t tempo;
	uint32_t microseconds_per_tick;
public:
	void convert(const char* input_filename, const char* output_filename, const char* options);
};

void handle_error(int err);
bool compare_bytes(byte_arr buffer, byte_arr bytes, int length);
