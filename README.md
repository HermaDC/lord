

# Docs

typedef struct Track {
    TrackType type;       // para saber qué es
    int id;

    Status status;

    struct Track* next;
    struct Track* prev;

    Direction dir;

    Sensor* sensors;

} Track;

typedef struct {
    Track base;

    Track* branch;        // bifurcación
    SwitchPosition pos;   // posición actual

} Switch;
