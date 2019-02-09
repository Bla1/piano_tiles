#include <MidiFile.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <sys/time.h>
#include <termios.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>


using namespace smf;
using namespace std;

struct note_data {
		note_data(int start, int duration, int note, bool on) : startTick(start), noteDuration(duration), noteNum(note), noteOn(on) {}
		int startTick;
		int noteDuration;
		int noteNum;
		bool noteOn;
};

int main(int argc, char** argv){
	if(argc < 2){
		std::cout<<"Please provide input midi file\n";
		return 1;
	}
	if(argc < 3){
		std::cout<<"Please provide serial port\n";
		return 1;
	}

	std::string filename (argv[1]);
	std::cout<<"file name is " << filename << "\n";
	smf::MidiFile midifile(filename);

	midifile.linkNotePairs();
	MidiEvent* mev;

	vector<note_data> notes;
	vector<note_data> notes_off;

	for (int i=0; i<midifile[0].size(); i++) {
		mev = &midifile[0][i];
		if (!mev->isNote()) {
			continue;
		}
		notes.push_back(note_data(mev->tick, mev->getTickDuration(), mev->getKeyNumber(), mev->isNoteOn()));
	}

	////////////////////opening the serial port///////////////////////////

	int fd = open (argv[2], O_RDWR | O_NOCTTY);
	if (fd < 0) { perror(argv[2]); exit(-1); }

	struct termios oldtio, tio;

	tcgetattr(fd, &oldtio);
	tio.c_cflag 	= B9600 | CS8 | CLOCAL | CREAD;
	tio.c_iflag 	= 0;
	tio.c_oflag 	= 0;
	tio.c_lflag 	= 0;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &tio);


	///////////////////note computation///////////////////////////


	struct timeval tv;

	gettimeofday(&tv, NULL);

	unsigned long long millisecondsSinceEpoch =
		(unsigned long long)(tv.tv_sec) * 1000 +
		(unsigned long long)(tv.tv_usec) / 1000;

	cout << millisecondsSinceEpoch << "\n";

	unsigned long long current_time = millisecondsSinceEpoch;

	note_data* curr_note;

	unsigned val = 0;
	string vector_string = to_string(val);

	while(notes.size()){

		gettimeofday(&tv, NULL);
		current_time = (unsigned long long)(tv.tv_sec) * 1000 +
					   (unsigned long long)(tv.tv_usec) / 1000;
		if (current_time - millisecondsSinceEpoch > (unsigned long long) notes.front().startTick) {
			curr_note = &notes.front();
			unsigned index = (unsigned)curr_note->noteNum - 60;
			if (curr_note->noteOn) {
				cout << curr_note->startTick << "\t" << curr_note->noteDuration << "\t" << curr_note->noteNum << "\n";


				if(index < 12){
					val |= 1 << index;
				}
			} else if(index < 12) {
				unsigned bitmask = -1 - (1<<index);
				val &= bitmask;
			}
			notes.erase(notes.begin());
			vector_string = to_string(val);
			write(fd, vector_string.c_str(), strlen(vector_string.c_str()));
			write(fd, "\r      \r", 8*sizeof(char));
		}
	}

	cout << endl;

	////////////////////closing the serial port///////////////////////////

	vector_string = "0";

	write(fd, vector_string.c_str(), strlen(vector_string.c_str()));
	write(fd, "\r            ", 13*sizeof(char));
	write(fd, "\r", sizeof(char));

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);

	return 0;
}
