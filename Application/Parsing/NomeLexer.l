%{
#include <Parsing/NomeDriver.h>

//Parser bridging
#include "NomeParser.hpp"

#define YY_DECL int yylex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, yyscan_t yyscanner, Nome::CNomeDriver* driver)
#define YY_USER_ACTION { \
    yylloc->first_line = yylineno; \
    yylloc->last_line = yylineno; \
    yylloc->first_column = yycolumn; \
    yylloc->last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng; \
    driver->tokenBegin = driver->currOffset; driver->tokenEnd = driver->currOffset + yyleng - 1; driver->currOffset += yyleng; }
	
#define LEXER_KEYWORD \
    yylval->Keyword = Nome::AKeyword::Create(*driver->GetASTContext(), yytext, \
        driver->OffsetToLocation(driver->tokenBegin, yylloc->first_line, yylloc->first_column), \
        driver->OffsetToLocation(driver->tokenEnd, yylloc->last_line, yylloc->last_column));

#define LEXER_IDENT \
    yylval->Ident = Nome::AIdent::Create(*driver->GetASTContext(), yytext, \
        driver->OffsetToLocation(driver->tokenBegin, yylloc->first_line, yylloc->first_column), \
        driver->OffsetToLocation(driver->tokenEnd, yylloc->last_line, yylloc->last_column));

#define LEXER_NUMBER \
    yylval->Expr = Nome::ANumber::Create(*driver->GetASTContext(), yytext, \
        driver->OffsetToLocation(driver->tokenBegin, yylloc->first_line, yylloc->first_column), \
        driver->OffsetToLocation(driver->tokenEnd, yylloc->last_line, yylloc->last_column));

int isatty(int fd) { return 0; }
%}

%option reentrant bison-bridge bison-locations
%option nounistd
%option noyywrap
%option yylineno

%x MULTI_COMMENT
%x LINE_COMMENT

ID [a-z][a-zA-Z0-9]*

%%
<INITIAL>{
    "(*"                    BEGIN(MULTI_COMMENT);
    "#"                     BEGIN(LINE_COMMENT);
}
<MULTI_COMMENT>{
    "*)"                    BEGIN(INITIAL);
    .                       ;
    \n                      ;
}
<LINE_COMMENT>{
    \n                      BEGIN(INITIAL);
    .                       ;
}
expr                        { LEXER_KEYWORD; return EXPR; }
sin                         { LEXER_KEYWORD; return SIN; }
cos                         { LEXER_KEYWORD; return COS; }
tan                         { LEXER_KEYWORD; return TAN; }
cot                         { LEXER_KEYWORD; return COT; }
sec                         { LEXER_KEYWORD; return SEC; }
csc                         { LEXER_KEYWORD; return CSC; }
arcsin                      { LEXER_KEYWORD; return ARCSIN; }
arccos                      { LEXER_KEYWORD; return ARCCOS; }
arctan                      { LEXER_KEYWORD; return ARCTAN; }
arccot                      { LEXER_KEYWORD; return ARCCOT; }
arcsec                      { LEXER_KEYWORD; return ARCSEC; }
arccsc                      { LEXER_KEYWORD; return ARCCSC; }
point                       { LEXER_KEYWORD; return POINT; }
endpoint                    { LEXER_KEYWORD; return ENDPOINT; }
polyline                    { LEXER_KEYWORD; return POLYLINE; }
endpolyline                 { LEXER_KEYWORD; return ENDPOLYLINE; }
closed                      { LEXER_KEYWORD; return CLOSED; }
face                        { LEXER_KEYWORD; return FACE; }
endface                     { LEXER_KEYWORD; return ENDFACE; }
object                      { LEXER_KEYWORD; return OBJECT; }
endobject                   { LEXER_KEYWORD; return ENDOBJECT; }
mesh                        { LEXER_KEYWORD; return MESH; }
endmesh                     { LEXER_KEYWORD; return ENDMESH; }
group                       { LEXER_KEYWORD; return GROUP; }
endgroup                    { LEXER_KEYWORD; return ENDGROUP; }
circle                      { LEXER_KEYWORD; return CIRCLE; }
endcircle                   { LEXER_KEYWORD; return ENDCIRCLE; }
funnel                      { LEXER_KEYWORD; return FUNNEL; }
endfunnel                   { LEXER_KEYWORD; return ENDFUNNEL; }
tunnel                      { LEXER_KEYWORD; return TUNNEL; }
endtunnel                   { LEXER_KEYWORD; return ENDTUNNEL; }
beziercurve                 { LEXER_KEYWORD; return BEZIERCURVE; }
endbeziercurve              { LEXER_KEYWORD; return ENDBEZIERCURVE; }
slices                      { LEXER_KEYWORD; return SLICES; }
bspline                     { LEXER_KEYWORD; return BSPLINE; }
endbspline                  { LEXER_KEYWORD; return ENDBSPLINE; }
order                       { LEXER_KEYWORD; return ORDER; }
instance                    { LEXER_KEYWORD; return INSTANCE; }
endinstance                 { LEXER_KEYWORD; return ENDINSTANCE; }
rotate                      { LEXER_KEYWORD; return ROTATE; }
scale                       { LEXER_KEYWORD; return SCALE; }
translate                   { LEXER_KEYWORD; return TRANSLATE; }
surface                     { LEXER_KEYWORD; return SURFACE; }
endsurface                  { LEXER_KEYWORD; return ENDSURFACE; }
color                       { LEXER_KEYWORD; return COLOR; } 
background                  { LEXER_KEYWORD; return BACKGROUND; }
endbackground               { LEXER_KEYWORD; return ENDBACKGROUND; }
foreground                  { LEXER_KEYWORD; return FOREGROUND; }
endforeground               { LEXER_KEYWORD; return ENDFOREGROUND; }
insidefaces                 { LEXER_KEYWORD; return INSIDEFACES; }
endinsidefaces              { LEXER_KEYWORD; return ENDINSIDEFACES; }
outsidefaces                { LEXER_KEYWORD; return OUTSIDEFACES; }
endoutsidefaces             { LEXER_KEYWORD; return ENDOUTSIDEFACES; }
offsetfaces                 { LEXER_KEYWORD; return OFFSETFACES; }
endoffsetfaces              { LEXER_KEYWORD; return ENDOFFSETFACES; }
frontfaces                  { LEXER_KEYWORD; return FRONTFACES; }
endfrontfaces               { LEXER_KEYWORD; return ENDFRONTFACES; }
backfaces                   { LEXER_KEYWORD; return BACKFACES; }
endbackfaces                { LEXER_KEYWORD; return ENDBACKFACES; }
rimfaces                    { LEXER_KEYWORD; return RIMFACES; }
endrimfaces                 { LEXER_KEYWORD; return ENDRIMFACES; }
bank                        { LEXER_KEYWORD; return BANK; }
endbank                     { LEXER_KEYWORD; return ENDBANK; }
set                         { LEXER_KEYWORD; return SET; }
delete                      { LEXER_KEYWORD; return DELETE; }
enddelete                   { LEXER_KEYWORD; return ENDDELETE; }
subdivision                 { LEXER_KEYWORD; return SUBDIVISION; }
endsubdivision              { LEXER_KEYWORD; return ENDSUBDIVISION; }
type                        { LEXER_KEYWORD; return TYPE; }
subdivisions                { LEXER_KEYWORD; return SUBDIVISIONS; }
offset                      { LEXER_KEYWORD; return OFFSET; }
endoffset                   { LEXER_KEYWORD; return ENDOFFSET; }
min                         { LEXER_KEYWORD; return MIN; }
max                         { LEXER_KEYWORD; return MAX; }
step                        { LEXER_KEYWORD; return STEP; }
\$[a-zA-Z\.][a-zA-Z0-9_\.]* { LEXER_IDENT; return IDENT; }
[a-zA-Z\.][a-zA-Z0-9_\.]*   { LEXER_IDENT; return IDENT; }
-?([0-9]*[.])?[0-9]+        { LEXER_NUMBER; return NUMBER; }
[ \t\n]+                    ;
.                           { return yytext[0]; }
%%

namespace Nome
{

void CNomeDriver::ParseToAST()
{
    yyscan_t scanner;
    yylex_init(&scanner);
    YY_BUFFER_STATE buffer;
    if (SourceMgr && SourceFile)
    {
        std::string inputString = SourceFile->GetAsString();
        buffer = yy_scan_bytes(inputString.data(), (int)inputString.size(), scanner);
    }
    else
    {
        buffer = yy_scan_bytes(SourceString.data(), (int)SourceString.size(), scanner);
    }
    yyset_lineno(1, scanner);
    yyset_column(1, scanner);
    InvokeParser(scanner);
    yy_delete_buffer(buffer, scanner);
    yylex_destroy(scanner);
}

}
