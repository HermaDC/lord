#include <stdio.h>

typedef enum Status {
    CLEAR,
    OCCUPIED,
    WARNING
} Status;

typedef enum direction{
    NEXT = 1,
    PREV = -1
} Direction;

typedef struct Sensor {
    int hex_direction;
    int actual_state;
    //TODO think of adding a recent train or similar to check those first 
    //                      To be quicker that searching in all of the tracks 
    //                      Then add the actual with train and erasing the previous one
} Sensor;

typedef enum TrackType {
    STRAIGHT,
    SWITCH_TRACK
} TrackType;

typedef enum SwitchPosition {
    STRAIGHT_POS,
    DIVERGING_POS
} SwitchPosition;

typedef struct Track {
    TrackType type;
    int id;

    Status status;

    struct Track* next;
    struct Track* prev;

    Direction dir;

    Sensor* sensors;

} Track;

typedef struct {
    Track base;

    Track* branch;
    SwitchPosition pos;

} Switch;


int count_branch_tracks(Track* branch);

int update_track_status(Track* track);

void update_system_status(Track* head);

Track **load_system_layout_from_file(const char *path);

void free_tracks(Track* head, Track* original);