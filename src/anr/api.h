// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#pragma once

/* Type of a type symbol. */
typedef struct {
} Meta;

/* Signed, 1-byte integer. */
typedef struct {
} I1;

/* Signed, 2-byte integer. */
typedef struct {
} I2;

/* Signed, 4-byte integer. */
typedef struct {
} I4;

/* Signed, 8-byte integer. */
typedef struct {
} I8;

/* Signed, pointer-size integer. */
typedef struct {
} Ix;

/* Unsigned, 1-byte integer. */
typedef struct {
} U1;

/* Unsigned, 2-byte integer. */
typedef struct {
} U2;

/* Unsigned, 4-byte integer. */
typedef struct {
} U4;

/* Unsigned, 8-byte integer. */
typedef struct {
} U8;

/* Unsigned, pointer-size integer. */
typedef struct {
} Ux;

/* Floating-point, 4-byte real. */
typedef struct {
} F4;

/* Floating-point, 8-byte real. */
typedef struct {
} F8;

/* Variant of type. */
typedef enum {
  /* Meta type. */
  TYP_META,
  /* I1 type. */
  TYP_I1,
  /* I2 type. */
  TYP_I2,
  /* I4 type. */
  TYP_I4,
  /* I8 type. */
  TYP_I8,
  /* IX type. */
  TYP_IX,
  /* U1 type. */
  TYP_U1,
  /* U2 type. */
  TYP_U2,
  /* U4 type. */
  TYP_U4,
  /* U8 type. */
  TYP_U8,
  /* UX type. */
  TYP_UX,
  /* F4 type. */
  TYP_F4,
  /* F8 type. */
  TYP_F8
} TypeTag;

/* Meaning of a symbol. */
typedef struct {
  union {
    /* Type as a meta. */
    Meta meta;
    /* Type as a i1. */
    I1   i1;
    /* Type as a i2. */
    I2   i2;
    /* Type as a i4. */
    I4   i4;
    /* Type as a i8. */
    I8   i8;
    /* Type as a ix. */
    Ix   ix;
    /* Type as a u1. */
    U1   u1;
    /* Type as a u2. */
    U2   u2;
    /* Type as a u4. */
    U4   u4;
    /* Type as a u8. */
    U8   u8;
    /* Type as a ux. */
    Ux   ux;
    /* Type as a f4. */
    F4   f4;
    /* Type as a f8. */
    F8   f8;
  };

  /* Variant of the type. */
  TypeTag tag;
} Type;

/* Instance of meta type. */
extern Type const TYP_INS_META;
/* Instance of i1 type. */
extern Type const TYP_INS_I1;
/* Instance of i2 type. */
extern Type const TYP_INS_I2;
/* Instance of i4 type. */
extern Type const TYP_INS_I4;
/* Instance of i8 type. */
extern Type const TYP_INS_I8;
/* Instance of ix type. */
extern Type const TYP_INS_IX;
/* Instance of u1 type. */
extern Type const TYP_UNS_U1;
/* Instance of u2 type. */
extern Type const TYP_UNS_U2;
/* Instance of u4 type. */
extern Type const TYP_UNS_U4;
/* Instance of u8 type. */
extern Type const TYP_UNS_U8;
/* Instance of ux type. */
extern Type const TYP_UNS_UX;
/* Instance of f4 type. */
extern Type const TYP_FNS_F4;
/* Instance of f8 type. */
extern Type const TYP_FNS_F8;
