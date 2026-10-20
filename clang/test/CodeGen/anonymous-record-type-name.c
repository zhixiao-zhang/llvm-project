// RUN: %clang_cc1 -triple x86_64-linux-gnu -no-opaque-pointers -fraw2fat-anonymous-record-names -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-linux-gnu -no-opaque-pointers -emit-llvm %s -o - | FileCheck %s --check-prefix=DEFAULT

// With -fraw2fat-anonymous-record-names, an untagged record without a typedef
// name is named after its members: the same member list gets the same name in
// every translation unit and every enclosing record, and a different member
// list gets another name. Without the flag, such records stay "anon".

// CHECK: %struct.sv = type { i8*, %[[SVU:union\.anon_[0-9A-F]+]] }
// CHECK: %[[SVU]] = type { i8* }
// CHECK: %struct.av = type { i8*, %[[SVU]].0 }
// CHECK: %struct.xop = type { %[[XOP:union\.anon_[0-9A-F]+]] }
// CHECK-NOT: %[[SVU]] = type
// CHECK: %union.YYSTYPE = type { i8* }
// CHECK: %struct.nested = type { [2 x %[[INNER:struct\.anon_[0-9A-F]+]]] }
// CHECK: %[[INNER]] = type { %[[INNERU:union\.anon_[0-9A-F]+]] }
// CHECK: %struct.named_inner = type { %struct.anon_{{[0-9A-F]+}} }

// DEFAULT: %struct.sv = type { i8*, %union.anon }
// DEFAULT: %struct.av = type { i8*, %union.anon.0 }

#define SV_HEAD_UNION union { char *svu_pv; long svu_iv; } sv_u
struct sv { void *any; SV_HEAD_UNION; };
struct av { void *any; SV_HEAD_UNION; };
struct xop { union { char *xop_name; int xop_class; } u; };
typedef union { int ival; char *pval; } YYSTYPE;
struct nested { struct { union { char *p; } u; } items[2]; };
struct named_inner { struct { union { char *q; } u; } item; };

struct sv sv; struct av av; struct xop xop; YYSTYPE yy;
struct nested nested; struct named_inner named_inner;
