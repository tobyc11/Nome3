%code requires
{
#include <Parsing/NomeDriver.h>

//Lexer interface
typedef void* yyscan_t;
}

%code
{
//Lexer interface
extern int yylex(YYSTYPE * yylval_param, YYLTYPE * yylloc_param, yyscan_t yyscanner, Nome::CNomeDriver* driver);

void yyerror(YYLTYPE* loc, yyscan_t scanner, Nome::CNomeDriver* driver, const char* msg)
{
    printf("%s %d:%d\n", msg, loc->first_line, loc->first_column);
    //TODO: notify driver
}
}

%define api.pure full
%define parse.error verbose
%locations
%lex-param {yyscan_t scanner} {Nome::CNomeDriver* driver}
%parse-param {yyscan_t scanner} {Nome::CNomeDriver* driver}
%union {
    Nome::AExpr* Expr;
    Nome::AIdent* Ident;
    Nome::ACommand* Command;
    Nome::ATransform* Transform;
	Nome::AExprList* ExprList;
	Nome::ACommandArgument* CommandArgument;
}
%token <Ident> IDENT
%token <Expr> NUMBER
%left '-' '+'
%left '*' '/'
%precedence NEG
%right '^'
%token <Ident> EXPR SIN COS TAN COT SEC CSC ARCSIN ARCCOS ARCTAN ARCCOT ARCSEC ARCCSC
%token <Ident> POINT ENDPOINT
%token <Ident> POLYLINE ENDPOLYLINE CLOSED
%token <Ident> FACE ENDFACE
%token <Ident> OBJECT ENDOBJECT
%token <Ident> MESH ENDMESH
%token <Ident> GROUP ENDGROUP
%token <Ident> CIRCLE ENDCIRCLE
%token <Ident> FUNNEL ENDFUNNEL
%token <Ident> TUNNEL ENDTUNNEL
%token <Ident> BEZIERCURVE ENDBEZIERCURVE SLICES
%token <Ident> BSPLINE ENDBSPLINE ORDER
%token <Ident> INSTANCE ENDINSTANCE ROTATE SCALE TRANSLATE
%token <Ident> SURFACE ENDSURFACE COLOR
%token <Ident> BACKGROUND ENDBACKGROUND
%token <Ident> FOREGROUND ENDFOREGROUND
%token <Ident> INSIDEFACES ENDINSIDEFACES
%token <Ident> OUTSIDEFACES ENDOUTSIDEFACES
%token <Ident> OFFSETFACES ENDOFFSETFACES
%token <Ident> FRONTFACES ENDFRONTFACES
%token <Ident> BACKFACES ENDBACKFACES
%token <Ident> RIMFACES ENDRIMFACES
%token <Ident> BANK ENDBANK SET
%token <Ident> DELETE ENDDELETE
%token <Ident> SUBDIVISION ENDSUBDIVISION TYPE SUBDIVISIONS
%token <Ident> OFFSET ENDOFFSET MIN MAX STEP
%type <Command> command command_list
%type <CommandArgument> command_arg_list
%type <ExprList> plain_exp_list
%type <Expr> exp num_exp
%type <Command> point polyline face object mesh group circle funnel tunnel bezier_curve bspline instance surface background foreground insidefaces outsidefaces offsetfaces frontfaces backfaces rimfaces bank set delete face_for_deletion subdivision offset
%type <Transform> transform
%type <Ident> kwd_or_ident

%%
input:
  %empty
| input command { driver->GetASTContext()->AppendCommand($2); }
| num_exp { driver->GetASTContext()->SetExpr($1); }
;

kwd_or_ident: IDENT | POINT | POLYLINE | FACE | OBJECT | MESH | GROUP | CIRCLE | FUNNEL | TUNNEL | BEZIERCURVE | BSPLINE | INSTANCE;

command: point | polyline | face | object | mesh | group |circle | funnel | tunnel | bezier_curve | bspline |
  instance | surface | background | foreground | insidefaces | outsidefaces | offsetfaces |
  frontfaces | backfaces | rimfaces | bank | delete | subdivision | offset
| '(' kwd_or_ident IDENT command_arg_list command_list ')' {
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $3, $2, nullptr, $4, $5);
}
;

command_list:
  %empty { $$ = nullptr; }
| command command_list { $$ = $1; $$->SetNext($2); }
;

command_arg_list:
  %empty { $$ = nullptr; }
| IDENT '=' exp command_arg_list { $$ = Nome::ACommandArgument::Create(*driver->GetASTContext(), $1, $3, $4); }
;

plain_exp_list:
  %empty { $$ = Nome::AExprList::Create(*driver->GetASTContext()); }
| plain_exp_list exp { $$->AddExpr($2); }
;

exp:
  NUMBER               { $$ = $1; }
| IDENT                { $$ = $1; }
| exp '+' exp          { $$ = Nome::ABinaryOp::Create(*driver->GetASTContext(), Nome::ABinaryOp::BOP_ADD, $1, $3); }
| exp '-' exp          { $$ = Nome::ABinaryOp::Create(*driver->GetASTContext(), Nome::ABinaryOp::BOP_SUB, $1, $3); }
| exp '*' exp          { $$ = Nome::ABinaryOp::Create(*driver->GetASTContext(), Nome::ABinaryOp::BOP_MUL, $1, $3); }
| exp '/' exp          { $$ = Nome::ABinaryOp::Create(*driver->GetASTContext(), Nome::ABinaryOp::BOP_DIV, $1, $3); }
| '-' exp %prec NEG    { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_NEG, $2, nullptr); }
| exp '^' exp          { $$ = Nome::ABinaryOp::Create(*driver->GetASTContext(), Nome::ABinaryOp::BOP_EXP, $1, $3); }
| '(' exp ')'          { $$ = $2; }
| SIN '(' exp ')'      { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_SIN, $3, nullptr); }
| COS '(' exp ')'      { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_COS, $3, nullptr); }
| TAN '(' exp ')'      { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_TAN, $3, nullptr); }
| COT '(' exp ')'      { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_COT, $3, nullptr); }
| SEC '(' exp ')'      { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_SEC, $3, nullptr); }
| CSC '(' exp ')'      { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_CSC, $3, nullptr); }
| ARCSIN '(' exp ')'   { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_ARCSIN, $3, nullptr); }
| ARCCOS '(' exp ')'   { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_ARCCOS, $3, nullptr); }
| ARCTAN '(' exp ')'   { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_ARCTAN, $3, nullptr); }
| ARCCOT '(' exp ')'   { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_ARCCOT, $3, nullptr); }
| ARCSEC '(' exp ')'   { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_ARCSEC, $3, nullptr); }
| ARCCSC '(' exp ')'   { $$ = Nome::AUnaryOp::Create(*driver->GetASTContext(), Nome::AUnaryOp::UOP_ARCCSC, $3, nullptr); }
| '[' plain_exp_list ']' { $$ = $2; }
;

num_exp: NUMBER | '{' EXPR exp '}' { $$ = $3; };

ident_list: IDENT { driver->IdentList.push_back($1); } | ident_list IDENT { driver->IdentList.push_back($2); };

point: POINT IDENT '(' num_exp num_exp num_exp ')' ENDPOINT
{
	auto* exprList = Nome::AExprList::Create(*driver->GetASTContext());
	exprList->AddExpr($4);
	exprList->AddExpr($5);
	exprList->AddExpr($6);
	auto* arg = Nome::ACommandArgument::Create(*driver->GetASTContext(), driver->MakeIdent("position"), exprList, nullptr);
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $POINT, $ENDPOINT, arg);
};

polyline: POLYLINE IDENT '(' ident_list ')' polyline_ext ENDPOLYLINE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $POLYLINE, $ENDPOLYLINE, nullptr);
    driver->Ext.MoveTo(*driver, $$);
	driver->MoveIdentList($$, "point_list");
};

polyline_ext: %empty | CLOSED { driver->Ext.NamedArgs[$1] = driver->MakeIdent("true"); };

face: FACE IDENT '(' ident_list ')' face_ext ENDFACE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $FACE, $ENDFACE);
    driver->Ext.MoveTo(*driver, $$);
	driver->MoveIdentList($$, "point_list");
};

face_ext: %empty | SURFACE IDENT { driver->Ext.NamedArgs[$1] = $2; };

object: OBJECT IDENT '(' ident_list ')' ENDOBJECT
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $OBJECT, $ENDOBJECT);
	driver->MoveIdentList($$, "primitives");
};

mesh: MESH IDENT face_list ENDMESH
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $MESH, $ENDMESH);
	driver->MoveFaceList($$);
};

face_list: %empty | face_list face { driver->FaceList.push_back($2); };

group: GROUP IDENT instance_list ENDGROUP
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $GROUP, $ENDGROUP);
	driver->MoveInstanceList($$);
};

instance_list: %empty | instance_list instance { driver->InstanceList.push_back($2); };

circle: CIRCLE IDENT '(' num_exp num_exp ')' ENDCIRCLE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $CIRCLE, $ENDCIRCLE);
	$$->AddArgument(driver->MakeIdent("subdiv"), $4);
	$$->AddArgument(driver->MakeIdent("radius"), $5);
};

funnel: FUNNEL IDENT '(' num_exp num_exp num_exp num_exp ')' ENDFUNNEL
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $FUNNEL, $ENDFUNNEL);
	$$->AddArgument(driver->MakeIdent("subdiv"), $4);
	$$->AddArgument(driver->MakeIdent("radius"), $5);
	$$->AddArgument(driver->MakeIdent("ratio"), $6);
	$$->AddArgument(driver->MakeIdent("height"), $7);
};

tunnel: TUNNEL IDENT '(' num_exp num_exp num_exp num_exp ')' ENDTUNNEL
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $TUNNEL, $ENDTUNNEL);
	$$->AddArgument(driver->MakeIdent("subdiv"), $4);
	$$->AddArgument(driver->MakeIdent("radius"), $5);
	$$->AddArgument(driver->MakeIdent("ratio"), $6);
	$$->AddArgument(driver->MakeIdent("height"), $7);
};

bezier_curve: BEZIERCURVE IDENT '(' ident_list ')' bezier_curve_ext ENDBEZIERCURVE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $BEZIERCURVE, $ENDBEZIERCURVE);
    driver->Ext.MoveTo(*driver, $$);
	driver->MoveIdentList($$, "point_list");
};

bezier_curve_ext: %empty | SLICES num_exp { driver->Ext.NamedArgs[$1] = $2; };

bspline: BSPLINE IDENT '(' ident_list ')' bspline_ext ENDBSPLINE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $BSPLINE, $ENDBSPLINE);
    driver->Ext.MoveTo(*driver, $$);
	driver->MoveIdentList($$, "point_list");
};

bspline_ext: %empty
 | bspline_ext CLOSED { driver->Ext.NamedArgs[$2] = $2; }
 | bspline_ext SLICES num_exp { driver->Ext.NamedArgs[$2] = $3; }
 | bspline_ext ORDER num_exp { driver->Ext.NamedArgs[$2] = $3; }
 ;

instance: INSTANCE IDENT IDENT instance_ext ENDINSTANCE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $INSTANCE, $ENDINSTANCE);
	$$->AddArgument(driver->MakeIdent("target"), $3);
	driver->MoveTransformList($$, "transformation");
    driver->Ext.MoveTo(*driver, $$);
};

instance_ext: %empty
 | instance_ext SURFACE IDENT { driver->Ext.NamedArgs[$2] = $3; }
 | instance_ext transform { driver->TransformList.push_back($2); };

transform:
  ROTATE '(' num_exp num_exp num_exp ')' '(' num_exp ')'
  {
      $$ = Nome::ATransform::Create(*driver->GetASTContext());
      $$->Type = Nome::ATransform::TF_ROTATE;
      $$->AxisX = $3; $$->AxisY = $4; $$->AxisZ = $5;
      $$->Deg = $8;
  }
| SCALE '(' num_exp num_exp num_exp ')'
  {
      $$ = Nome::ATransform::Create(*driver->GetASTContext());
      $$->Type = Nome::ATransform::TF_SCALE;
      $$->AxisX = $3; $$->AxisY = $4; $$->AxisZ = $5;
  }
| TRANSLATE '(' num_exp num_exp num_exp ')'
  {
      $$ = Nome::ATransform::Create(*driver->GetASTContext());
      $$->Type = Nome::ATransform::TF_TRANSLATE;
      $$->AxisX = $3; $$->AxisY = $4; $$->AxisZ = $5;
  }
;

surface: SURFACE IDENT COLOR '(' num_exp num_exp num_exp ')' ENDSURFACE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $SURFACE, $ENDSURFACE);
	auto* exprList = Nome::AExprList::Create(*driver->GetASTContext());
	exprList->AddExpr($5);
	exprList->AddExpr($6);
	exprList->AddExpr($7);
    $$->AddArgument(driver->MakeIdent("color"), exprList);
};

background: BACKGROUND SURFACE IDENT ENDBACKGROUND
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->AddArgument($SURFACE, $IDENT);
};

foreground: FOREGROUND SURFACE IDENT ENDFOREGROUND
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->AddArgument($SURFACE, $IDENT);
};

insidefaces: INSIDEFACES SURFACE IDENT ENDINSIDEFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->AddArgument($SURFACE, $IDENT);
};

outsidefaces: OUTSIDEFACES SURFACE IDENT ENDOUTSIDEFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->AddArgument($SURFACE, $IDENT);
};

offsetfaces: OFFSETFACES SURFACE IDENT ENDOFFSETFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->AddArgument($SURFACE, $IDENT);
};

frontfaces: FRONTFACES SURFACE IDENT ENDFRONTFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->AddArgument($SURFACE, $IDENT);
};

backfaces: BACKFACES SURFACE IDENT ENDBACKFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->AddArgument($SURFACE, $IDENT);
};

rimfaces: RIMFACES SURFACE IDENT ENDRIMFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->AddArgument($SURFACE, $IDENT);
};

bank: BANK IDENT set_list ENDBANK
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $BANK, $ENDBANK);
	driver->MoveSetList($$);
};

set_list: %empty | set_list set { driver->SetList.push_back($2); };

set: SET IDENT num_exp num_exp num_exp num_exp
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $SET, nullptr);
	$$->AddArgument(driver->MakeIdent("value"), $3);
	$$->AddArgument(driver->MakeIdent("min"), $4);
	$$->AddArgument(driver->MakeIdent("max"), $5);
	$$->AddArgument(driver->MakeIdent("step"), $6);
};

delete: DELETE face_for_deletion_list ENDDELETE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $DELETE, $ENDDELETE);
	driver->MoveFaceForDeletionList($$);
};

face_for_deletion: FACE IDENT ENDFACE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $FACE, $ENDFACE);
};

face_for_deletion_list: %empty | face_for_deletion_list face_for_deletion { driver->FaceForDeletionList.push_back($2); };

subdivision: SUBDIVISION IDENT TYPE IDENT SUBDIVISIONS num_exp ENDSUBDIVISION
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $SUBDIVISION, $ENDSUBDIVISION);
    //TODO: Incomplete
};

offset: OFFSET IDENT TYPE IDENT MIN num_exp MAX num_exp STEP num_exp ENDOFFSET
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $OFFSET, $ENDOFFSET);
    //TODO: Incomplete
};

%%

namespace Nome
{

void CNomeDriver::InvokeParser(void* scanner)
{
    yyparse(scanner, this);
}

}
