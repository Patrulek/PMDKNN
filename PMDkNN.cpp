// PMDkNN.cpp : Defines the entry point for the console application.
//

// okolo 90s na 15% danych


#include "stdafx.h"
#include "MemoryMapped.h"

using namespace std;

#include <vector>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <ctime>
#include <string>
#include <fstream>
#include <list>
#include <iomanip>

#define BLOCK_SIZE 262144
#define USER_SIZE 1020000

typedef unsigned int uint;

inline int strtoi(const string & _str);
void sortUsers();
inline string truncDataToWholeLines(string & _data);
bool readAndProcessFile(string _filepath);
void processChunk(string & _chunk);

void find100_forallopt2();
void printOutput(vector<vector<pair<int, float>>> & _distances);

vector<unordered_set<int>> user_songs(USER_SIZE);
vector<vector<int>> user_songs2(USER_SIZE);
vector<int> sizes(USER_SIZE);

vector<unordered_set<int>> songs_to_users3(USER_SIZE);
vector<vector<int>> songs_to_users4(USER_SIZE);

unordered_set<int> unique_users100;

inline int binary_find(vector<int> const & _s1, int _val)
{
	auto it_b = _s1.begin();
	auto it_e = _s1.end();
	// Finds the lower bound in at most log(last - first) + 1 comparisons
	auto it = lower_bound(it_b, it_e, _val);

	if (it != it_e )//&& _val >= *it)
		return it - it_b; // found
	else
		return _s1.size(); // not found
}

inline int binary_find(vector<int>::iterator & _s1, vector<int>::iterator & _s2, int _val)
{
	// Finds the lower bound in at most log(last - first) + 1 comparisons
	auto it = lower_bound(_s1, _s2, _val);

	if (it != _s2)//&& _val >= *it)
		return it - _s1; // found
	else
		return _s2 - _s1; // not found
}

void printOutput(vector<vector<pair<int, float>>> & _distances) {
	fstream file("distances.txt", fstream::out | fstream::trunc);

	for (auto it = unique_users100.begin(); it != unique_users100.end(); it++) {
		file << endl << "User = " << *it << endl;

		for (int i = 99; i >= 0; i--) {
			if (_distances[*it][i].second <= 0)
				break;

			file << _distances[*it][i].first << " " << setprecision(3) << _distances[*it][i].second << endl;
		}
	}

	file.close();
}

// najmniejsze z przodu
inline bool sort_pairs2(pair<int, float> const & _s1, pair<int, float> const & _s2) {
	return _s1.second < _s2.second;
}

inline bool sort_pairs3(pair<int, float> const & _s1, float _s2) {
	return _s1.second < _s2;
}

inline int binary_find(vector<pair<int, float>> & _s1, float _val) {
	auto it_b = _s1.begin();
	auto it_e = _s1.end();
	// Finds the lower bound in at most log(last - first) + 1 comparisons
	auto it = lower_bound(it_b, it_e, _val, sort_pairs3);

	if (it != it_e)//&& _val >= *it)
		return it - it_b; // found
	else
		return _s1.size(); // not found
}

int main()
{
	double time = clock();

	if (!readAndProcessFile("F:/Downloads/facts.csv"))
		return -1;

	cout << "Reading/processing file: " << (clock() - time) / CLOCKS_PER_SEC * 1000 << " ms" << endl;

	time = clock();
	sortUsers();
	cout << "Sorting users: " << (clock() - time) / CLOCKS_PER_SEC * 1000 << " ms" << endl;

	time = clock();

	find100_forallopt2();
	cout << "Finding 100NN for all users: " << (clock() - time) / CLOCKS_PER_SEC * 1000 << " ms" << endl;

	return 0;
}

void find100_forallopt2() {
	vector<vector<pair<int, float>>> distances(USER_SIZE);
	vector<int> temp_dist(USER_SIZE);
	vector<int> offsets(USER_SIZE);
	vector<int> ints; ints.reserve(10000);

	for(int user_id = 0; user_id < USER_SIZE; user_id++) {
		if (user_id % 10000 == 0)
			cout << user_id / 10000 << "%" << endl;

		ints.clear();

		// dla wszystkich piosenek uzytkownika
		for(auto song_it = user_songs2[user_id].begin(), song_end = user_songs2[user_id].end(); song_it != song_end; song_it++) {
			for (auto user_it = songs_to_users4[*song_it].begin() + offsets[*song_it], user_end = songs_to_users4[*song_it].end(); user_it != user_end; user_it++) {
				if (user_id == *user_it)
					continue;

				ints.emplace_back(*user_it);
				temp_dist[*user_it]++;
			}

			offsets[*song_it]++;
		}

		if (distances[user_id].capacity() == 0)
			distances[user_id].reserve(100);

		for(int i = 0; i < ints.size(); i++) {
			int user_id2 = ints[i];
			float dist = (float)temp_dist[user_id2] / (sizes[user_id] + sizes[user_id2] - temp_dist[user_id2]);

			bool exists = false;
			int idx_1 = binary_find(distances[user_id], dist);

			while (idx_1 < distances[user_id].size() && distances[user_id][idx_1].second == dist) {
				if (distances[user_id][idx_1++].first == user_id2) {
					exists = true;
					break;
				}
			}

			// first user
			if ( !exists ) {
				if (distances[user_id].size() < 100) {
					distances[user_id].emplace_back(make_pair(user_id2, dist));

					if (distances[user_id].size() == 100)
						sort(distances[user_id].begin(), distances[user_id].end(), sort_pairs2);
				}
				else if (dist > distances[user_id][0].second) {
					int idx = binary_find(distances[user_id], dist) - 1;
					
					move(distances[user_id].begin() + 1, distances[user_id].begin() + idx + 1, distances[user_id].begin());

					distances[user_id][idx].first = user_id2;
					distances[user_id][idx].second = dist;
				}
			}

			// second user
			if (distances[user_id2].capacity() == 0)
				distances[user_id2].reserve(100);

			idx_1 = binary_find(distances[user_id2], dist);
			exists = false;

			while (idx_1 < distances[user_id2].size() && distances[user_id2][idx_1].second == dist) {
				if (distances[user_id2][idx_1++].first == user_id) {
					exists = true;
					break;
				}
			}

			if ( !exists ) {
				if (distances[user_id2].size() < 100) {
					distances[user_id2].emplace_back(make_pair(user_id, dist));

					if (distances[user_id2].size() == 100)
						sort(distances[user_id2].begin(), distances[user_id2].end(), sort_pairs2);
				}
				else if (dist > distances[user_id2][0].second) {
					int idx = binary_find(distances[user_id2], dist) - 1;

					move(distances[user_id2].begin() + 1, distances[user_id2].begin() + idx + 1, distances[user_id2].begin());

					distances[user_id2][idx].first = user_id;
					distances[user_id2][idx].second = dist;
				}
			}
			
			temp_dist[user_id2] = 0;
		}
	}

	printOutput(distances);
}

inline bool sort_songs(vector<int> const & _s1, vector<int> const & _s2) {
	return _s1.size() > _s2.size();
}

void sortUsers() {
	/*for (int i = 0; i < USER_SIZE; i++) {
		sizes[i] = user_songs[i].size();
		
		if (i == 6646 || i == 34396) {
			cout << "Size(" << i << ") = " << sizes[i] << endl;

			for (auto it = user_songs[i].begin(); it != user_songs[i].end(); it++)
				cout << *it << ", ";
			cout << endl << endl;
		}
	}*/
	for (int i = 0; i < USER_SIZE; i++) {
		user_songs2[i].assign(user_songs[i].begin(), user_songs[i].end());
		sort(user_songs2[i].begin(), user_songs2[i].end());
		sizes[i] = user_songs[i].size();
	}

	for (int i = 0; i < USER_SIZE; i++) {
		if (songs_to_users3[i].size() < 2)
			continue;

		songs_to_users4[i].assign(songs_to_users3[i].begin(), songs_to_users3[i].end());
		sort(songs_to_users4[i].begin(), songs_to_users4[i].end());
	}

	songs_to_users3.clear(); songs_to_users3.shrink_to_fit();
	user_songs.clear(); user_songs.shrink_to_fit();
}

void processChunk(string & _data) {
	int l = _data.length();

	if (l == 0)
		return;

	int start_pos = 0, end_pos = 0;
	const int SEP_GAP = 1;
	const int END_LINE_GAP = 2;
	int user_id_int = 0, song_id_int = 0, date_id_int = 0;
	int last_user = 0, last_song = 0;

	do {
		end_pos = _data.find(',', start_pos);
		string song_id = _data.substr(start_pos, end_pos - start_pos);
		start_pos = end_pos + SEP_GAP;
		song_id_int = strtoi(song_id);

		end_pos = _data.find(',', start_pos);
		string user_id = _data.substr(start_pos, end_pos - start_pos);
		start_pos = end_pos + SEP_GAP;
		user_id_int = strtoi(user_id);

		end_pos = _data.find('\r', start_pos) + END_LINE_GAP; start_pos = end_pos;

		if (song_id_int == last_song && user_id_int == last_user)
			continue;

		last_user = user_id_int;
		last_song = song_id_int;

		if (unique_users100.size() < 100)
			unique_users100.insert(user_id_int);

		songs_to_users3[song_id_int].insert(user_id_int);
		user_songs[user_id_int].insert(song_id_int);

	} while (end_pos < l - 2);
}

inline string truncDataToWholeLines(string & _data) {
	int cnt = 0, l = _data.length() - 1;

	while (_data[l--] != '\n')
		cnt++;

	if (cnt == 0)
		return "";

	int pos = l + 2;
	string rem = _data.substr(pos);
	_data = _data.substr(0, pos);

	return rem;
}

bool readAndProcessFile(string _filepath) {
	MemoryMapped file_mmf(_filepath, MemoryMapped::MapRange::WholeFile, MemoryMapped::CacheHint::SequentialScan);

	if (!file_mmf.isValid()) {
		cout << "Nie wczytano pliku: " << _filepath << endl;
		return false;
	}

	int offset = 25;	// usuniêcie pierwszej linii
	const unsigned char * raw_data = file_mmf.getData() + offset;

	unsigned int pos = 0, cnt = 0, x = 0;
	string remainder = "";
	int texts_number = floor((double)(file_mmf.size() - offset) / BLOCK_SIZE) + 1;

	do {
		char buffer[BLOCK_SIZE];
		unsigned int bytes = BLOCK_SIZE;

		if (pos == texts_number - 1) {
			unsigned int diff = (texts_number - 1) * BLOCK_SIZE;
			bytes = file_mmf.size() - diff + 1;
		}

		memcpy(buffer, raw_data + x, bytes);
		x += bytes;

		string file_data(buffer, bytes);
		file_data = remainder + file_data;

		remainder = truncDataToWholeLines(file_data);

		processChunk(file_data);
	} while (pos++ < texts_number - 1);

	return true;
}

inline int strtoi(const string & _str) {
	int sum = 0, init_max = _str.length() - 1, cnt = 0;

	if (init_max == 0)
		return (int)_str[0] - 48;

	for (int i = init_max; i > -1; i--) {
		int num = (int)_str[i] - 48;

		for (int j = 0; j < cnt; j++)
			num *= 10;

		sum += num;
		cnt++;
	}

	return sum;
}