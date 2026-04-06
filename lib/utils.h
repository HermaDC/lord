#include "types.h"

//generates the log mensages in LOG_PATH, the format is [LEVEL] message
void log_message(LogLevel level, const char *format, ...);

//Creates a track in system with the params, if id is 0 will generate one automatically
ErrorCode create_track(System *system, int id, Sensor *sensor, int next, int prev);

//Creates a switch in system with the params, if id is 0 will generate one automatically
ErrorCode create_switch(System *system, int id, Sensor *sensor, int next, int prev, int branch);

//Creates a switch and inserts it in system with the params, if id is 0 will generate one automatically
ErrorCode insert_switch(System *system, int id, Sensor *sensor, int track_next, int track_prev, int branch);

//Creates a num_tracks straight line of tracks in system, in index stores the index of the first track of the line
ErrorCode create_straight_line(System *system, int num_tracks, size_t *index);

//Prints the system tracks with the switches, starting from index
void print_tracks_with_switches(System *system, int index);

//Return the index of the last track connected counting from start_index
int get_last_track(System *system, int start_index);

//Returns the index of the next track from start_index
int get_next_track(System *system, int start_index);

//Counts recursively the tracks, including the branches, counting from branch_index
int count_branch_tracks(System *system, int branch_index);

//Forces to update the track_index track to new_status
ErrorCode force_update_track_status(System *system, int track_index, Status new_status);

//Updates all the system
void update_system_status(System *system, int index);

//Loads the system layout from a file, returns an array of systems, and stores in out_count the number of systems loaded
System *load_system_layout_from_file(const char *path, size_t *count);

//saves the actual layout described in system to path
ErrorCode save_system_to_file(System *system, const char* path);
