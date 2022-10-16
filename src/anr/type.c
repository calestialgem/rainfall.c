// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "anr/mod.h"
#include "dbg/api.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

/* Create a built-in type. */
#define createBuiltinType(createdTag) \
  { .tag = createdTag }

Type const META_TYPE_INSTANCE   = createBuiltinType(TYPE_META);
Type const VOID_TYPE_INSTANCE   = createBuiltinType(TYPE_VOID);
Type const BOOL_TYPE_INSTANCE   = createBuiltinType(TYPE_BOOL);
Type const BYTE_TYPE_INSTANCE   = createBuiltinType(TYPE_BYTE);
Type const INT_TYPE_INSTANCE    = createBuiltinType(TYPE_INT);
Type const UXS_TYPE_INSTANCE    = createBuiltinType(TYPE_UXS);
Type const FLOAT_TYPE_INSTANCE  = createBuiltinType(TYPE_FLOAT);
Type const DOUBLE_TYPE_INSTANCE = createBuiltinType(TYPE_DOUBLE);

bool compareTypeEquality(Type left, Type right) {
  return left.tag == right.tag;
}

bool checkDefaultability(Type checked) {
  switch (checked.tag) {
  case TYPE_META:
  case TYPE_BOOL:
  case TYPE_BYTE:
  case TYPE_INT:
  case TYPE_UXS:
  case TYPE_FLOAT:
  case TYPE_DOUBLE: return true;
  case TYPE_VOID: return false;
  default: unexpected("Unknown type variant!");
  }
}

bool checkConvertability(Type source, Type destination) {
  // Types might be the same, or they might be arithmetic types that could be
  // converted to each other.
  return compareTypeEquality(source, destination) ||
         (checkArithmeticity(source) && checkArithmeticity(destination) &&
          checkArithmeticConvertability(source, destination));
}

bool checkArithmeticConvertability(Type source, Type destination) {
  return destination.tag == BOOL_TYPE_INSTANCE.tag ||
         source.tag <= destination.tag;
}

bool checkArithmeticity(Type checked) {
  return checked.tag >= BOOL_TYPE_INSTANCE.tag;
}

bool checkIntegerness(Type checked) {
  switch (checked.tag) {
  case TYPE_BOOL:
  case TYPE_BYTE:
  case TYPE_INT:
  case TYPE_UXS: return true;
  case TYPE_FLOAT:
  case TYPE_DOUBLE: return false;
  case TYPE_META:
  case TYPE_VOID: unexpected("Not an arithmetic type!");
  default: unexpected("Unknown type variant!");
  }
}

bool checkSignedness(Type checked) {
  switch (checked.tag) {
  case TYPE_BYTE:
  case TYPE_INT: return true;
  case TYPE_BOOL:
  case TYPE_UXS: return false;
  case TYPE_FLOAT:
  case TYPE_DOUBLE: unexpected("Not an integer type!");
  case TYPE_META:
  case TYPE_VOID: unexpected("Not an arithmetic type!");
  default: unexpected("Unknown type variant!");
  }
}

Type findCombination(Type left, Type right) {
  // The arithmetic type that could be the destination of conversion from the
  // other one should be the type of a binary expresison that has operands of
  // the given arithmetic types.
  return !checkArithmeticConvertability(left, right) ? left : right;
}

char const* nameType(Type named) {
  switch (named.tag) {
  case TYPE_META: return "type";
  case TYPE_VOID: return "void";
  case TYPE_BOOL: return "bool";
  case TYPE_BYTE: return "byte";
  case TYPE_INT: return "int";
  case TYPE_UXS: return "uxs";
  case TYPE_FLOAT: return "float";
  case TYPE_DOUBLE: return "double";
  default: unexpected("Unknown type variant!");
  }
}

Value defaultValue(Type defaulted) {
  switch (defaulted.tag) {
  case TYPE_BOOL: return (Value){.asBool = false};
  case TYPE_BYTE: return (Value){.asByte = 0};
  case TYPE_INT: return (Value){.asInt = 0};
  case TYPE_UXS: return (Value){.asUxs = 0};
  case TYPE_FLOAT: return (Value){.asFloat = 0};
  case TYPE_DOUBLE: return (Value){.asDouble = 0};
  case TYPE_META:
  case TYPE_VOID: unexpected("There is no default value for the given type!");
  default: unexpected("Unknown type variant!");
  }
}

uint64_t getMaximumValue(Type gotten) {
  switch (gotten.tag) {
  case TYPE_BOOL: return true;
  case TYPE_BYTE: return CHAR_MAX;
  case TYPE_INT: return INT_MAX;
  case TYPE_UXS: return SIZE_MAX;
  case TYPE_FLOAT:
  case TYPE_DOUBLE: unexpected("Not an integer type!");
  case TYPE_META:
  case TYPE_VOID: unexpected("Not an arithmetic type!");
  default: unexpected("Unknown type variant!");
  }
}

Value convertValue(Type source, Type destination, Value converted) {
  // If they are equal, return it.
  if (compareTypeEquality(source, destination)) return converted;

  // Otherwise they must be convertable arithmetic types.
  return convertArithmetic(source, destination, converted);
}

Value convertArithmetic(Type source, Type destination, Value converted) {
  switch (source.tag) {
  case TYPE_BOOL:
    switch (destination.tag) {
    case TYPE_BOOL: return converted;
    case TYPE_BYTE: return (Value){.asByte = converted.asBool};
    case TYPE_INT: return (Value){.asInt = converted.asBool};
    case TYPE_UXS: return (Value){.asUxs = converted.asBool};
    case TYPE_FLOAT: return (Value){.asFloat = converted.asBool};
    case TYPE_DOUBLE: return (Value){.asDouble = converted.asBool};
    case TYPE_META:
    case TYPE_VOID: unexpected("Not an arithmetic type!");
    default: unexpected("Unknown type variant!");
    };
  case TYPE_BYTE:
    switch (destination.tag) {
    case TYPE_BOOL: return (Value){.asBool = converted.asByte};
    case TYPE_BYTE: return converted;
    case TYPE_INT: return (Value){.asInt = converted.asByte};
    case TYPE_UXS: return (Value){.asUxs = converted.asByte};
    case TYPE_FLOAT: return (Value){.asFloat = converted.asByte};
    case TYPE_DOUBLE: return (Value){.asDouble = converted.asByte};
    case TYPE_META:
    case TYPE_VOID: unexpected("Not an arithmetic type!");
    default: unexpected("Unknown type variant!");
    };
  case TYPE_INT:
    switch (destination.tag) {
    case TYPE_BOOL: return (Value){.asBool = converted.asInt};
    case TYPE_INT: return converted;
    case TYPE_UXS: return (Value){.asUxs = converted.asInt};
    case TYPE_FLOAT: return (Value){.asFloat = converted.asInt};
    case TYPE_DOUBLE: return (Value){.asDouble = converted.asInt};
    case TYPE_BYTE: unexpected("Narrowing arithmetic conversion!");
    case TYPE_META:
    case TYPE_VOID: unexpected("Not an arithmetic type!");
    default: unexpected("Unknown type variant!");
    };
  case TYPE_UXS:
    switch (destination.tag) {
    case TYPE_BOOL: return (Value){.asBool = converted.asUxs};
    case TYPE_UXS: return converted;
    case TYPE_FLOAT: return (Value){.asFloat = converted.asUxs};
    case TYPE_DOUBLE: return (Value){.asDouble = converted.asUxs};
    case TYPE_BYTE:
    case TYPE_INT: unexpected("Narrowing arithmetic conversion!");
    case TYPE_META:
    case TYPE_VOID: unexpected("Not an arithmetic type!");
    default: unexpected("Unknown type variant!");
    };
  case TYPE_FLOAT:
    switch (destination.tag) {
    case TYPE_BOOL: return (Value){.asBool = converted.asFloat};
    case TYPE_FLOAT: return converted;
    case TYPE_DOUBLE: return (Value){.asDouble = converted.asFloat};
    case TYPE_BYTE:
    case TYPE_INT:
    case TYPE_UXS: unexpected("Narrowing arithmetic conversion!");
    case TYPE_META:
    case TYPE_VOID: unexpected("Not an arithmetic type!");
    default: unexpected("Unknown type variant!");
    };
  case TYPE_DOUBLE:
    switch (destination.tag) {
    case TYPE_BOOL: return (Value){.asBool = converted.asDouble};
    case TYPE_DOUBLE: return converted;
    case TYPE_BYTE:
    case TYPE_INT:
    case TYPE_UXS:
    case TYPE_FLOAT: unexpected("Narrowing arithmetic conversion!");
    case TYPE_META:
    case TYPE_VOID: unexpected("Not an arithmetic type!");
    default: unexpected("Unknown type variant!");
    };
  case TYPE_META:
  case TYPE_VOID: unexpected("Not an arithmetic type!");
  default: unexpected("Unknown type variant!");
  }
}

Value convertSignedArithmetic(Type destination, int64_t converted) {
  switch (destination.tag) {
  case TYPE_BYTE: return (Value){.asByte = converted};
  case TYPE_INT: return (Value){.asInt = converted};
  case TYPE_BOOL:
  case TYPE_UXS: unexpected("Not a signed integer type!");
  case TYPE_FLOAT:
  case TYPE_DOUBLE: unexpected("Not an integer type!");
  case TYPE_META:
  case TYPE_VOID: unexpected("Not an arithmetic type!");
  default: unexpected("Unknown type variant!");
  }
}

Value convertUnsignedArithmetic(Type destination, uint64_t converted) {
  switch (destination.tag) {
  case TYPE_BOOL: return (Value){.asBool = converted};
  case TYPE_UXS: return (Value){.asUxs = converted};
  case TYPE_BYTE:
  case TYPE_INT: unexpected("Not an unsigned integer type!");
  case TYPE_FLOAT:
  case TYPE_DOUBLE: unexpected("Not an integer type!");
  case TYPE_META:
  case TYPE_VOID: unexpected("Not an arithmetic type!");
  default: unexpected("Unknown type variant!");
  }
}
