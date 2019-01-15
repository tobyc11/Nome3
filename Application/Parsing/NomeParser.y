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
    Nome::AKeyword* Keyword;
    Nome::AExpr* Expr;
    Nome::AIdent* Ident;
    Nome::ACommand* Command;
    Nome::ATransform* Transform;
}
%token <Ident> IDENT
%token <Expr> NUMBER
%left '-' '+'
%left '*' '/'
%precedence NEG
%right '^'
%token <Keyword> EXPR SIN COS TAN COT SEC CSC ARCSIN ARCCOS ARCTAN ARCCOT ARCSEC ARCCSC
%token <Keyword> POINT ENDPOINT
%token <Keyword> POLYLINE ENDPOLYLINE CLOSED
%token <Keyword> FACE ENDFACE
%token <Keyword> OBJECT ENDOBJECT
%token <Keyword> MESH ENDMESH
%token <Keyword> GROUP ENDGROUP
%token <Keyword> CIRCLE ENDCIRCLE
%token <Keyword> FUNNEL ENDFUNNEL
%token <Keyword> TUNNEL ENDTUNNEL
%token <Keyword> BEZIERCURVE ENDBEZIERCURVE SLICES
%token <Keyword> BSPLINE ENDBSPLINE ORDER
%token <Keyword> INSTANCE ENDINSTANCE ROTATE SCALE TRANSLATE
%token <Keyword> SURFACE ENDSURFACE COLOR
%token <Keyword> BACKGROUND ENDBACKGROUND
%token <Keyword> FOREGROUND ENDFOREGROUND
%token <Keyword> INSIDEFACES ENDINSIDEFACES
%token <Keyword> OUTSIDEFACES ENDOUTSIDEFACES
%token <Keyword> OFFSETFACES ENDOFFSETFACES
%token <Keyword> FRONTFACES ENDFRONTFACES
%token <Keyword> BACKFACES ENDBACKFACES
%token <Keyword> RIMFACES ENDRIMFACES
%token <Keyword> BANK ENDBANK SET
%token <Keyword> DELETE ENDDELETE
%token <Keyword> SUBDIVISION ENDSUBDIVISION TYPE SUBDIVISIONS
%token <Keyword> OFFSET ENDOFFSET MIN MAX STEP
%type <Command> command
%type <Expr> exp num_exp
%type <Command> point polyline face object mesh group circle funnel tunnel bezier_curve bspline instance surface background foreground insidefaces outsidefaces offsetfaces frontfaces backfaces rimfaces bank set delete subdivision offset
%type <Transform> transform

%%
input:
  %empty
| input command { driver->GetASTContext()->AppendCommand($2); }
| num_exp { driver->GetASTContext()->SetExpr($1); }
;

command: point | polyline | face | object | mesh | group |
  circle | funnel | tunnel | bezier_curve | bspline | instance |
  surface | background | foreground | insidefaces | outsidefaces | offsetfaces |
  frontfaces | backfaces | rimfaces |
  bank | delete | subdivision | offset
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
;

num_exp: NUMBER | '{' EXPR exp '}' { $$ = $3; };

ident_list: IDENT { driver->IdentList.push_back($1); } | ident_list IDENT { driver->IdentList.push_back($2); };

point: POINT IDENT '(' num_exp num_exp num_exp ')' ENDPOINT
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $POINT, $ENDPOINT);
    $$->Args.push_back($4);
    $$->Args.push_back($5);
    $$->Args.push_back($6);
};

polyline: POLYLINE IDENT '(' ident_list ')' polyline_ext ENDPOLYLINE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $POLYLINE, $ENDPOLYLINE);
    $$->Args.insert($$->Args.begin(), driver->IdentList.begin(), driver->IdentList.end());
    driver->IdentList.clear();
    driver->Ext.MoveTo($$);
};

polyline_ext: %empty | CLOSED { driver->Ext.NamedArgs[$1] = nullptr; };

face: FACE IDENT '(' ident_list ')' face_ext ENDFACE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $FACE, $ENDFACE);
    $$->Args.insert($$->Args.begin(), driver->IdentList.begin(), driver->IdentList.end());
    driver->IdentList.clear();
};

face_ext: %empty | SURFACE IDENT {};

object: OBJECT IDENT '(' ident_list ')' ENDOBJECT
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $OBJECT, $ENDOBJECT);
    $$->Args.insert($$->Args.begin(), driver->IdentList.begin(), driver->IdentList.end());
    driver->IdentList.clear();
};

mesh: MESH IDENT face_list ENDMESH
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $MESH, $ENDMESH);
    $$->SubCommands.insert($$->SubCommands.begin(), driver->FaceList.begin(), driver->FaceList.end());
    driver->FaceList.clear();
};

face_list: %empty | face_list face { driver->FaceList.push_back($2); };

group: GROUP IDENT instance_list ENDGROUP
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $GROUP, $ENDGROUP);
    $$->SubCommands.insert($$->SubCommands.begin(), driver->InstanceList.begin(), driver->InstanceList.end());
    driver->InstanceList.clear();
};

instance_list: %empty | instance_list instance { driver->InstanceList.push_back($2); };

circle: CIRCLE IDENT '(' num_exp num_exp ')' ENDCIRCLE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $CIRCLE, $ENDCIRCLE);
    $$->Args.push_back($4);
    $$->Args.push_back($5);
};

funnel: FUNNEL IDENT '(' num_exp num_exp num_exp num_exp ')' ENDFUNNEL
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $FUNNEL, $ENDFUNNEL);
    $$->Args.push_back($4);
    $$->Args.push_back($5);
    $$->Args.push_back($6);
    $$->Args.push_back($7);
};

tunnel: TUNNEL IDENT '(' num_exp num_exp num_exp num_exp ')' ENDTUNNEL
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $TUNNEL, $ENDTUNNEL);
    $$->Args.push_back($4);
    $$->Args.push_back($5);
    $$->Args.push_back($6);
    $$->Args.push_back($7);
};

bezier_curve: BEZIERCURVE IDENT '(' ident_list ')' SLICES num_exp ENDBEZIERCURVE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $BEZIERCURVE, $ENDBEZIERCURVE);
    $$->Args.insert($$->Args.begin(), driver->IdentList.begin(), driver->IdentList.end());
    driver->IdentList.clear();
    //TODO: dont ignore slices
};

bspline: BSPLINE IDENT '(' ident_list ')' CLOSED SLICES num_exp ENDBSPLINE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $BSPLINE, $ENDBSPLINE);
    $$->Args.insert($$->Args.begin(), driver->IdentList.begin(), driver->IdentList.end());
    driver->IdentList.clear();
    //TODO: dont ignore slices/closed
};

instance: INSTANCE IDENT IDENT instance_ext ENDINSTANCE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $2, $INSTANCE, $ENDINSTANCE);
    $$->Args.push_back($3);
    driver->Ext.MoveTo($$);
};

instance_ext: %empty | instance_ext SURFACE IDENT | instance_ext transform { driver->Ext.Args.push_back($2); };

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
    $$->Args.push_back($5);
    $$->Args.push_back($6);
    $$->Args.push_back($7);
};

background: BACKGROUND SURFACE IDENT ENDBACKGROUND
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->NamedArgs[$SURFACE] = $IDENT;
};

foreground: FOREGROUND SURFACE IDENT ENDFOREGROUND
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->NamedArgs[$SURFACE] = $IDENT;
};

insidefaces: INSIDEFACES SURFACE IDENT ENDINSIDEFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->NamedArgs[$SURFACE] = $IDENT;
};

outsidefaces: OUTSIDEFACES SURFACE IDENT ENDOUTSIDEFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->NamedArgs[$SURFACE] = $IDENT;
};

offsetfaces: OFFSETFACES SURFACE IDENT ENDOFFSETFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->NamedArgs[$SURFACE] = $IDENT;
};

frontfaces: FRONTFACES SURFACE IDENT ENDFRONTFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->NamedArgs[$SURFACE] = $IDENT;
};

backfaces: BACKFACES SURFACE IDENT ENDBACKFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->NamedArgs[$SURFACE] = $IDENT;
};

rimfaces: RIMFACES SURFACE IDENT ENDRIMFACES
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $1, $4);
    $$->NamedArgs[$SURFACE] = $IDENT;
};

bank: BANK IDENT set_list ENDBANK
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $BANK, $ENDBANK);
    $$->SubCommands.insert($$->SubCommands.begin(), driver->SetList.begin(), driver->SetList.end());
    driver->SetList.clear();
};

set_list: %empty | set_list set { driver->SetList.push_back($2); };

set: SET IDENT num_exp num_exp num_exp num_exp
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), $IDENT, $SET, nullptr);
    $$->Args.push_back($3);
    $$->Args.push_back($4);
    $$->Args.push_back($5);
    $$->Args.push_back($6);
};

delete: DELETE face_list ENDDELETE
{
    $$ = Nome::ACommand::Create(*driver->GetASTContext(), nullptr, $DELETE, $ENDDELETE);
    $$->SubCommands.insert($$->SubCommands.begin(), driver->FaceList.begin(), driver->FaceList.end());
    driver->FaceList.clear();
};

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
