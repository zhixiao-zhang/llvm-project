; Isomorphic struct types merge only when their names agree up to the
; ".N" suffixes LLVMContext appends on a collision.
; RUN: llvm-link -opaque-pointers=0 -S %s %p/Inputs/type-merge-by-name.ll | FileCheck %s

; CHECK-DAG: %struct.sv = type { i8*, %union.anon_A }
; CHECK-DAG: %union.anon_A = type { i8* }
; CHECK-DAG: %struct.av = type { i8*, %union.anon_A }
; CHECK-DAG: %union.XOPRETANY = type { i8* }
; CHECK-DAG: %union.YYSTYPE = type { i8* }
; CHECK-NOT: %union.anon_A.
; CHECK-NOT: %union.XOPRETANY.
; CHECK-NOT: %union.YYSTYPE.

%struct.sv = type { i8*, %union.anon_A }
%union.anon_A = type { i8* }
%struct.av = type { i8*, %union.anon_A.0 }
%union.anon_A.0 = type { i8* }
%union.XOPRETANY = type { i8* }

@sv = global %struct.sv zeroinitializer
@av = global %struct.av zeroinitializer
@xop = global %union.XOPRETANY zeroinitializer

; CHECK-DAG: @sv = global %struct.sv
; CHECK-DAG: @av = global %struct.av
; CHECK-DAG: @xop = global %union.XOPRETANY
; CHECK-DAG: @sv2 = global %struct.sv
; CHECK-DAG: @xop2 = global %union.XOPRETANY
; CHECK-DAG: @yy = global %union.YYSTYPE
