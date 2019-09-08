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
expr                        { LEXER_IDENT; return EXPR; }
sin                         { LEXER_IDENT; return SIN; }
cos                         { LEXER_IDENT; return COS; }
tan                         { LEXER_IDENT; return TAN; }
cot                         { LEXER_IDENT; return COT; }
sec                         { LEXER_IDENT; return SEC; }
csc                         { LEXER_IDENT; return CSC; }
arcsin                      { LEXER_IDENT; return ARCSIN; }
arccos                      { LEXER_IDENT; return ARCCOS; }
arctan                      { LEXER_IDENT; return ARCTAN; }
arccot                      { LEXER_IDENT; return ARCCOT; }
arcsec                      { LEXER_IDENT; return ARCSEC; }
arccsc                      { LEXER_IDENT; return ARCCSC; }
point                       { LEXER_IDENT; return POINT; }
endpoint                    { LEXER_IDENT; return ENDPOINT; }
polyline                    { LEXER_IDENT; return POLYLINE; }
endpolyline                 { LEXER_IDENT; return ENDPOLYLINE; }
closed                      { LEXER_IDENT; return CLOSED; }
face                        { LEXER_IDENT; return FACE; }
endface                     { LEXER_IDENT; return ENDFACE; }
object                      { LEXER_IDENT; return OBJECT; }
endobject                   { LEXER_IDENT; return ENDOBJECT; }
mesh                        { LEXER_IDENT; return MESH; }
endmesh                     { LEXER_IDENT; return ENDMESH; }
group                       { LEXER_IDENT; return GROUP; }
endgroup                    { LEXER_IDENT; return ENDGROUP; }
circle                      { LEXER_IDENT; return CIRCLE; }
endcircle                   { LEXER_IDENT; return ENDCIRCLE; }
funnel                      { LEXER_IDENT; return FUNNEL; }
endfunnel                   { LEXER_IDENT; return ENDFUNNEL; }
tunnel                      { LEXER_IDENT; return TUNNEL; }
endtunnel                   { LEXER_IDENT; return ENDTUNNEL; }
beziercurve                 { LEXER_IDENT; return BEZIERCURVE; }
endbeziercurve              { LEXER_IDENT; return ENDBEZIERCURVE; }
slices                      { LEXER_IDENT; return SLICES; }
bspline[0-9]*               { LEXER_IDENT; return BSPLINE; }
endbspline[0-9]*            { LEXER_IDENT; return ENDBSPLINE; }
order                       { LEXER_IDENT; return ORDER; }
instance                    { LEXER_IDENT; return INSTANCE; }
endinstance                 { LEXER_IDENT; return ENDINSTANCE; }
rotate                      { LEXER_IDENT; return ROTATE; }
scale                       { LEXER_IDENT; return SCALE; }
translate                   { LEXER_IDENT; return TRANSLATE; }
surface                     { LEXER_IDENT; return SURFACE; }
endsurface                  { LEXER_IDENT; return ENDSURFACE; }
color                       { LEXER_IDENT; return COLOR; } 
background                  { LEXER_IDENT; return BACKGROUND; }
endbackground               { LEXER_IDENT; return ENDBACKGROUND; }
foreground                  { LEXER_IDENT; return FOREGROUND; }
endforeground               { LEXER_IDENT; return ENDFOREGROUND; }
insidefaces                 { LEXER_IDENT; return INSIDEFACES; }
endinsidefaces              { LEXER_IDENT; return ENDINSIDEFACES; }
outsidefaces                { LEXER_IDENT; return OUTSIDEFACES; }
endoutsidefaces             { LEXER_IDENT; return ENDOUTSIDEFACES; }
offsetfaces                 { LEXER_IDENT; return OFFSETFACES; }
endoffsetfaces              { LEXER_IDENT; return ENDOFFSETFACES; }
frontfaces                  { LEXER_IDENT; return FRONTFACES; }
endfrontfaces               { LEXER_IDENT; return ENDFRONTFACES; }
backfaces                   { LEXER_IDENT; return BACKFACES; }
endbackfaces                { LEXER_IDENT; return ENDBACKFACES; }
rimfaces                    { LEXER_IDENT; return RIMFACES; }
endrimfaces                 { LEXER_IDENT; return ENDRIMFACES; }
bank                        { LEXER_IDENT; return BANK; }
endbank                     { LEXER_IDENT; return ENDBANK; }
set                         { LEXER_IDENT; return SET; }
delete                      { LEXER_IDENT; return DELETE; }
enddelete                   { LEXER_IDENT; return ENDDELETE; }
subdivision                 { LEXER_IDENT; return SUBDIVISION; }
endsubdivision              { LEXER_IDENT; return ENDSUBDIVISION; }
type                        { LEXER_IDENT; return TYPE; }
subdivisions                { LEXER_IDENT; return SUBDIVISIONS; }
offset                      { LEXER_IDENT; return OFFSET; }
endoffset                   { LEXER_IDENT; return ENDOFFSET; }
min                         { LEXER_IDENT; return MIN; }
max                         { LEXER_IDENT; return MAX; }
step                        { LEXER_IDENT; return STEP; }

\$[a-zA-Z\.][a-zA-Z0-9_\.]* { LEXER_IDENT; return IDENT; }
[a-zA-Z\.][a-zA-Z0-9_\.]*   { LEXER_IDENT; return IDENT; }
-?([0-9]*[.])?[0-9]+        { LEXER_NUMBER; return NUMBER; }
[ \t\r\n]+                    ;
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
