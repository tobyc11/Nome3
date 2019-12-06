grammar Nom;

file : command* EOF;

expression
   :  ident LPAREN expression RPAREN # Call
   |  expression  POW expression # BinOp
   |  expression  (TIMES | DIV)  expression # BinOp
   |  expression  (PLUS | MINUS) expression # BinOp
   |  LPAREN expression RPAREN # SubExp
   |  (PLUS | MINUS)* atom # UnaryOp
   |  '{' 'expr' expression '}' # SubExp
   ;

atom
   : scientific
   | ident
   ;

scientific
   : SCIENTIFIC_NUMBER
   ;

ident
   : IDENT
   | '$' IDENT
   ;

argClosed : 'closed' ;
argHidden : 'hidden' ;
argSurface : 'surface' ident ;
argSlices : 'slices' expression ;
argOrder : 'order' expression ;
argTransform
   : 'rotate' LPAREN expression expression expression RPAREN LPAREN expression RPAREN
   | 'scale' LPAREN expression expression expression RPAREN
   | 'translate' LPAREN expression expression expression RPAREN
   ;
argColor : 'color' LPAREN expression expression expression RPAREN ;

command
   : 'point' ident LPAREN expression expression expression RPAREN 'endpoint' # CmdPoint
   | 'polyline' ident LPAREN ident* RPAREN argClosed* 'endpolyline' # CmdPolyline
   | 'face' ident LPAREN ident* RPAREN argSurface* 'endface' # CmdFace
   | 'object' ident LPAREN ident* RPAREN 'endobject' # CmdObject
   | 'mesh' ident command* 'endmesh' # CmdMesh
   | 'group' ident command* 'endgroup' # CmdGroup
   | 'circle' ident LPAREN expression expression RPAREN 'endcircle' # CmdCircle
   | 'funnel' ident LPAREN expression expression expression expression RPAREN 'endfunnel' # CmdFunnel
   | 'tunnel' ident LPAREN expression expression expression expression RPAREN 'endtunnel' # CmdTunnel
   | 'beziercurve' ident LPAREN ident* RPAREN argSlices* 'endbeziercurve' # CmdBezierCurve
   | 'bspline' ident LPAREN ident* RPAREN (argClosed | argSlices | argOrder)* 'endbspline' # CmdBspline
   | 'instance' ident ident (argSurface | argTransform | argHidden)* 'endinstance' # CmdInstance
   | 'surface' ident argColor 'endsurface' # CmdSurface
   | 'background' argSurface 'endbackground' # CmdBackground
   | 'foreground' argSurface 'endforeground' # CmdForeground
   | 'insidefaces' argSurface 'endinsidefaces' # CmdInsideFaces
   | 'outsidefaces' argSurface 'endoutsidefaces' # CmdOutsideFaces
   | 'offsetfaces' argSurface 'endoffsetfaces' # CmdOffsetFaces
   | 'frontfaces' argSurface 'endfrontfaces' # CmdFrontFaces
   | 'backfaces' argSurface 'endbackfaces' # CmdBackFaces
   | 'rimfaces' argSurface 'endrimfaces' # CmdRimFaces
   | 'bank' ident set* 'endbank' # CmdBank
   | 'delete' deleteFace* 'enddelete' # CmdDelete
   | 'subdivision' ident 'type' ident 'subdivisions' expression 'endsubdivision' # CmdSubdivision
   | 'offset' ident 'type' ident 'min' expression 'max' expression 'step' expression 'endoffset' # CmdOffset
   ;

set : 'set' ident expression expression expression expression ;

deleteFace : 'face' ident 'endface' ;


IDENT : VALID_ID_START VALID_ID_CHAR* ;
fragment VALID_ID_START : ('a' .. 'z') | ('A' .. 'Z') | '_' | '.' ;
fragment VALID_ID_CHAR : VALID_ID_START | ('0' .. '9') ;

//The NUMBER part gets its potential sign from "(PLUS | MINUS)* atom" in the expression rule
SCIENTIFIC_NUMBER : NUMBER (E SIGN? UNSIGNED_INTEGER)? ;
fragment NUMBER : ('0' .. '9') + ('.' ('0' .. '9') +)? ;
fragment UNSIGNED_INTEGER : ('0' .. '9')+ ;
fragment E : 'E' | 'e' ;
fragment SIGN : ('+' | '-') ;

LPAREN : '(' ;
RPAREN : ')' ;
PLUS : '+' ;
MINUS : '-' ;
TIMES : '*' ;
DIV : '/' ;
GT : '>' ;
LT : '<' ;
EQ : '=' ;
POINT : '.' ;
POW : '^' ;
WS : [ \r\n\t] + -> skip ;

COMMENT : '(*' .*? '*)' -> skip ;
LINE_COMMENT : '#' ~('\n'|'\r')* -> skip ;
