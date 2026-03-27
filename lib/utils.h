#include "types.h"


Track *create_straight_line(int num_tracks);

Switch *create_switch(int id, Sensor *sensor, Track *next, Track *prev, Track *branch);

Switch *insert_switch(Track *track_prev, Track *track_next, Sensor *sensor, Track *branch);

int count_branch_tracks(Track *branch);

Track *get_next_track(Track *track);

int update_track_status(Track *track);

ErrorCode force_update_track_status(Track *track, Status new_status);

void update_system_status(Track *head);

#ifdef DEBUG
Track **load_system_layout_from_file(const char *path, size_t *count);
#endif

Track *get_last_track(Track *head);

void free_tracks(Track *head, Track *original);

// Parse a single layout line (e.g. "10 SW(5) 10") into a Track* chain
// Returns head of the created track chain, or NULL on error.
//Track *parse_layout_line(const char *input);

//saves the layout to path
ErrorCode save_layout_to_file(const char *path, Track *head);
