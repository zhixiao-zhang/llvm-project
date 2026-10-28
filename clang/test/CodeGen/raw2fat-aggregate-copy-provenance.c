// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -no-opaque-pointers \
// RUN:   -fraw2fat-sizeof-provenance -disable-llvm-passes -emit-llvm -o - %s \
// RUN:   | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -no-opaque-pointers \
// RUN:   -disable-llvm-passes -emit-llvm -o - %s \
// RUN:   | FileCheck %s --check-prefix=OFF

// An aggregate copy of a record that holds pointers is a memcpy of the
// record's size with no sizeof expression behind it. The memcpy records the
// copied type, so Raw2Fat can size the copy even after the optimizer has
// folded the casts that named the record. The length stays a constant.

struct group {
  struct group *next;
  char *allocation;
  long alloc_size;
  unsigned in_use;
};

struct plain {
  long a, b, c, d;
};

struct wrap {
  struct group groups[2];
};

// CHECK-LABEL: define{{.*}} void @copy_record(
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}, i64 32, i1 false){{.*}}!raw2fat.copy_type ![[GROUP:[0-9]+]]
// OFF-LABEL: define{{.*}} void @copy_record(
// OFF: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}, i64 32, i1 false)
// OFF-NOT: raw2fat
void copy_record(struct group *dst, struct group *src) { *dst = *src; }

// A record that holds an array of such records is marked as itself.
// CHECK-LABEL: define{{.*}} void @copy_wrapper(
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}, i64 64, i1 false){{.*}}!raw2fat.copy_type ![[WRAP:[0-9]+]]
void copy_wrapper(struct wrap *dst, struct wrap *src) { *dst = *src; }

// A record without pointers is not marked.
// CHECK-LABEL: define{{.*}} void @copy_plain(
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}, i64 32, i1 false){{$}}
void copy_plain(struct plain *dst, struct plain *src) { *dst = *src; }

// A copy that is not an aggregate assignment is not marked.
// CHECK-LABEL: define{{.*}} void @copy_bytes(
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}, i64 16, i1 false){{$}}
void copy_bytes(struct group *dst, struct group *src) {
  __builtin_memcpy(dst, src, 16);
}

// CHECK: ![[GROUP]] = !{!"v1", %struct.group* null}
// CHECK: ![[WRAP]] = !{!"v1", %struct.wrap* null}
