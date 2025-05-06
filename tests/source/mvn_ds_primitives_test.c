/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds_primitives_test.h"

#include "mvn_ds/mvn_ds.h"
#include "mvn_ds/mvn_ds_arr.h"  // For mvn_val_arr
#include "mvn_ds/mvn_ds_hmap.h" // For mvn_val_hmap
#include "mvn_ds/mvn_ds_str.h"  // For mvn_val_str_borrow
#include "mvn_ds_test_utils.h"

#include <math.h>    // For fabsf, fabs
#include <stdbool.h> // For bool
#include <stdint.h>  // For int types
#include <stdio.h>
#include <string.h>

// --- Test Functions ---

static bool test_primitives(void)
{
    // --- Test MVN_VAL_NULL ---
    mvn_val_t val_null = mvn_val_null();
    TEST_ASSERT(val_null.type == MVN_VAL_NULL, "Null type mismatch");
    mvn_val_free(&val_null);
    TEST_ASSERT(val_null.type == MVN_VAL_NULL, "Null type mismatch after free");

    // --- Test MVN_VAL_BOOL ---
    mvn_val_t val_bool_true = mvn_val_bool(true);
    TEST_ASSERT(val_bool_true.type == MVN_VAL_BOOL, "Bool(true) type mismatch");
    TEST_ASSERT(val_bool_true.b == true, "Bool(true) value mismatch");
    mvn_val_free(&val_bool_true);
    TEST_ASSERT(val_bool_true.type == MVN_VAL_NULL, "Bool(true) type mismatch after free");

    mvn_val_t val_bool_false = mvn_val_bool(false);
    TEST_ASSERT(val_bool_false.type == MVN_VAL_BOOL, "Bool(false) type mismatch");
    TEST_ASSERT(val_bool_false.b == false, "Bool(false) value mismatch");
    mvn_val_free(&val_bool_false);
    TEST_ASSERT(val_bool_false.type == MVN_VAL_NULL, "Bool(false) type mismatch after free");

    // --- Test MVN_VAL_I8 ---
    mvn_val_t val_i8_val = mvn_val_i8(INT8_C(-120));
    TEST_ASSERT(val_i8_val.type == MVN_VAL_I8, "I8 type mismatch");
    TEST_ASSERT(val_i8_val.i8 == INT8_C(-120), "I8 value mismatch");
    mvn_val_free(&val_i8_val);
    TEST_ASSERT(val_i8_val.type == MVN_VAL_NULL, "I8 type mismatch after free");

    // --- Test MVN_VAL_I16 ---
    mvn_val_t val_i16_val = mvn_val_i16(INT16_C(30000));
    TEST_ASSERT(val_i16_val.type == MVN_VAL_I16, "I16 type mismatch");
    TEST_ASSERT(val_i16_val.i16 == INT16_C(30000), "I16 value mismatch");
    mvn_val_free(&val_i16_val);
    TEST_ASSERT(val_i16_val.type == MVN_VAL_NULL, "I16 type mismatch after free");

    // --- Test MVN_VAL_I32 ---
    mvn_val_t val_i32_pos = mvn_val_i32(12345);
    TEST_ASSERT(val_i32_pos.type == MVN_VAL_I32, "I32(pos) type mismatch");
    TEST_ASSERT(val_i32_pos.i32 == 12345, "I32(pos) value mismatch");
    mvn_val_free(&val_i32_pos);
    TEST_ASSERT(val_i32_pos.type == MVN_VAL_NULL, "I32(pos) type mismatch after free");

    mvn_val_t val_i32_neg = mvn_val_i32(-54321);
    TEST_ASSERT(val_i32_neg.type == MVN_VAL_I32, "I32(neg) type mismatch");
    TEST_ASSERT(val_i32_neg.i32 == -54321, "I32(neg) value mismatch");
    mvn_val_free(&val_i32_neg);
    TEST_ASSERT(val_i32_neg.type == MVN_VAL_NULL, "I32(neg) type mismatch after free");

    mvn_val_t val_i32_zero = mvn_val_i32(0);
    TEST_ASSERT(val_i32_zero.type == MVN_VAL_I32, "I32(zero) type mismatch");
    TEST_ASSERT(val_i32_zero.i32 == 0, "I32(zero) value mismatch");
    mvn_val_free(&val_i32_zero);
    TEST_ASSERT(val_i32_zero.type == MVN_VAL_NULL, "I32(zero) type mismatch after free");

    // --- Test MVN_VAL_I64 ---
    mvn_val_t val_i64_pos = mvn_val_i64(9876543210LL);
    TEST_ASSERT(val_i64_pos.type == MVN_VAL_I64, "I64(pos) type mismatch");
    TEST_ASSERT(val_i64_pos.i64 == 9876543210LL, "I64(pos) value mismatch");
    mvn_val_free(&val_i64_pos);
    TEST_ASSERT(val_i64_pos.type == MVN_VAL_NULL, "I64(pos) type mismatch after free");

    mvn_val_t val_i64_neg = mvn_val_i64(-1029384756LL);
    TEST_ASSERT(val_i64_neg.type == MVN_VAL_I64, "I64(neg) type mismatch");
    TEST_ASSERT(val_i64_neg.i64 == -1029384756LL, "I64(neg) value mismatch");
    mvn_val_free(&val_i64_neg);
    TEST_ASSERT(val_i64_neg.type == MVN_VAL_NULL, "I64(neg) type mismatch after free");

    mvn_val_t val_i64_zero = mvn_val_i64(0LL);
    TEST_ASSERT(val_i64_zero.type == MVN_VAL_I64, "I64(zero) type mismatch");
    TEST_ASSERT(val_i64_zero.i64 == 0LL, "I64(zero) value mismatch");
    mvn_val_free(&val_i64_zero);
    TEST_ASSERT(val_i64_zero.type == MVN_VAL_NULL, "I64(zero) type mismatch after free");

    // --- Test MVN_VAL_U32 ---
    mvn_val_t val_u32_pos = mvn_val_u32(12345U);
    TEST_ASSERT(val_u32_pos.type == MVN_VAL_U32, "U32(pos) type mismatch");
    TEST_ASSERT(val_u32_pos.u32 == 12345U, "U32(pos) value mismatch");
    mvn_val_free(&val_u32_pos);
    TEST_ASSERT(val_u32_pos.type == MVN_VAL_NULL, "U32(pos) type mismatch after free");

    mvn_val_t val_u32_zero = mvn_val_u32(0U);
    TEST_ASSERT(val_u32_zero.type == MVN_VAL_U32, "U32(zero) type mismatch");
    TEST_ASSERT(val_u32_zero.u32 == 0U, "U32(zero) value mismatch");
    mvn_val_free(&val_u32_zero);
    TEST_ASSERT(val_u32_zero.type == MVN_VAL_NULL, "U32(zero) type mismatch after free");

    // --- Test MVN_VAL_U8 ---
    mvn_val_t val_u8_val = mvn_val_u8(UINT8_C(250));
    TEST_ASSERT(val_u8_val.type == MVN_VAL_U8, "U8 type mismatch");
    TEST_ASSERT(val_u8_val.u8 == UINT8_C(250), "U8 value mismatch");
    mvn_val_free(&val_u8_val);
    TEST_ASSERT(val_u8_val.type == MVN_VAL_NULL, "U8 type mismatch after free");

    // --- Test MVN_VAL_U16 ---
    mvn_val_t val_u16_val = mvn_val_u16(UINT16_C(60000));
    TEST_ASSERT(val_u16_val.type == MVN_VAL_U16, "U16 type mismatch");
    TEST_ASSERT(val_u16_val.u16 == UINT16_C(60000), "U16 value mismatch");
    mvn_val_free(&val_u16_val);
    TEST_ASSERT(val_u16_val.type == MVN_VAL_NULL, "U16 type mismatch after free");

    // --- Test MVN_VAL_U64 ---
    mvn_val_t val_u64_pos = mvn_val_u64(9876543210ULL);
    TEST_ASSERT(val_u64_pos.type == MVN_VAL_U64, "U64(pos) type mismatch");
    TEST_ASSERT(val_u64_pos.u64 == 9876543210ULL, "U64(pos) value mismatch");
    mvn_val_free(&val_u64_pos);
    TEST_ASSERT(val_u64_pos.type == MVN_VAL_NULL, "U64(pos) type mismatch after free");

    mvn_val_t val_u64_zero = mvn_val_u64(0ULL);
    TEST_ASSERT(val_u64_zero.type == MVN_VAL_U64, "U64(zero) type mismatch");
    TEST_ASSERT(val_u64_zero.u64 == 0ULL, "U64(zero) value mismatch");
    mvn_val_free(&val_u64_zero);
    TEST_ASSERT(val_u64_zero.type == MVN_VAL_NULL, "U64(zero) type mismatch after free");

    // --- Test MVN_VAL_F32 ---
    mvn_val_t val_f32_pos = mvn_val_f32(123.456f);
    TEST_ASSERT(val_f32_pos.type == MVN_VAL_F32, "F32(pos) type mismatch");
    TEST_ASSERT_FLOAT_EQ(val_f32_pos.f32, 123.456f, "F32(pos) value mismatch");
    mvn_val_free(&val_f32_pos);
    TEST_ASSERT(val_f32_pos.type == MVN_VAL_NULL, "F32(pos) type mismatch after free");

    mvn_val_t val_f32_neg = mvn_val_f32(-987.654f);
    TEST_ASSERT(val_f32_neg.type == MVN_VAL_F32, "F32(neg) type mismatch");
    TEST_ASSERT_FLOAT_EQ(val_f32_neg.f32, -987.654f, "F32(neg) value mismatch");
    mvn_val_free(&val_f32_neg);
    TEST_ASSERT(val_f32_neg.type == MVN_VAL_NULL, "F32(neg) type mismatch after free");

    mvn_val_t val_f32_zero = mvn_val_f32(0.0f);
    TEST_ASSERT(val_f32_zero.type == MVN_VAL_F32, "F32(zero) type mismatch");
    TEST_ASSERT_FLOAT_EQ(val_f32_zero.f32, 0.0f, "F32(zero) value mismatch");
    mvn_val_free(&val_f32_zero);
    TEST_ASSERT(val_f32_zero.type == MVN_VAL_NULL, "F32(zero) type mismatch after free");

    // --- Test MVN_VAL_F64 ---
    mvn_val_t val_f64_pos = mvn_val_f64(123456.789012);
    TEST_ASSERT(val_f64_pos.type == MVN_VAL_F64, "F64(pos) type mismatch");
    TEST_ASSERT_DOUBLE_EQ(val_f64_pos.f64, 123456.789012, "F64(pos) value mismatch");
    mvn_val_free(&val_f64_pos);
    TEST_ASSERT(val_f64_pos.type == MVN_VAL_NULL, "F64(pos) type mismatch after free");

    mvn_val_t val_f64_neg = mvn_val_f64(-987654.321098);
    TEST_ASSERT(val_f64_neg.type == MVN_VAL_F64, "F64(neg) type mismatch");
    TEST_ASSERT_DOUBLE_EQ(val_f64_neg.f64, -987654.321098, "F64(neg) value mismatch");
    mvn_val_free(&val_f64_neg);
    TEST_ASSERT(val_f64_neg.type == MVN_VAL_NULL, "F64(neg) type mismatch after free");

    mvn_val_t val_f64_zero = mvn_val_f64(0.0);
    TEST_ASSERT(val_f64_zero.type == MVN_VAL_F64, "F64(zero) type mismatch");
    TEST_ASSERT_DOUBLE_EQ(val_f64_zero.f64, 0.0, "F64(zero) value mismatch");
    mvn_val_free(&val_f64_zero);
    TEST_ASSERT(val_f64_zero.type == MVN_VAL_NULL, "F64(zero) type mismatch after free");

    // --- Test MVN_VAL_CHAR ---
    mvn_val_t val_char_alpha = mvn_val_char('A');
    TEST_ASSERT(val_char_alpha.type == MVN_VAL_CHAR, "Char(A) type mismatch");
    TEST_ASSERT(val_char_alpha.c == 'A', "Char(A) value mismatch");
    mvn_val_free(&val_char_alpha);
    TEST_ASSERT(val_char_alpha.type == MVN_VAL_NULL, "Char(A) type mismatch after free");

    mvn_val_t val_char_digit = mvn_val_char('7');
    TEST_ASSERT(val_char_digit.type == MVN_VAL_CHAR, "Char(7) type mismatch");
    TEST_ASSERT(val_char_digit.c == '7', "Char(7) value mismatch");
    mvn_val_free(&val_char_digit);
    TEST_ASSERT(val_char_digit.type == MVN_VAL_NULL, "Char(7) type mismatch after free");

    mvn_val_t val_char_null = mvn_val_char('\0');
    TEST_ASSERT(val_char_null.type == MVN_VAL_CHAR, "Char(\\0) type mismatch");
    TEST_ASSERT(val_char_null.c == '\0', "Char(\\0) value mismatch");
    mvn_val_free(&val_char_null);
    TEST_ASSERT(val_char_null.type == MVN_VAL_NULL, "Char(\\0) type mismatch after free");

    // --- Test MVN_VAL_PTR ---
    int       dummy_int_for_ptr = 42;
    void     *ptr_val_raw       = &dummy_int_for_ptr;
    mvn_val_t val_ptr_val       = mvn_val_ptr(ptr_val_raw);
    TEST_ASSERT(val_ptr_val.type == MVN_VAL_PTR, "Ptr type mismatch");
    TEST_ASSERT(val_ptr_val.ptr == ptr_val_raw, "Ptr value mismatch");
    mvn_val_free(&val_ptr_val); // Should not free ptr_val_raw, just reset type
    TEST_ASSERT(val_ptr_val.type == MVN_VAL_NULL, "Ptr type mismatch after free");
    TEST_ASSERT(*(int *)ptr_val_raw == 42, "Pointed-to data for Ptr should remain valid");

    mvn_val_t val_ptr_null = mvn_val_ptr(NULL);
    TEST_ASSERT(val_ptr_null.type == MVN_VAL_PTR, "Ptr(NULL) type mismatch");
    TEST_ASSERT(val_ptr_null.ptr == NULL, "Ptr(NULL) value mismatch");
    mvn_val_free(&val_ptr_null);
    TEST_ASSERT(val_ptr_null.type == MVN_VAL_NULL, "Ptr(NULL) type mismatch after free");

    return true; // All tests passed
}

static bool test_primitive_equality_detailed(void)
{
    /*
     * Copyright (c) 2024 Jake Larson
     */
    mvn_val_t val_null1 = mvn_val_null();
    mvn_val_t val_null2 = mvn_val_null();

    mvn_val_t val_bool_t1 = mvn_val_bool(true);
    mvn_val_t val_bool_t2 = mvn_val_bool(true);
    mvn_val_t val_bool_f1 = mvn_val_bool(false);

    mvn_val_t val_i8_5  = mvn_val_i8(INT8_C(5));
    mvn_val_t val_i8_5b = mvn_val_i8(INT8_C(5));
    mvn_val_t val_i8_6  = mvn_val_i8(INT8_C(6));

    mvn_val_t val_i16_500  = mvn_val_i16(INT16_C(500));
    mvn_val_t val_i16_500b = mvn_val_i16(INT16_C(500));
    mvn_val_t val_i16_600  = mvn_val_i16(INT16_C(600));

    mvn_val_t val_i32_10  = mvn_val_i32(10);
    mvn_val_t val_i32_10b = mvn_val_i32(10);
    mvn_val_t val_i32_20  = mvn_val_i32(20);

    mvn_val_t val_i64_50  = mvn_val_i64(50LL);
    mvn_val_t val_i64_50b = mvn_val_i64(50LL);
    mvn_val_t val_i64_60  = mvn_val_i64(60LL);

    mvn_val_t val_u32_100  = mvn_val_u32(100U);
    mvn_val_t val_u32_100b = mvn_val_u32(100U);
    mvn_val_t val_u32_200  = mvn_val_u32(200U);

    mvn_val_t val_u8_20  = mvn_val_u8(UINT8_C(20));
    mvn_val_t val_u8_20b = mvn_val_u8(UINT8_C(20));
    mvn_val_t val_u8_30  = mvn_val_u8(UINT8_C(30));

    mvn_val_t val_u16_700  = mvn_val_u16(UINT16_C(700));
    mvn_val_t val_u16_700b = mvn_val_u16(UINT16_C(700));
    mvn_val_t val_u16_800  = mvn_val_u16(UINT16_C(800));

    mvn_val_t val_u64_500  = mvn_val_u64(500ULL);
    mvn_val_t val_u64_500b = mvn_val_u64(500ULL);
    mvn_val_t val_u64_600  = mvn_val_u64(600ULL);

    mvn_val_t val_f32_1p23  = mvn_val_f32(1.23f);
    mvn_val_t val_f32_1p23b = mvn_val_f32(1.23f);
    mvn_val_t val_f32_1p23e = mvn_val_f32(1.230000001f); // Within default epsilon for f32
    mvn_val_t val_f32_1p24  = mvn_val_f32(1.24f);

    mvn_val_t val_f64_3p1415  = mvn_val_f64(3.1415);
    mvn_val_t val_f64_3p1415b = mvn_val_f64(3.1415);
    mvn_val_t val_f64_3p1415e = mvn_val_f64(3.141500000000001); // Within default epsilon for f64
    mvn_val_t val_f64_3p1416  = mvn_val_f64(3.1416);

    mvn_val_t val_char_a  = mvn_val_char('a');
    mvn_val_t val_char_ab = mvn_val_char('a'); // Same as val_char_a
    mvn_val_t val_char_b  = mvn_val_char('b');

    int       dummy_ptr_target1 = 1;
    int       dummy_ptr_target2 = 2;
    mvn_val_t val_ptr_1         = mvn_val_ptr(&dummy_ptr_target1);
    mvn_val_t val_ptr_1b        = mvn_val_ptr(&dummy_ptr_target1); // Points to same
    mvn_val_t val_ptr_2         = mvn_val_ptr(&dummy_ptr_target2);
    mvn_val_t val_ptr_null_eq   = mvn_val_ptr(NULL);
    mvn_val_t val_ptr_null_eq_b = mvn_val_ptr(NULL);

    // Same type, same value
    TEST_ASSERT(mvn_val_equal(&val_null1, &val_null2), "NULL == NULL failed");
    TEST_ASSERT(mvn_val_equal(&val_bool_t1, &val_bool_t2), "Bool(T) == Bool(T) failed");
    TEST_ASSERT(mvn_val_equal(&val_i8_5, &val_i8_5b), "I8(5) == I8(5) failed");
    TEST_ASSERT(mvn_val_equal(&val_i16_500, &val_i16_500b), "I16(500) == I16(500) failed");
    TEST_ASSERT(mvn_val_equal(&val_i32_10, &val_i32_10b), "I32(10) == I32(10) failed");
    TEST_ASSERT(mvn_val_equal(&val_i64_50, &val_i64_50b), "I64(50) == I64(50) failed");
    TEST_ASSERT(mvn_val_equal(&val_u32_100, &val_u32_100b), "U32(100) == U32(100) failed");
    TEST_ASSERT(mvn_val_equal(&val_u8_20, &val_u8_20b), "U8(20) == U8(20) failed");
    TEST_ASSERT(mvn_val_equal(&val_u16_700, &val_u16_700b), "U16(700) == U16(700) failed");
    TEST_ASSERT(mvn_val_equal(&val_u64_500, &val_u64_500b), "U64(500) == U64(500) failed");
    TEST_ASSERT(mvn_val_equal(&val_f32_1p23, &val_f32_1p23b), "F32(1.23) == F32(1.23) failed");
    TEST_ASSERT(mvn_val_equal(&val_f32_1p23, &val_f32_1p23e), "F32(1.23) == F32(1.23+eps) failed");
    TEST_ASSERT(mvn_val_equal(&val_f64_3p1415, &val_f64_3p1415b),
                "F64(3.1415) == F64(3.1415) failed");
    TEST_ASSERT(mvn_val_equal(&val_f64_3p1415, &val_f64_3p1415e),
                "F64(3.1415) == F64(3.1415+eps) failed");
    TEST_ASSERT(mvn_val_equal(&val_char_a, &val_char_ab), "Char(a) == Char(a) failed");
    TEST_ASSERT(mvn_val_equal(&val_ptr_1, &val_ptr_1b), "Ptr(1) == Ptr(1) failed");
    TEST_ASSERT(mvn_val_equal(&val_ptr_null_eq, &val_ptr_null_eq_b),
                "Ptr(NULL) == Ptr(NULL) failed");

    // Same type, different value
    TEST_ASSERT(!mvn_val_equal(&val_bool_t1, &val_bool_f1), "Bool(T) == Bool(F) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i8_5, &val_i8_6), "I8(5) == I8(6) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i16_500, &val_i16_600), "I16(500) == I16(600) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i32_10, &val_i32_20), "I32(10) == I32(20) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i64_50, &val_i64_60), "I64(50) == I64(60) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_u32_100, &val_u32_200), "U32(100) == U32(200) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_u8_20, &val_u8_30), "U8(20) == U8(30) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_u16_700, &val_u16_800), "U16(700) == U16(800) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_u64_500, &val_u64_600), "U64(500) == U64(600) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_f32_1p23, &val_f32_1p24),
                "F32(1.23) == F32(1.24) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_f64_3p1415, &val_f64_3p1416),
                "F64(3.1415) == F64(3.1416) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_char_a, &val_char_b), "Char(a) == Char(b) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_ptr_1, &val_ptr_2), "Ptr(1) == Ptr(2) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_ptr_1, &val_ptr_null_eq), "Ptr(1) == Ptr(NULL) reported true");

    // Different types
    TEST_ASSERT(!mvn_val_equal(&val_null1, &val_bool_t1), "NULL == Bool reported true");
    TEST_ASSERT(!mvn_val_equal(&val_bool_t1, &val_i8_5), "Bool == I8 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i8_5, &val_i16_500), "I8 == I16 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i16_500, &val_i32_10), "I16 == I32 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i32_10, &val_i64_50), "I32 == I64 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i64_50, &val_u8_20), "I64 == U8 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_u8_20, &val_u16_700), "U8 == U16 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_u16_700, &val_u32_100), "U16 == U32 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_u32_100, &val_u64_500), "U32 == U64 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_u64_500, &val_f32_1p23), "U64 == F32 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_f32_1p23, &val_f64_3p1415), "F32 == F64 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_f64_3p1415, &val_char_a), "F64 == Char reported true");
    TEST_ASSERT(!mvn_val_equal(&val_char_a, &val_ptr_1), "Char == Ptr reported true");
    TEST_ASSERT(!mvn_val_equal(&val_ptr_1, &val_null1), "Ptr == NULL reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i32_10, &val_u32_100),
                "I32 == U32 reported true"); // Signed vs Unsigned

    // Freeing primitives just resets their type, no actual memory deallocation for the value itself
    mvn_val_free(&val_null1);
    mvn_val_free(&val_null2);
    mvn_val_free(&val_bool_t1);
    mvn_val_free(&val_bool_t2);
    mvn_val_free(&val_bool_f1);
    mvn_val_free(&val_i32_10);
    mvn_val_free(&val_i32_10b);
    mvn_val_free(&val_i32_20);
    mvn_val_free(&val_i64_50);
    mvn_val_free(&val_i64_50b);
    mvn_val_free(&val_i64_60);
    mvn_val_free(&val_u32_100);
    mvn_val_free(&val_u32_100b);
    mvn_val_free(&val_u32_200);
    mvn_val_free(&val_u64_500);
    mvn_val_free(&val_u64_500b);
    mvn_val_free(&val_u64_600);
    mvn_val_free(&val_f32_1p23);
    mvn_val_free(&val_f32_1p23b);
    mvn_val_free(&val_f32_1p23e);
    mvn_val_free(&val_f32_1p24);
    mvn_val_free(&val_f64_3p1415);
    mvn_val_free(&val_f64_3p1415b);
    mvn_val_free(&val_f64_3p1415e);
    mvn_val_free(&val_f64_3p1416);
    mvn_val_free(&val_char_a);
    mvn_val_free(&val_char_ab);
    mvn_val_free(&val_char_b);
    mvn_val_free(&val_i8_5);
    mvn_val_free(&val_i8_5b);
    mvn_val_free(&val_i8_6);
    mvn_val_free(&val_i16_500);
    mvn_val_free(&val_i16_500b);
    mvn_val_free(&val_i16_600);
    mvn_val_free(&val_u8_20);
    mvn_val_free(&val_u8_20b);
    mvn_val_free(&val_u8_30);
    mvn_val_free(&val_u16_700);
    mvn_val_free(&val_u16_700b);
    mvn_val_free(&val_u16_800);
    mvn_val_free(&val_ptr_1);
    mvn_val_free(&val_ptr_1b);
    mvn_val_free(&val_ptr_2);
    mvn_val_free(&val_ptr_null_eq);
    mvn_val_free(&val_ptr_null_eq_b);

    return true;
}

static bool test_primitive_print(void)
{
    /*
     * Copyright (c) 2024 Jake Larson
     */
    printf("Testing mvn_val_print for all primitive types (visual check/no crash):\n");

    mvn_val_t val_null_p = mvn_val_null();
    printf("  Null: ");
    mvn_val_print(&val_null_p);
    printf("\n");

    mvn_val_t val_bool_t_p = mvn_val_bool(true);
    printf("  Bool(T): ");
    mvn_val_print(&val_bool_t_p);
    printf("\n");
    mvn_val_free(&val_bool_t_p);

    mvn_val_t val_bool_f_p = mvn_val_bool(false);
    printf("  Bool(F): ");
    mvn_val_print(&val_bool_f_p);
    printf("\n");
    mvn_val_free(&val_bool_f_p);

    mvn_val_t val_i32_p = mvn_val_i32(42);
    printf("  I32: ");
    mvn_val_print(&val_i32_p);
    printf("\n");
    mvn_val_free(&val_i32_p);

    mvn_val_t val_i64_p = mvn_val_i64(1234567890123LL);
    printf("  I64: ");
    mvn_val_print(&val_i64_p);
    printf("\n");
    mvn_val_free(&val_i64_p);

    mvn_val_t val_i8_p = mvn_val_i8(INT8_C(-100));
    printf("  I8: ");
    mvn_val_print(&val_i8_p);
    printf("\n");
    mvn_val_free(&val_i8_p);

    mvn_val_t val_i16_p = mvn_val_i16(INT16_C(20000));
    printf("  I16: ");
    mvn_val_print(&val_i16_p);
    printf("\n");
    mvn_val_free(&val_i16_p);

    mvn_val_t val_u32_p = mvn_val_u32(3000000000U);
    printf("  U32: ");
    mvn_val_print(&val_u32_p);
    printf("\n");
    mvn_val_free(&val_u32_p);

    mvn_val_t val_u64_p = mvn_val_u64(12345678901234567890ULL);
    printf("  U64: ");
    mvn_val_print(&val_u64_p);
    printf("\n");
    mvn_val_free(&val_u64_p);

    mvn_val_t val_u8_p = mvn_val_u8(UINT8_C(200));
    printf("  U8: ");
    mvn_val_print(&val_u8_p);
    printf("\n");
    mvn_val_free(&val_u8_p);

    mvn_val_t val_u16_p = mvn_val_u16(UINT16_C(50000));
    printf("  U16: ");
    mvn_val_print(&val_u16_p);
    printf("\n");
    mvn_val_free(&val_u16_p);

    mvn_val_t val_f32_p = mvn_val_f32(3.14f);
    printf("  F32: ");
    mvn_val_print(&val_f32_p);
    printf("\n");
    mvn_val_free(&val_f32_p);

    mvn_val_t val_f64_p = mvn_val_f64(2.718281828459);
    printf("  F64: ");
    mvn_val_print(&val_f64_p);
    printf("\n");
    mvn_val_free(&val_f64_p);

    mvn_val_t val_char_p = mvn_val_char('X');
    printf("  Char: ");
    mvn_val_print(&val_char_p);
    printf("\n");
    mvn_val_free(&val_char_p);

    int       dummy_print_ptr_target = 77;
    mvn_val_t val_ptr_p              = mvn_val_ptr(&dummy_print_ptr_target);
    printf("  Ptr: ");
    mvn_val_print(&val_ptr_p);
    printf("\n");
    mvn_val_free(&val_ptr_p);

    printf("End of mvn_val_print primitive types test.\n");
    return true; // Test passed if no crashes
}

static bool test_val_type_to_string_conversion(void)
{
    /*
     * Copyright (c) 2024 Jake Larson
     */
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_NULL), "NULL") == 0,
                "MVN_VAL_NULL to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_BOOL), "BOOL") == 0,
                "MVN_VAL_BOOL to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_I8), "I8") == 0, "MVN_VAL_I8 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_I16), "I16") == 0,
                "MVN_VAL_I16 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_I32), "I32") == 0,
                "MVN_VAL_I32 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_I64), "I64") == 0,
                "MVN_VAL_I64 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_U8), "U8") == 0, "MVN_VAL_U8 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_U16), "U16") == 0,
                "MVN_VAL_U16 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_U32), "U32") == 0,
                "MVN_VAL_U32 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_U64), "U64") == 0,
                "MVN_VAL_U64 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_F32), "F32") == 0,
                "MVN_VAL_F32 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_F64), "F64") == 0,
                "MVN_VAL_F64 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_CHAR), "CHAR") == 0,
                "MVN_VAL_CHAR to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_PTR), "PTR") == 0,
                "MVN_VAL_PTR to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_STRING), "STRING") == 0,
                "MVN_VAL_STRING to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_ARRAY), "ARRAY") == 0,
                "MVN_VAL_ARRAY to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_HASHMAP), "HASHMAP") == 0,
                "MVN_VAL_HASHMAP to string failed");
    // Test an out-of-range type, expecting "UNKNOWN"
    TEST_ASSERT(strcmp(mvn_val_type_to_str((mvn_val_type_t)999), "UNKNOWN") == 0,
                "Unknown type to string failed");

    return true;
}

static bool test_additional_primitive_operations(void)
{
    /*
     * Copyright (c) 2024 Jake Larson
     */
    printf("Testing mvn_val_print(NULL):\n");
    mvn_val_print(NULL); // Test printing a NULL mvn_val_t pointer
    printf("\nEnd of mvn_val_print(NULL) test.\n");

    mvn_val_t val_i32_sample  = mvn_val_i32(123);
    mvn_val_t val_str_sample  = mvn_val_str("sample"); // Changed from mvn_val_str_borrow
    mvn_val_t val_arr_sample  = mvn_val_arr();
    mvn_val_t val_hmap_sample = mvn_val_hmap();

    // Test mvn_val_equal with NULL mvn_val_t* pointers
    TEST_ASSERT(!mvn_val_equal(NULL, &val_i32_sample), "mvn_val_equal(NULL, &val) should be false");
    TEST_ASSERT(!mvn_val_equal(&val_i32_sample, NULL), "mvn_val_equal(&val, NULL) should be false");
    // Behavior of mvn_val_equal(NULL, NULL) depends on its implementation.
    // Assuming it returns false if either pointer is NULL, as per the snippet.
    // If it's defined that two NULL pointers are "equal", this assertion would change.
    TEST_ASSERT(mvn_val_equal(NULL, NULL),                   // Changed from !mvn_val_equal
                "mvn_val_equal(NULL, NULL) should be true"); // Updated message

    // Test mvn_val_equal comparing primitives with complex types (should be false due to type
    // mismatch)
    TEST_ASSERT(!mvn_val_equal(&val_i32_sample, &val_str_sample), "I32 == String should be false");
    TEST_ASSERT(!mvn_val_equal(&val_i32_sample, &val_arr_sample), "I32 == Array should be false");
    TEST_ASSERT(!mvn_val_equal(&val_i32_sample, &val_hmap_sample), "I32 == Hmap should be false");

    // Clean up complex types if they were fully created (not just borrowed for string)
    // If mvn_val_str_borrow was used, no free for val_str_sample.str
    // If mvn_val_str was used, then mvn_str_free(val_str_sample.str) or
    // mvn_val_free(&val_str_sample)
    mvn_val_free(&val_str_sample); // Frees the string if mvn_val_str created it
    mvn_val_free(&val_arr_sample);
    mvn_val_free(&val_hmap_sample);
    // val_i32_sample is primitive, mvn_val_free just resets its type

    return true; // Test passed
}

/**
 * \brief           Run all primitives tests
 * \param[out]      passed_tests: Pointer to passed tests counter
 * \param[out]      failed_tests: Pointer to failed tests counter
 * \param[out]      total_tests: Pointer to total tests counter
 */
int run_primitives_tests(int *passed_tests, int *failed_tests, int *total_tests)
{
    printf("\n===== RUNNING PRIMITIVES TESTS =====\n");

    int passed_before = *passed_tests;
    int failed_before = *failed_tests;

    RUN_TEST(test_primitives);
    RUN_TEST(test_additional_primitive_operations);
    RUN_TEST(test_primitive_equality_detailed);   // Added
    RUN_TEST(test_primitive_print);               // Added
    RUN_TEST(test_val_type_to_string_conversion); // Added

    int tests_run = (*passed_tests - passed_before) + (*failed_tests - failed_before);
    (*total_tests) += tests_run;

    printf("\n");                            // Add a newline after the tests for this module
    return (*failed_tests == failed_before); // Return true if no new failures
}

int main(void)
{
    int passed = 0;
    int failed = 0;
    int total  = 0;

    run_primitives_tests(&passed, &failed, &total);

    printf("\n===== PRIMITIVES TEST SUMMARY =====\n");
    print_test_summary(total, passed, failed);

    return (failed > 0) ? 1 : 0;
}
