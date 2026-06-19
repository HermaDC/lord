#include "eval.h"

static inline Value make_none(void) {
	Value v;
	v.type = VALUE_NONE;
	v.data.number_value = 0;
	return v;
}

static inline Value make_number(double number) {
	Value v;
	v.type = VALUE_NUMBER;
	v.data.number_value = number;
	return v;
}

static inline Value make_bool(int truthy) {
	Value v;
	v.type = VALUE_BOOL;
	v.data.bool_value = truthy ? 1 : 0;
	return v;
}

static inline double value_to_number(Value value) {
	switch(value.type) {
	case VALUE_NUMBER:
		return value.data.number_value;
	case VALUE_BOOL:
		return value.data.bool_value ? 1.0 : 0.0;
	default:
		return 0.0;
	}
}

static inline int value_is_truthy(Value value) {
	switch(value.type) {
	case VALUE_BOOL:
		return value.data.bool_value != 0;
	case VALUE_NUMBER:
		return value.data.number_value != 0.0;
	default:
		return 0;
	}
}
