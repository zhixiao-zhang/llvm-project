%struct.sv = type { i8*, %union.anon_A }
%union.anon_A = type { i8* }
%union.XOPRETANY = type { i8* }
%union.YYSTYPE = type { i8* }

@sv2 = global %struct.sv zeroinitializer
@xop2 = global %union.XOPRETANY zeroinitializer
@yy = global %union.YYSTYPE zeroinitializer
