// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -no-opaque-pointers \
// RUN:   -disable-llvm-passes -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -disable-llvm-passes \
// RUN:   -emit-llvm -o - %s | FileCheck %s --check-prefix=OPAQUE

struct alpha {
  int tag;
  void (*callback)(struct alpha *);
};

struct omega {
  int tag;
  void (*callback)(struct omega *);
};

struct gamma {
  unsigned first : 1;
  unsigned second : 1;
  unsigned third : 1;
  int tail;
  void (*callback)(struct gamma *);
};

void use_alpha(struct alpha *a) {
  a->tag = 1;
}

void use_omega(struct omega *o) {
  o->tag = 2;
}

void use_gamma(struct gamma *g) {
  g->tail = 3;
}

// CHECK: %struct.alpha = type { i32, {}* }
// CHECK: %struct.omega = type { i32, {}* }
// CHECK: %struct.gamma = type { i8, i32, {}* }
// CHECK: !raw2fat.fptr_field_types = !{![[ALPHA:[0-9]+]], ![[GAMMA:[0-9]+]], ![[OMEGA:[0-9]+]]}
// CHECK: ![[ALPHA]] = !{!"v1", %struct.alpha* null, i32 1, void (%struct.alpha*)* null}
// CHECK: ![[GAMMA]] = !{!"v1", %struct.gamma* null, i32 2, void (%struct.gamma*)* null}
// CHECK: ![[OMEGA]] = !{!"v1", %struct.omega* null, i32 1, void (%struct.omega*)* null}

// OPAQUE: %struct.alpha = type { i32, ptr }
// OPAQUE: %struct.omega = type { i32, ptr }
// OPAQUE: %struct.gamma = type { i8, i32, ptr }
// OPAQUE-NOT: !raw2fat.fptr_field_types
