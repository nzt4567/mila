/* lexan.h */

typedef enum {
   IDENT, NUMB, PLUS, MINUS, TIMES, DIVIDE, 
   EQ, NEQ, LT, GT, LTE, GTE, LPAR, RPAR, ASSIGN,
   COMMA, SEMICOLON, RSQPAR, LSQPAR, DOT,
   kwVAR, kwCONST, kwBEGIN, kwEND, kwIF, kwTHEN, kwELSE, 
   kwWHILE, kwDO, kwWRITE, kwREAD, kwFOR, kwTO, kwDOWNTO,
   EOI, ERR
} LexSymbolType;

extern const char *symbTable[36];

#define MAX_IDENT_LEN 32

typedef struct LexicalSymbol {
  LexSymbolType type;
  char      ident[MAX_IDENT_LEN];  /* atribut symbolu IDENT */
  int       number;                 /* atribut symbolu NUMB */
} LexicalSymbol;

LexicalSymbol readLexem(void);
int initLexan(char*);
