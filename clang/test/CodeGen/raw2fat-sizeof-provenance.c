// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -no-opaque-pointers \
// RUN:   -fraw2fat-sizeof-provenance -disable-llvm-passes -emit-llvm -o - %s \
// RUN:   | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -no-opaque-pointers \
// RUN:   -disable-llvm-passes -emit-llvm -o - %s \
// RUN:   | FileCheck %s --check-prefix=OFF

// sizeof of a record that holds pointers marks the operand of the
// instruction that consumes it; the IR itself is unchanged.

typedef __SIZE_TYPE__ size_t;

struct group {
  struct group *next;
  char *allocation;
  size_t alloc_size;
  unsigned in_use;
};

struct plain {
  long a, b, c, d;
};

void use(size_t, size_t);
void *alloc(size_t);

// CHECK-LABEL: define{{.*}} i32 @compare(
// CHECK: icmp uge i64 %{{.*}}, 32, !raw2fat.sizeof ![[GROUP:[0-9]+]]
int compare(size_t slop) { return slop >= sizeof(struct group); }

// CHECK-LABEL: define{{.*}} i64 @subtract(
// CHECK: sub i64 %{{.*}}, 32, !raw2fat.sizeof ![[GROUP]]
size_t subtract(size_t slop) {
  slop -= sizeof(struct group);
  return slop;
}

// The array form records the array type.
// CHECK-LABEL: define{{.*}} i64 @array(
// CHECK: mul i64 %{{.*}}, 64, !raw2fat.sizeof ![[ARRAY:[0-9]+]]
size_t array(size_t n) { return n * sizeof(struct group[2]); }

// A record without pointers is not marked.
// CHECK-LABEL: define{{.*}} i32 @no_pointers(
// CHECK: icmp ugt i64 %{{.*}}, 32{{$}}
int no_pointers(size_t x) { return x > sizeof(struct plain); }

// A value folded into another constant marks nothing, and the leftover
// record does not mark a later equal literal.
// CHECK-LABEL: define{{.*}} i32 @folded(
// CHECK: mul i64 64, %{{.*}}{{$}}
// CHECK: add i64 %{{.*}}, 32{{$}}
int folded(size_t n, size_t x) { return 2 * sizeof(struct group) * n + 32 > x; }

// An equal literal in the same instruction makes the operand ambiguous.
// CHECK-LABEL: define{{.*}} void @ambiguous(
// CHECK: call void @use(i64 noundef 32, i64 noundef 32){{$}}
void ambiguous(void) { use(32, sizeof(struct group)); }

// A call argument is marked.
// CHECK-LABEL: define{{.*}} i8* @allocate(
// CHECK: call i8* @alloc(i64 noundef 32), !raw2fat.sizeof ![[ALLOC:[0-9]+]]
void *allocate(void) { return alloc(sizeof(struct group)); }

// CHECK-DAG: ![[GROUP]] = !{!"v1", i32 1, %struct.group* null}
// CHECK-DAG: ![[ARRAY]] = !{!"v1", i32 1, [2 x %struct.group]* null}
// CHECK-DAG: ![[ALLOC]] = !{!"v1", i32 0, %struct.group* null}

// OFF-NOT: raw2fat.sizeof
