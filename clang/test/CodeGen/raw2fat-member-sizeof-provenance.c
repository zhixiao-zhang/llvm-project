// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -no-opaque-pointers \
// RUN:   -fraw2fat-sizeof-provenance -disable-llvm-passes -emit-llvm -o - %s \
// RUN:   | FileCheck %s

// sizeof of a member that holds pointers but is no such record marks the
// consumer with the record and the member's index path in it.

typedef __SIZE_TYPE__ size_t;

struct data {
  unsigned count;
  unsigned char *what;
  void *data[1];
};

struct outer {
  int tag;
  struct data inner;
};

union either {
  void *p;
  long n;
};

struct with_union {
  union either u;
};

// An element of a trailing array member, as a record that grows its trailing
// array is sized by.
// CHECK-LABEL: define{{.*}} i64 @element(
// CHECK: mul i64 8, %{{.*}}, !raw2fat.member_sizeof ![[ELEMENT:[0-9]+]]
size_t element(struct data *r, size_t n) {
  return sizeof(r->data[0]) * n;
}

// A dereference of the decayed array is the same element.
// CHECK-LABEL: define{{.*}} i64 @deref(
// CHECK: mul i64 8, %{{.*}}, !raw2fat.member_sizeof ![[ELEMENT]]
size_t deref(struct data *r, size_t n) { return sizeof *r->data * n; }

// The whole array member, and a pointer member.
// CHECK-LABEL: define{{.*}} i32 @members(
// CHECK: icmp ugt i64 %{{.*}}, 8, !raw2fat.member_sizeof ![[ARRAY:[0-9]+]]
// CHECK: icmp ugt i64 %{{.*}}, 8, !raw2fat.member_sizeof ![[WHAT:[0-9]+]]
int members(struct data *r, size_t x, size_t y) {
  return (x > sizeof(r->data)) + (y > sizeof(r->what));
}

// A nested member names its innermost record.
// CHECK-LABEL: define{{.*}} i64 @nested(
// CHECK: mul i64 8, %{{.*}}, !raw2fat.member_sizeof ![[ELEMENT]]
size_t nested(struct outer *o, size_t n) {
  return sizeof(o->inner.data[0]) * n;
}

// The pointee of a pointer member is not part of the record.
// CHECK-LABEL: define{{.*}} i64 @pointee(
// CHECK: mul i64 8, %{{.*}}{{$}}
size_t pointee(void ***pp, size_t n) {
  struct { void **p; } s = {*pp};
  return sizeof(s.p[0]) * n;
}

// A type, a member without pointers, and a union member are not marked.
// CHECK-LABEL: define{{.*}} i64 @unmarked(
// CHECK: mul i64 8, %{{.*}}{{$}}
// CHECK: mul i64 4, %{{.*}}{{$}}
// CHECK: mul i64 8, %{{.*}}{{$}}
size_t unmarked(struct data *r, struct with_union *w, size_t n) {
  return sizeof(void *) * n + sizeof(r->count) * n + sizeof(w->u.p) * n;
}

// CHECK-DAG: ![[ELEMENT]] = !{!"v1", i32 0, %struct.data* null, ![[ELEMENT_PATH:[0-9]+]]}
// CHECK-DAG: ![[ELEMENT_PATH]] = !{i32 2, i32 0}
// CHECK-DAG: ![[ARRAY]] = !{!"v1", i32 1, %struct.data* null, ![[ARRAY_PATH:[0-9]+]]}
// CHECK-DAG: ![[ARRAY_PATH]] = !{i32 2}
// CHECK-DAG: ![[WHAT]] = !{!"v1", i32 1, %struct.data* null, ![[WHAT_PATH:[0-9]+]]}
// CHECK-DAG: ![[WHAT_PATH]] = !{i32 1}
