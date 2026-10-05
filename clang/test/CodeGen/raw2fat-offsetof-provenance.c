// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -no-opaque-pointers \
// RUN:   -fraw2fat-sizeof-provenance -disable-llvm-passes -emit-llvm -o - %s \
// RUN:   | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -no-opaque-pointers \
// RUN:   -disable-llvm-passes -emit-llvm -o - %s \
// RUN:   | FileCheck %s --check-prefix=OFF

// offsetof into a record that holds pointers marks the operand of the
// instruction that consumes it, like sizeof; the IR itself is unchanged.

typedef __SIZE_TYPE__ size_t;
#define offsetof(t, m) __builtin_offsetof(t, m)

struct he {
  struct he *next;
  void *hek;
  long value;
};
struct hek {
  unsigned hash;
  int len;
  char key[1];
};
struct shared_he {
  struct he he;
  struct hek hek;
};
struct plain {
  long a, b;
};

void *alloc(size_t);

// Perl's S_share_hek_flags: the header size of a combined allocation.
// CHECK-LABEL: define{{.*}} i8* @share(
// CHECK: add i64 32, %{{.*}}, !raw2fat.offsetof ![[SHARED:[0-9]+]]
void *share(size_t len) {
  return alloc(offsetof(struct shared_he, hek.key[0]) + len + 2);
}

// container_of: the record pointer back from a member address. Pointer minus
// integer negates the offset, so the GEP operand is recorded as -1 - 1.
// CHECK-LABEL: define{{.*}} %struct.shared_he* @from_hek(
// CHECK: getelementptr inbounds i8, i8* %{{.*}}, i64 -24, !raw2fat.offsetof ![[NEG:[0-9]+]]
struct shared_he *from_hek(struct hek *k) {
  return (struct shared_he *)((char *)k - offsetof(struct shared_he, hek));
}

// The first member is always at offset 0 and is not marked.
// CHECK-LABEL: define{{.*}} i64 @first(
// CHECK-NOT: !raw2fat.offsetof
// CHECK: ret i64
size_t first(size_t n) { return n + offsetof(struct he, next); }

// Records without pointers keep their layout.
// CHECK-LABEL: define{{.*}} i64 @plain(
// CHECK-NOT: !raw2fat.offsetof
// CHECK: ret i64
size_t plain(size_t n) { return n + offsetof(struct plain, b); }

// CHECK: ![[SHARED]] = !{!"v1", i32 0, %struct.shared_he* null}
// CHECK: ![[NEG]] = !{!"v1", i32 -2, %struct.shared_he* null}

// OFF-NOT: raw2fat.offsetof
