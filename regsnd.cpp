/* ACM sound registration utility */
/* (c) ABel [TeamX], 2001-2004    */

// version 1.1, 24/12/2002: work with 8-bit names
//  some bug in GCC does not allow to work with names containing letter 'ï'

// version 1.2, 16/02/2003: ACM-files were opened in text-mode. Wrong number
//  of samples were written to list-file, and Fallout refused to play the SFXs.
//  Found by sghi

// version 1.3, 03 May 2003: RegSnd sorted names in uppercase, but Fallout
//  uses lowercase sorting.

// vesrion 1.4, 08 July 2004: The ACM names must be stored in ANSI (Windows) code page instead
//  of OEM (DOS). So the locale switch was removed from main().

// BIS Mapper2 can recreate deleted sndlist.lst file (Fallout itself tries to
//  do this too, but it inserts incorrect file tags).
// Looks like my utility is no longer needed.

#include <windows.h>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <clocale>
using namespace std;

const char list_file[] = "sndlist.lst";
const char list_back[] = "sndlist.bak";

struct SoundDesc {
	string name;
	int samples;
	int file_size;
};
typedef map<string, SoundDesc> Map;
typedef const Map::value_type Pair;
bool updated = false;
int added = 0;

ostream& operator<<(ostream &out, Pair &val) {
	return out << val. second. name << "\n" << val. second. samples << "\n" << val. second. file_size << "\n";
}

void read_list (Map &a) {
	ifstream in(list_file); in. setf(ios::skipws);
	int rec_cnt = 0;
	if (!in) {
		cout << "Cannot locate sound list file `" << list_file << "'.\n";
		throw -1;
	}
	cout << "Reading sound list...";
	in >> rec_cnt;
	while (!in. eof()) {
		string s; SoundDesc desc; int dummy;
		in >> s;
		desc. name = string(s.c_str());
// 03.05.2003: not strupr, but strlwr
//		strupr ((char*)s. c_str());
		_strlwr ((char*)s. c_str());
		if (s != "") {
			in >> desc. samples; in >> desc. file_size; in >> dummy;
			a[s] = desc;
		}
	}
	if (a. size() != rec_cnt) {
		updated = true;
		cout << " Actual count of sounds (" << a.size() << ") differs from header: " << rec_cnt << "\n";
	} else {
		cout << " Ok\n";
	}
}
void write_list (const Map &a) {
	CopyFile (list_file, list_back, FALSE);
	ofstream out(list_file);
	out << a.size() << endl;
	Map::const_iterator p = a.begin();
	int i = 2;
	while (p != a.end()) {
//		strupr ((char*)p->second.name.c_str());
		out << *p << i << "\n";
		p++; i += 2;
	}
}

int read_acm_samples (const char* file_name) {
	// 16.02.2003 File was opened in text-mode. Cool results ;)
	ifstream in(file_name, ios::in | ios::binary);
	int sig, res;
	in. read ((char*)&sig, 4). read ((char*)&res, 4);
	res <<= 1;
	if (sig != 0x01032897) {
		res = -1;
	}
	return res;
}

void add_new(Map &a) {
	WIN32_FIND_DATA fd;
	HANDLE hfnd = FindFirstFile ("*.acm", &fd);
	if (hfnd != INVALID_HANDLE_VALUE) {
		try {
			do {
				if (!(fd. dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					int samples = read_acm_samples (fd. cFileName);
					if (samples != -1) {
						SoundDesc desc;
						desc. name = string (fd. cFileName);
						desc. file_size = fd. nFileSizeLow;
						desc. samples = samples;
						string key = string (fd. cFileName);
// 03.05.2003: not strupr, but strlwr
//						strupr ((char*) key.c_str());
						_strlwr ((char*) key.c_str());
						a[key] = desc;
						added++;
					}
				}
			} while (FindNextFile (hfnd, &fd));
		} catch (...) {
			FindClose (hfnd);
			throw -1;
		}
		FindClose (hfnd);
	}
	updated = updated || (added > 0);
}

int main() {
// 08.07.2004: We don't need the locale. Names must be in Windows code page, not in DOS one.
//	SetFileApisToOEM();
//	setlocale (LC_ALL, "C");
	cout << "Sound registration utility v1.4  (c) 2001-2004 ABel [TeamX]\n";
	Map sounds;
	try {
		read_list (sounds);
		add_new (sounds);
		if (updated) {
			write_list (sounds);
			cout << added << " record(s) updated.\n";
		} else {
			cout << "No changes have been made.\n";
		}
	} catch(...) {
		cout << "Errors encountered. Program terminated.\n";
	}
	return 0;
}
