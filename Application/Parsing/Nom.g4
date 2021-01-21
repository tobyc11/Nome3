grammar Nom;

file : generalizedCommand* EOF;

/* Basic data */
expression
   :  ident LPAREN expression RPAREN # Call
   |  expression op=POW expression # BinOp
   |  expression op=(TIMES | DIV)  expression # BinOp
   |  expression op=(PLUS | MINUS) expression # BinOp
   |  LPAREN expression RPAREN # SubExpParen
   |  (PLUS | MINUS) expression # UnaryOp
   |  beg=LBRACE sec=KW_EXPR expression end=RBRACE # SubExpCurly
   |  atom # AtomExpr
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
   | DOLLAR IDENT
   ;

vector
   /* This is so that the parser tries the 4 expression version before 3 and 2 */
   : LPAREN expression expression expression expression RPAREN
   | LPAREN expression expression expression RPAREN
   | LPAREN expression* RPAREN ;

/* Arguments and commands */
argClosed : KW_CLOSED ;
argHidden : KW_HIDDEN ;
argSurface : KW_SURFACE ident ;
argSlices : KW_SLICES expression ;
argOrder : KW_ORDER expression ;
argTransform
   : KW_ROTATE LPAREN exp1=expression exp2=expression exp3=expression RPAREN LPAREN exp4=expression RPAREN # argTransformTwo
   | KW_SCALE LPAREN expression expression expression RPAREN # argTransformOne
   | KW_TRANSLATE LPAREN expression expression expression RPAREN # argTransformOne
   ;
argColor : KW_COLOR LPAREN expression expression expression RPAREN ;


positionalArg
   : expression
   | vector
   ;

namedArg
   : argClosed
   | argHidden
   | argSurface
   | argSlices
   | argOrder
   | argTransform
   | argColor
   ;

anyArg : positionalArg | namedArg ;

setCommand : KW_SET ident expression expression expression expression ;

generalizedCommand : (KW_CMD | KW_SURFACE) anyArg* generalizedCommand* setCommand* KW_END_CMD ;


/* subdivision and offset currently misparse, but we aren't using them anyway */
fragment VALID_CMDS
   : 'point' | 'polyline' | 'sweep' | 'sweepcontrol' | 'face' | 'object' | 'mesh' | 'group' | 'circle'
   | 'funnel' | 'tunnel' | 'torusknot' | 'torus' | 'beziercurve' | 'bspline' | 'instance' | 'background' | 'foreground'
   | 'insidefaces' | 'outsidefaces' | 'offsetfaces' | 'frontfaces' | 'backfaces' | 'rimfaces' | 'bank'
   | 'delete' | 'subdivision' | 'offset'
   ;

KW_CMD : VALID_CMDS;
KW_END_CMD : 'end' VALID_CMDS | 'end' KW_SURFACE ;
KW_SET : 'set' ;

/* keywords used in arguments */
KW_CLOSED : 'closed' ;
KW_HIDDEN : 'hidden' ;
KW_SURFACE : 'surface' ;
KW_SLICES : 'slices' ;
KW_ORDER : 'order' ;
KW_ROTATE : 'rotate' ;
KW_SCALE : 'scale' ;
KW_TRANSLATE : 'translate' ;
KW_COLOR : 'color' ;

/* other keywords */
KW_EXPR : 'expr' ;

IDENT : VALID_ID_START VALID_ID_CHAR* ;
fragment VALID_ID_START : ('a' .. 'z') | ('A' .. 'Z') | '_' | '.' ;
fragment VALID_ID_CHAR : VALID_ID_START | ('0' .. '9') ;

//The NUMBER part gets its potential sign from "(PLUS | MINUS)* atom" in the expression rule
SCIENTIFIC_NUMBER : NUMBER (E SIGN? UNSIGNED_INTEGER)? ;
fragment NUMBER : ('0' .. '9') + ('.' ('0' .. '9') +)? ;
fragment UNSIGNED_INTEGER : ('0' .. '9')+ ;
fragment E : 'E' | 'e' ;
fragment SIGN : ('+' | '-') ;

LBRACE : '{' ;
RBRACE : '}' ;
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
DOLLAR : '$' ;
WS : [ \r\n\t] + -> skip ;

COMMENT : '(*' .*? '*)' -> skip ;
LINE_COMMENT : '#' ~('\n'|'\r')* -> skip ;
