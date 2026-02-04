

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