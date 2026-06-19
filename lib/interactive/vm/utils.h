#include "eval.h"

static inline Value make_none(void);
static inline Value make_number(double number);
static inline Value make_bool(int truthy);
static inline double value_to_number(Value value);
static inline int value_is_truthy(Value value);
