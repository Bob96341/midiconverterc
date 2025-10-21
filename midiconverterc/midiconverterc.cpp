#include "midiconverterc.h"
#include <cerrno>
#include <cstring>
void handle_error(int err) {
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

bool compare_bytes(byte_arr buffer, byte_arr bytes, int length) {
	for (int i = 0; i < length; i++)
		if (buffer[i] != bytes[i]) return false;
	return true;
}

FILE* MidiConverter::open_file(const char* filename, const char* mode) {
	FILE *file = fopen(filename,mode);
	if(!file) 
		handle_error(errno);
	return file;
}

void MidiConverter::close_file(FILE* file) {
	if (fclose(file) != 0) {
		std::cerr << "Error closing file: " << strerror(errno) << std::endl;
		//handle_error(errno);
		exit(1);
	}
}

byte_arr MidiConverter::get_word(byte_arr buffer, FILE* file, int bytes) {
	buffer.clear();
	for (int i = 0; i < bytes; i++) {
		int c = getc(file);
		if (c == EOF) {
			//std::cout << "End of file\n";
			buffer.clear();
			return buffer;
		}
		buffer.push_back(static_cast<uint8_t>(c));
	}
	return buffer;
}

chunk MidiConverter::read_header(FILE* file) {
	chunk header;
	byte_arr buffer = get_word(buffer, file, 4);//MThd
	header.type = std::string(buffer.begin(), buffer.end());
	if (header.type != "MThd") {
		std::cerr << "Invalid MIDI file" << std::endl;
		close_file(file);
		exit(1);
	}
	buffer = get_word(buffer, file, 4);//length
	header.length = (uint32_t)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
	uint32_t len = header.length;
	while (len > 0) {
		buffer = get_word(buffer, file, 2);//format, ntrks, division
		header.data.push_back({2, 0, 0, buffer });
		len -= 2;
	}
	if ((header.data[0].event_data[0] << 8 | header.data[0].event_data[1]) == 2) {
		std::cerr << "Unsupported MIDI format" << std::endl;
		close_file(file);
		exit(1);
	}
	return header;
}

event MidiConverter::read_event(FILE* file) {
	event ev;
	//read delta time
	byte_arr buffer = get_word(buffer, file, 1);
	uint32_t delta_time = (uint32_t)buffer[0];
	ev.length = 1;
	while (buffer[0] & 0x80) {
		buffer = get_word(buffer, file, 1);
		delta_time = (delta_time << 7) | (buffer[0] & 0x7F);
		ev.length++;
	}
	//read event type
	buffer = get_word(buffer, file, 1);
	ev.length++;
	ev.type = buffer[0];
	ev.delta_time = delta_time;
	if (buffer[0] != 0xFF) {
		uint8_t first_half = ev.type & 0xF0;
		switch (first_half) {
		case 0xC0:
		case 0xD0:
			buffer = get_word(buffer, file, 1);
			ev.length++;
			ev.event_data = buffer;
			break;
		case 0xA0:
		case 0xB0:
		case 0xE0:
		case 0x90:
		case 0x80:
			buffer = get_word(buffer, file, 2);
			ev.length += 2;
			ev.event_data = buffer;
			break;
		default:
			break;
		}
	}
	else {
		//meta event
		buffer = get_word(buffer, file, 1); //meta type
		ev.length++;
		ev.event_data.push_back(buffer[0]);
		buffer = get_word(buffer, file, 1); //length
		ev.length++;
		uint32_t meta_length = buffer[0];
		ev.event_data.push_back(buffer[0]);
		while (buffer[0] & 0x80) {
			buffer = get_word(buffer, file, 1);
			ev.length++;
			meta_length = (meta_length << 7) | (buffer[0] & 0x7F);
			ev.event_data.push_back(buffer[0]);
		}
		byte_arr meta_data = get_word(buffer, file, meta_length);
		ev.length += meta_length;
		ev.event_data.insert(ev.event_data.end(), meta_data.begin(), meta_data.end());
	}
	return ev;
}

chunk MidiConverter::read_track(FILE* file) {
	chunk track;
	byte_arr buffer = get_word(buffer, file, 4);//MTrk
	track.type = std::string(buffer.begin(), buffer.end());
	if (track.type != "MTrk") {
		std::cerr << "Invalid MIDI file" << std::endl;
		close_file(file);
		exit(1);
	}
	buffer = get_word(buffer, file, 4);//length
	track.length = (uint32_t)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
	uint32_t len = track.length;
	while (len > 0) {
		event ev = read_event(file);
		len -= ev.length;
		track.data.push_back(ev);
	}
	return track;
}

int MidiConverter::read_write_track(FILE* file, FILE* output_file) {
	byte_arr buffer = get_word(buffer, file, 4);//length
	uint32_t len = (uint32_t)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
	while (len > 0) {
		std::vector<event> ev;
		uint32_t delta_time = 0, next_delta_time = 0;
		for (int i = 0; (ev[i] = read_event(file)).delta_time != 0; i++) {
			if (ev[i].delta_time != 0) {
				next_delta_time = delta_time + ev[i].delta_time;
				/*fseek(file, -ev[i].length, SEEK_CUR);
				ev.pop_back();*/
				break; //this part will fail if the next time step doesn't have a note event
			}
			
		}
		//process concurrent events here then set delta_time to next_delta_time
	}
	return 0;
}

void MidiConverter::convert(const char* input_filename, const char* output_filename, const char* options) {
	FILE* input_file = open_file(input_filename, "rb");
	std::cout << "Opened file: " << input_filename << std::endl;
	FILE* output_file = open_file(output_filename, "w");
	std::cout << "Opened file: " << output_filename << std::endl;

	byte_arr buffer;
	while (!(buffer = get_word(buffer, input_file, 4)).empty()) {
		if (buffer.size() < 4) break;
		std::string chunk_type = std::string(buffer.begin(), buffer.end());
		if (chunk_type == "MThd") {
			fseek(input_file, -4, SEEK_CUR);
			chunk header = read_header(input_file);
			time_division = (uint32_t)((header.data[2].event_data[0] << 8) | header.data[2].event_data[1]);
			std::cout << header.type << " chunk read, length: " << header.length << std::endl;
			std::cout << "Format type: " << ((header.data[0].event_data[0] << 8) | header.data[0].event_data[1]) << std::endl;
			std::cout << "Number of tracks: " << ((header.data[1].event_data[0] << 8) | header.data[1].event_data[1]) << std::endl;
			std::cout << "Time division: " << ((header.data[2].event_data[0] << 8) | header.data[2].event_data[1]) << std::endl;
		}
		else if(chunk_type == "MTrk"){
			char c;
			for (c = options[0]; c != '\0' && c != 'R' && c != 'W'; c = *(++options));
			switch (c) {
			case 'R': {
				fseek(input_file, -4, SEEK_CUR);
				std::cout << "Only reading\n";
				chunk track = read_track(input_file);
				std::cout << track.type << " chunk read, length: " << track.length << std::endl;
				for (int i = 0; i < track.data.size(); i++) {
					event ev = track.data[i];
					std::cout << "Event " << i << ": delta time: " << ev.delta_time << ", type: " << std::hex << (int)ev.type << ", data: ";
					for (int j = 0; j < ev.event_data.size(); j++) {
						std::cout << std::hex << (int)ev.event_data[j] << " ";
					}
					std::cout << std::dec << std::endl;
				}
				break;
			}
			case 'W':
			default:
				std::cout << "Reading and writing\n";
				if (read_write_track(input_file, output_file)) {
					//failure
					std::cerr << "Error reading/writing track\n";
					close_file(input_file);
					close_file(output_file);
					exit(1);
				}
				std::cout << "Track processed successfully\n";
				break;
			}
		}
		else {
			std::cerr << "Unknown chunk type: " << chunk_type << std::endl;
			break;
		}
	}

	std::cout << "Finished processing MIDI file\n";

	close_file(input_file);
	close_file(output_file);
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
if event code 0xFF
	meta event
	if meta type 0x2F
		end of track
		break
	else
		use or ignore meta event

vector for notes
when note on
	push note to stack
when note off
	remove note from stack

reading notes:
read_note:
check delta_time_of_next_event
if delta_time_of_next_event == 0:
	repeat read_note for next event
else:
	length of cycle = delta_time_of_next_event (with the formula to convert to us)
(figure out a way to make sure the previous noet is played if a note finished before the next note starts)

instead of cycles use delta time from next note in file
when reach end of cycle
	play first note in stack
	set new cycle to delta time of next note in file

if event code 0xFF 0x2F 0x00
	end of track
*/
