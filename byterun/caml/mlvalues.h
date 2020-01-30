/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*          Xavier Leroy and Damien Doligez, INRIA Rocquencourt           */
/*                                                                        */
/*   Copyright 1996 Institut National de Recherche en Informatique et     */
/*     en Automatique.                                                    */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/

#ifndef CAML_MLVALUES_H
#define CAML_MLVALUES_H

#ifndef CAML_NAME_SPACE
#include "compatibility.h"
#endif
#include "config.h"
#include "misc.h"

/* Needed here for domain_state */
typedef intnat value;
typedef atomic_intnat atomic_value;
typedef int32_t opcode_t;
typedef opcode_t * code_t;

#include "domain_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions

  word: Four bytes on 32 and 16 bit architectures,
        eight bytes on 64 bit architectures.
  long: A C integer having the same number of bytes as a word.
  val: The ML representation of something.  A long or a block or a pointer
       outside the heap.  If it is a block, it is the (encoded) address
       of an object.  If it is a long, it is encoded as well.
  block: Something allocated.  It always has a header and some
          fields or some number of bytes (a multiple of the word size).
  field: A word-sized val which is part of a block.
  bp: Pointer to the first byte of a block.  (a char *)
  op: Pointer to the first field of a block.  (a value *)
  hp: Pointer to the header of a block.  (a char *)
  int32_t: Four bytes on all architectures.
  int64_t: Eight bytes on all architectures.

  Remark: A block size is always a multiple of the word size, and at least
          one word plus the header.

  bosize: Size (in bytes) of the "bytes" part.
  wosize: Size (in words) of the "fields" part.
  bhsize: Size (in bytes) of the block with its header.
  whsize: Size (in words) of the block with its header.

  hd: A header.
  tag: The value of the tag field of the header.
  color: The value of the color field of the header.
         This is for use only by the GC.
*/

typedef uintnat header_t;
typedef uintnat mlsize_t;
typedef unsigned int tag_t;             /* Actually, an unsigned char */
typedef uintnat color_t;
typedef uintnat mark_t;

/* Longs vs blocks. */
#define Is_long(x)   (((x) & 1) != 0)
#define Is_block(x)  (((x) & 1) == 0)

/* Conversion macro names are always of the form  "to_from". */
/* Example: Val_long as in "Val from long" or "Val of long". */
#define Val_long(x)     ((intnat) (((uintnat)(x) << 1)) + 1)
#define Long_val(x)     ((x) >> 1)
#define Max_long (((intnat)1 << (8 * sizeof(value) - 2)) - 1)
#define Min_long (-((intnat)1 << (8 * sizeof(value) - 2)))
#define Val_int(x) Val_long(x)
#define Int_val(x) ((int) Long_val(x))
#define Unsigned_long_val(x) ((uintnat)(x) >> 1)
#define Unsigned_int_val(x)  ((int) Unsigned_long_val(x))

/* Structure of the header:

For 16-bit and 32-bit architectures:
     +--------+-------+-----+
     | wosize | color | tag |
     +--------+-------+-----+
bits  31    10 9     8 7   0

For 64-bit architectures:

     +--------+-------+-----+
     | wosize | color | tag |
     +--------+-------+-----+
bits  63    10 9     8 7   0

For x86-64 with Spacetime profiling:
  P = PROFINFO_WIDTH (as set by "configure", currently 26 bits, giving a
    maximum block size of just under 4Gb)
     +----------------+----------------+-------------+
     | profiling info | wosize         | color | tag |
     +----------------+----------------+-------------+
bits  63        (64-P) (63-P)        10 9     8 7   0

*/

#define Tag_hd(hd) ((tag_t) ((hd) & 0xFF))

#define Gen_profinfo_shift(width) (64 - (width))
#define Gen_profinfo_mask(width) ((1ull << (width)) - 1ull)
#define Gen_profinfo_hd(width, hd) \
  (((mlsize_t) ((hd) >> (Gen_profinfo_shift(width)))) \
   & (Gen_profinfo_mask(width)))

#ifdef WITH_PROFINFO
#define PROFINFO_SHIFT (Gen_profinfo_shift(PROFINFO_WIDTH))
#define PROFINFO_MASK (Gen_profinfo_mask(PROFINFO_WIDTH))
#define Hd_no_profinfo(hd) ((hd) & ~(PROFINFO_MASK << PROFINFO_SHIFT))
#define Wosize_hd(hd) ((mlsize_t) ((Hd_no_profinfo(hd)) >> 10))
#define Profinfo_hd(hd) (Gen_profinfo_hd(PROFINFO_WIDTH, hd))
#else
#define Wosize_hd(hd) ((mlsize_t) ((hd) >> 10))
#endif /* WITH_PROFINFO */

#define Hd_val(val) (((header_t *) (val)) [-1] + 0)
#define Hp_atomic_val(val) ((atomic_uintnat *)(val) - 1)
#define Hd_hp(hp) (* ((header_t *) (hp)))              /* Also an l-value. */
#define Hp_val(val) (((header_t *) (val)) - 1)
#define Hp_op(op) (Hp_val (op))
#define Hp_bp(bp) (Hp_val (bp))
#define Val_op(op) ((value) (op))
#define Val_hp(hp) ((value) (((header_t *) (hp)) + 1))
#define Op_hp(hp) ((value *) Val_hp (hp))
#define Bp_hp(hp) ((char *) Val_hp (hp))

#define Num_tags (1 << 8)
#ifdef ARCH_SIXTYFOUR
#define Max_wosize (((intnat)1 << (54-PROFINFO_WIDTH)) - 1)
#else
#define Max_wosize ((1 << 22) - 1)
#endif /* ARCH_SIXTYFOUR */

#define Wosize_val(val) (Wosize_hd (Hd_val (val)))
#define Wosize_op(op) (Wosize_val (op))
#define Wosize_bp(bp) (Wosize_val (bp))
#define Wosize_hp(hp) (Wosize_hd (Hd_hp (hp)))
#define Whsize_wosize(sz) ((sz) + 1)
#define Wosize_whsize(sz) ((sz) - 1)
#define Wosize_bhsize(sz) ((sz) / sizeof (value) - 1)
#define Bsize_wsize(sz) ((sz) * sizeof (value))
#define Wsize_bsize(sz) ((sz) / sizeof (value))
#define Bhsize_wosize(sz) (Bsize_wsize (Whsize_wosize (sz)))
#define Bhsize_bosize(sz) ((sz) + sizeof (header_t))
#define Bosize_val(val) (Bsize_wsize (Wosize_val (val)))
#define Bosize_op(op) (Bosize_val (Val_op (op)))
#define Bosize_bp(bp) (Bosize_val (Val_bp (bp)))
#define Bosize_hd(hd) (Bsize_wsize (Wosize_hd (hd)))
#define Whsize_hp(hp) (Whsize_wosize (Wosize_hp (hp)))
#define Whsize_val(val) (Whsize_hp (Hp_val (val)))
#define Whsize_bp(bp) (Whsize_val (Val_bp (bp)))
#define Whsize_hd(hd) (Whsize_wosize (Wosize_hd (hd)))
#define Bhsize_hp(hp) (Bsize_wsize (Whsize_hp (hp)))
#define Bhsize_hd(hd) (Bsize_wsize (Whsize_hd (hd)))

#define Profinfo_val(val) (Profinfo_hd (Hd_val (val)))

#ifdef ARCH_BIG_ENDIAN
#define Tag_val(val) (((unsigned char *) (val)) [-1])
                                                 /* Also an l-value. */
#define Tag_hp(hp) (((unsigned char *) (hp)) [sizeof(value)-1])
                                                 /* Also an l-value. */
#else
#define Tag_val(val) (((unsigned char *) (val)) [-sizeof(value)])
                                                 /* Also an l-value. */
#define Tag_hp(hp) (((unsigned char *) (hp)) [0])
                                                 /* Also an l-value. */
#endif

/* The lowest tag for blocks containing no value. */
#define No_scan_tag 251


/* 1- If tag < No_scan_tag : a tuple of fields.  */

/* Pointer to the first field. */
#define Op_val(x) ((value *) (x))
#define Op_atomic_val(x) ((atomic_value *) (x))
/* Fields are numbered from 0. */
#define Field(x, i) (((value *)(x)) [i])           /* Also an l-value. */

/* All values which are not blocks in the current domain's minor heap
   differ from caml_domain_state in at least one of the bits set in
   Young_val_bitmask */
#define Young_val_bitmask \
  ((uintnat)1 | ~(((uintnat)1 << Minor_heap_align_bits) - (uintnat)1))

/* All values which are not blocks in any domain's minor heap differ
   from caml_domain_state in at least one of the bits set in
   Minor_val_bitmask */
#define Minor_val_bitmask \
  ((uintnat)1 | ~(((uintnat)1 << (Minor_heap_align_bits + Minor_heap_sel_bits)) - (uintnat)1))


/* Is_young(val) is true iff val is a block in the current domain's minor heap.
   Since the minor heap is allocated in one aligned block, this can be tested
   via bitmasking. */
#define Is_young(val) \
  ((((uintnat)(val) ^ (uintnat)Caml_state) & Young_val_bitmask) == 0)

/* Is_minor(val) is true iff val is a block in any domain's minor heap. */
#define Is_minor(val) \
  ((((uintnat)(val) ^ (uintnat)Caml_state) & Minor_val_bitmask) == 0)

/* NOTE: [Forward_tag] and [Infix_tag] must be just under
   [No_scan_tag], with [Infix_tag] the lower one.
   See [caml_oldify_one] in minor_gc.c for more details.

   NOTE: Update stdlib/obj.ml whenever you change the tags.
 */

/* Forward_tag: forwarding pointer that the GC may silently shortcut.
   See stdlib/lazy.ml. */
#define Forward_tag 250
#define Forward_val(v) Field_imm(v, 0) /* FIXME: not immutable once shortcutting is implemented */

/* If tag == Infix_tag : an infix header inside a closure */
/* Infix_tag must be odd so that the infix header is scanned as an integer */
/* Infix_tag must be 1 modulo 4 and infix headers can only occur in blocks
   with tag Closure_tag (see compact.c). */

#define Infix_tag 249
#define Infix_offset_hd(hd) (Bosize_hd(hd))
#define Infix_offset_val(v) Infix_offset_hd(Hd_val(v))

/* Another special case: objects */
#define Object_tag 248
#define Class_val(val) Field_imm((val), 0)
#define Oid_val(val) Long_field((val), 1)
CAMLextern value caml_get_public_method (value obj, value tag);
/* Called as:
   caml_callback(caml_get_public_method(obj, caml_hash_variant(name)), obj) */
/* caml_get_public_method returns 0 if tag not in the table.
   Note however that tags being hashed, same tag does not necessarily mean
   same method name. */

static inline value Val_ptr(void* p)
{
  CAMLassert(((value)p & 1) == 0);
  return (value)p + 1;
}
static inline void* Ptr_val(value val)
{
  CAMLassert(val & 1);
  return (void*)(val - 1);
}

#define Val_pc(pc) Val_ptr(pc)
#define Pc_val(val) ((code_t)Ptr_val(val))

/* Special case of tuples of fields: closures */
#define Closure_tag 247
#define Bytecode_val(val) (Pc_val(val))
#define Val_bytecode(code) (Val_pc(code))
#define Code_val(val) Bytecode_val(Field_imm((val), 0))

/* This tag is used (with Forward_tag) to implement lazy values.
   See major_gc.c and stdlib/lazy.ml. */
#define Lazy_tag 246

/* Tag used for continuations (see fiber.c) */
#define Cont_tag 245

/* Another special case: variants */
CAMLextern value caml_hash_variant(char const * tag);

/* 2- If tag >= No_scan_tag : a sequence of bytes. */

/* Pointer to the first byte */
#define Bp_val(v) ((char *) (v))
#define Val_bp(p) ((value) (p))
/* Bytes are numbered from 0. */
#define Byte(x, i) (((char *) (x)) [i])            /* Also an l-value. */
#define Byte_u(x, i) (((unsigned char *) (x)) [i]) /* Also an l-value. */

/* Abstract things.  Their contents is not traced by the GC; therefore they
   must not contain any [value]. Must have odd number so that headers with
   this tag cannot be mistaken for pointers (see caml_obj_truncate).
*/
#define Abstract_tag 251
#define Data_abstract_val(v) ((void*) Op_val(v))

/* Strings. */
#define String_tag 252
#ifdef CAML_SAFE_STRING
#define String_val(x) ((const char *) Bp_val(x))
#else
#define String_val(x) ((char *) Bp_val(x))
#endif
#define Bytes_val(x) ((unsigned char *) Bp_val(x))
CAMLextern mlsize_t caml_string_length (value);   /* size in bytes */
CAMLextern int caml_string_is_c_safe (value);
  /* true if string contains no '\0' null characters */

/* Floating-point numbers. */
#define Double_tag 253
#define Double_wosize ((sizeof(double) / sizeof(value)))
#ifndef ARCH_ALIGN_DOUBLE
#define Double_val(v) (* (double *)(v))
#define Store_double_val(v,d) (* (double *)(v) = (d))
#else
CAMLextern double caml_Double_val (value);
CAMLextern void caml_Store_double_val (value,double);
#define Double_val(v) caml_Double_val(v)
#define Store_double_val(v,d) caml_Store_double_val(v,d)
#endif

/* Arrays of floating-point numbers. */
#define Double_array_tag 254

/* The [_flat_field] macros are for [floatarray] values and float-only records.
*/
#define Double_flat_field(v,i) Double_val((value)((double *)(v) + (i)))
#define Store_double_flat_field(v,i,d) do{ \
  mlsize_t caml__temp_i = (i); \
  double caml__temp_d = (d); \
  Store_double_val((value)((double *) (v) + caml__temp_i), caml__temp_d); \
}while(0)

/* The [_array_field] macros are for [float array]. */
#ifdef FLAT_FLOAT_ARRAY
  #define Double_array_field(v,i) Double_flat_field(v,i)
  #define Store_double_array_field(v,i,d) Store_double_flat_field(v,i,d)
#else
  #define Double_array_field(v,i) Double_val (Field(v,i))
  CAMLextern void caml_Store_double_array_field (value, mlsize_t, double);
  #define Store_double_array_field(v,i,d) caml_Store_double_array_field (v,i,d)
#endif

/* The old [_field] macros are for backward compatibility only.
   They work with [floatarray], float-only records, and [float array]. */
#ifdef FLAT_FLOAT_ARRAY
  #define Double_field(v,i) Double_flat_field(v,i)
  #define Store_double_field(v,i,d) Store_double_flat_field(v,i,d)
#else
  static inline double Double_field (value v, mlsize_t i) {
    if (Tag_val (v) == Double_array_tag){
      return Double_flat_field (v, i);
    }else{
      return Double_array_field (v, i);
    }
  }
  static inline void Store_double_field (value v, mlsize_t i, double d) {
    if (Tag_val (v) == Double_array_tag){
      Store_double_flat_field (v, i, d);
    }else{
      Store_double_array_field (v, i, d);
    }
  }
#endif /* FLAT_FLOAT_ARRAY */

CAMLextern mlsize_t caml_array_length (value);   /* size in items */
CAMLextern int caml_is_double_array (value);   /* 0 is false, 1 is true */


/* Custom blocks.  They contain a pointer to a "method suite"
   of functions (for finalization, comparison, hashing, etc)
   followed by raw data.  The contents of custom blocks is not traced by
   the GC; therefore, they must not contain any [value].
   See [custom.h] for operations on method suites. */
#define Custom_tag 255
#define Data_custom_val(v) ((void *) (Op_val(v) + 1))
struct custom_operations;       /* defined in [custom.h] */

/* Int32.t, Int64.t and Nativeint.t are represented as custom blocks. */

#define Int32_val(v) (*((int32_t *) Data_custom_val(v)))
#define Nativeint_val(v) (*((intnat *) Data_custom_val(v)))
#ifndef ARCH_ALIGN_INT64
#define Int64_val(v) (*((int64_t *) Data_custom_val(v)))
#else
CAMLextern int64_t caml_Int64_val(value v);
#define Int64_val(v) caml_Int64_val(v)
#endif

/* 3- Atoms are 0-tuples.  They are statically allocated once and for all. */

CAMLextern value caml_atom(tag_t);
#define Atom(tag) caml_atom(tag)

/* Booleans are integers 0 or 1 */

#define Val_bool(x) Val_int((x) != 0)
#define Bool_val(x) Int_val(x)
#define Val_false Val_int(0)
#define Val_true Val_int(1)
#define Val_not(x) (Val_false + Val_true - (x))

/* The unit value is 0 (tagged) */

#define Val_unit Val_int(0)

/* List constructors */
#define Val_emptylist Val_int(0)
#define Tag_cons 0

CAMLextern value caml_set_oo_id(value obj);

CAMLextern value caml_set_oo_id(value obj);

/* Field access macros and functions */

static inline value Field_imm(value x, int i) {
  Assert (Hd_val(x));
  Assert (Tag_val(x) == Infix_tag || i < Wosize_val(x));
  value v = Op_val(x)[i];
  return v;
}

CAMLextern value caml_read_barrier(value, int);

static inline void caml_read_field(value x, int i, value* ret) {
  Assert (Hd_val(x));
  /* See Note [MM] in memory.c */
  value v = atomic_load_explicit(&Op_atomic_val(x)[i], memory_order_relaxed);
  *ret = v;
}

#define Int_field(x, i) Int_val(Op_val(x)[i])
#define Long_field(x, i) Long_val(Op_val(x)[i])
#define Bool_field(x, i) Bool_val(Op_val(x)[i])



#ifdef __cplusplus
}
#endif

#endif /* CAML_MLVALUES_H */