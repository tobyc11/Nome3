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

Nome::AKeyword* NewKeyword(Nome::CNomeDriver* driver, const char* string)
{
    auto* astNode = Nome::AKeyword::Create(*driver->GetASTContext(), string,
        driver->OffsetToLocation(driver->tokenBegin), driver->OffsetToLocation(driver->tokenEnd));
    return astNode;
}

Nome::AIdent* NewIdent(Nome::CNomeDriver* driver, const char* string)
{
    auto* astNode = Nome::AIdent::Create(*driver->GetASTContext(), string,
        driver->OffsetToLocation(driver->tokenBegin), driver->OffsetToLocation(driver->tokenEnd));
    return astNode;
}

Nome::ANumber* NewNumber(Nome::CNomeDriver* driver, const char* string)
{
    auto* astNode = Nome::ANumber::Create(*driver->GetASTContext(), string,
        driver->OffsetToLocation(driver->tokenBegin), driver->OffsetToLocation(driver->tokenEnd));
    return astNode;
}

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
expr                        { yylval->Keyword = NewKeyword(driver, yytext); return EXPR; }
sin                         { yylval->Keyword = NewKeyword(driver, yytext); return SIN; }
cos                         { yylval->Keyword = NewKeyword(driver, yytext); return COS; }
tan                         { yylval->Keyword = NewKeyword(driver, yytext); return TAN; }
cot                         { yylval->Keyword = NewKeyword(driver, yytext); return COT; }
sec                         { yylval->Keyword = NewKeyword(driver, yytext); return SEC; }
csc                         { yylval->Keyword = NewKeyword(driver, yytext); return CSC; }
arcsin                      { yylval->Keyword = NewKeyword(driver, yytext); return ARCSIN; }
arccos                      { yylval->Keyword = NewKeyword(driver, yytext); return ARCCOS; }
arctan                      { yylval->Keyword = NewKeyword(driver, yytext); return ARCTAN; }
arccot                      { yylval->Keyword = NewKeyword(driver, yytext); return ARCCOT; }
arcsec                      { yylval->Keyword = NewKeyword(driver, yytext); return ARCSEC; }
arccsc                      { yylval->Keyword = NewKeyword(driver, yytext); return ARCCSC; }
point                       { yylval->Keyword = NewKeyword(driver, yytext); return POINT; }
endpoint                    { yylval->Keyword = NewKeyword(driver, yytext); return ENDPOINT; }
polyline                    { yylval->Keyword = NewKeyword(driver, yytext); return POLYLINE; }
endpolyline                 { yylval->Keyword = NewKeyword(driver, yytext); return ENDPOLYLINE; }
closed                      { yylval->Keyword = NewKeyword(driver, yytext); return CLOSED; }
face                        { yylval->Keyword = NewKeyword(driver, yytext); return FACE; }
endface                     { yylval->Keyword = NewKeyword(driver, yytext); return ENDFACE; }
object                      { yylval->Keyword = NewKeyword(driver, yytext); return OBJECT; }
endobject                   { yylval->Keyword = NewKeyword(driver, yytext); return ENDOBJECT; }
mesh                        { yylval->Keyword = NewKeyword(driver, yytext); return MESH; }
endmesh                     { yylval->Keyword = NewKeyword(driver, yytext); return ENDMESH; }
group                       { yylval->Keyword = NewKeyword(driver, yytext); return GROUP; }
endgroup                    { yylval->Keyword = NewKeyword(driver, yytext); return ENDGROUP; }
circle                      { yylval->Keyword = NewKeyword(driver, yytext); return CIRCLE; }
endcircle                   { yylval->Keyword = NewKeyword(driver, yytext); return ENDCIRCLE; }
funnel                      { yylval->Keyword = NewKeyword(driver, yytext); return FUNNEL; }
endfunnel                   { yylval->Keyword = NewKeyword(driver, yytext); return ENDFUNNEL; }
tunnel                      { yylval->Keyword = NewKeyword(driver, yytext); return TUNNEL; }
endtunnel                   { yylval->Keyword = NewKeyword(driver, yytext); return ENDTUNNEL; }
beziercurve                 { yylval->Keyword = NewKeyword(driver, yytext); return BEZIERCURVE; }
endbeziercurve              { yylval->Keyword = NewKeyword(driver, yytext); return ENDBEZIERCURVE; }
slices                      { yylval->Keyword = NewKeyword(driver, yytext); return SLICES; }
bspline                     { yylval->Keyword = NewKeyword(driver, yytext); return BSPLINE; }
endbspline                  { yylval->Keyword = NewKeyword(driver, yytext); return ENDBSPLINE; }
order                       { yylval->Keyword = NewKeyword(driver, yytext); return ORDER; }
instance                    { yylval->Keyword = NewKeyword(driver, yytext); return INSTANCE; }
endinstance                 { yylval->Keyword = NewKeyword(driver, yytext); return ENDINSTANCE; }
rotate                      { yylval->Keyword = NewKeyword(driver, yytext); return ROTATE; }
scale                       { yylval->Keyword = NewKeyword(driver, yytext); return SCALE; }
translate                   { yylval->Keyword = NewKeyword(driver, yytext); return TRANSLATE; }
surface                     { yylval->Keyword = NewKeyword(driver, yytext); return SURFACE; }
endsurface                  { yylval->Keyword = NewKeyword(driver, yytext); return ENDSURFACE; }
color                       { yylval->Keyword = NewKeyword(driver, yytext); return COLOR; } 
background                  { yylval->Keyword = NewKeyword(driver, yytext); return BACKGROUND; }
endbackground               { yylval->Keyword = NewKeyword(driver, yytext); return ENDBACKGROUND; }
foreground                  { yylval->Keyword = NewKeyword(driver, yytext); return FOREGROUND; }
endforeground               { yylval->Keyword = NewKeyword(driver, yytext); return ENDFOREGROUND; }
insidefaces                 { yylval->Keyword = NewKeyword(driver, yytext); return INSIDEFACES; }
endinsidefaces              { yylval->Keyword = NewKeyword(driver, yytext); return ENDINSIDEFACES; }
outsidefaces                { yylval->Keyword = NewKeyword(driver, yytext); return OUTSIDEFACES; }
endoutsidefaces             { yylval->Keyword = NewKeyword(driver, yytext); return ENDOUTSIDEFACES; }
offsetfaces                 { yylval->Keyword = NewKeyword(driver, yytext); return OFFSETFACES; }
endoffsetfaces              { yylval->Keyword = NewKeyword(driver, yytext); return ENDOFFSETFACES; }
frontfaces                  { yylval->Keyword = NewKeyword(driver, yytext); return FRONTFACES; }
endfrontfaces               { yylval->Keyword = NewKeyword(driver, yytext); return ENDFRONTFACES; }
backfaces                   { yylval->Keyword = NewKeyword(driver, yytext); return BACKFACES; }
endbackfaces                { yylval->Keyword = NewKeyword(driver, yytext); return ENDBACKFACES; }
rimfaces                    { yylval->Keyword = NewKeyword(driver, yytext); return RIMFACES; }
endrimfaces                 { yylval->Keyword = NewKeyword(driver, yytext); return ENDRIMFACES; }
bank                        { yylval->Keyword = NewKeyword(driver, yytext); return BANK; }
endbank                     { yylval->Keyword = NewKeyword(driver, yytext); return ENDBANK; }
set                         { yylval->Keyword = NewKeyword(driver, yytext); return SET; }
delete                      { yylval->Keyword = NewKeyword(driver, yytext); return DELETE; }
enddelete                   { yylval->Keyword = NewKeyword(driver, yytext); return ENDDELETE; }
subdivision                 { yylval->Keyword = NewKeyword(driver, yytext); return SUBDIVISION; }
endsubdivision              { yylval->Keyword = NewKeyword(driver, yytext); return ENDSUBDIVISION; }
type                        { yylval->Keyword = NewKeyword(driver, yytext); return TYPE; }
subdivisions                { yylval->Keyword = NewKeyword(driver, yytext); return SUBDIVISIONS; }
offset                      { yylval->Keyword = NewKeyword(driver, yytext); return OFFSET; }
endoffset                   { yylval->Keyword = NewKeyword(driver, yytext); return ENDOFFSET; }
min                         { yylval->Keyword = NewKeyword(driver, yytext); return MIN; }
max                         { yylval->Keyword = NewKeyword(driver, yytext); return MAX; }
step                        { yylval->Keyword = NewKeyword(driver, yytext); return STEP; }
\$[a-zA-Z\.][a-zA-Z0-9_\.]* { yylval->Ident = NewIdent(driver, yytext); return IDENT; }
[a-zA-Z\.][a-zA-Z0-9_\.]*   { yylval->Ident = NewIdent(driver, yytext); return IDENT; }
-?([0-9]*[.])?[0-9]+        { yylval->Expr = NewNumber(driver, yytext); return NUMBER; }
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
