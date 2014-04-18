/* parser.cpp */

#include "strom.h"
#include "parser.h"
#include "lexan.h"
#include "tabsym.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Prog *Program();
void Dekl();
void DeklKonst();
void ZbDeklKonst();
void DeklProm();
void ZbDeklProm();
void DeklPole(char*);
StatmList *SlozPrikaz();
StatmList *ZbPrikazu();
Statm *Prikaz();
Statm *CastElse();
Expr *Podminka();
Operator RelOp();
Expr *Vyraz();
Expr *ZbVyrazu(Expr*);
Expr *Term();
Expr *ZbTermu(Expr*);
Expr *Faktor();

LexicalSymbol Symb;

void ChybaSrovnani(LexSymbolType s) {
   printf("chyba pri srovnani, ocekava se %s.\n", symbTable[s]);
   exit(1);
}

void ChybaExpanze(const char* neterminal, LexSymbolType s) {
   printf("Chyba pri expanzi neterminalu %s, nedefinovany symbol %s.\n", neterminal, symbTable[s]);
   exit(1);
}

void Srovnani(LexSymbolType s) {
   if (Symb.type == s)
      Symb = readLexem();
   else
      ChybaSrovnani(s);
}

void Srovnani_IDENT(char *id)
{
   if (Symb.type == IDENT) {
      strcpy(id, Symb.ident);
      Symb = readLexem();
   } else
      ChybaSrovnani(IDENT);
}

void Srovnani_NUMB(int *h)
{
   if (Symb.type == NUMB) {
      *h = Symb.number;
      Symb = readLexem();
   } else 
      ChybaSrovnani(NUMB);
}

Prog *Program()
{
   Dekl();
   return new Prog(SlozPrikaz());
}

void Dekl()
{
   switch (Symb.type) {
   case kwVAR:
      DeklProm();
      Dekl();
      break;
   case kwCONST:
      DeklKonst();
      Dekl();
      break;
   default:
      ;
   }
}

void DeklKonst()
{
   char id[MAX_IDENT_LEN];
   int hod;
   Symb = readLexem();
   Srovnani_IDENT(id);
   Srovnani(EQ);
   Srovnani_NUMB(&hod);
   deklKonst(id, hod);
   ZbDeklKonst();
   Srovnani(SEMICOLON);
}

void ZbDeklKonst()
{
   if (Symb.type == COMMA) {
      char id[MAX_IDENT_LEN];
      int hod;
      Symb = readLexem();
      Srovnani_IDENT(id);
      Srovnani(EQ);
      Srovnani_NUMB(&hod);
      deklKonst(id, hod);
      ZbDeklKonst();
   }
}

void DeklPole(char* id)
{
   int start_idx, end_idx;

   Symb = readLexem(); // srovnal sem LSQPAR
   if (Symb.type == NUMB && Symb.number == 0)
      start_idx = 0;
   else
      ChybaSrovnani(NUMB);
   
   for (int i = 0; i < 2; ++i)
   {
      Symb = readLexem();
      if (Symb.type != DOT)
         ChybaSrovnani(DOT);
   }

   Symb = readLexem(); // srovnal sem druhej DOT
   if (Symb.type != NUMB)
      ChybaSrovnani(NUMB);
   end_idx = Symb.number;
   Symb = readLexem(); // srovnal sem NUMB
   Srovnani(RSQPAR); // srovnani ]
   deklPole(id, start_idx, end_idx); // SERIOUS!
}

// DeklProm() and deklProm(char*) ?!? MADNESS ?!?   
void DeklProm()
{
   char id[MAX_IDENT_LEN];
   Symb = readLexem();
   Srovnani_IDENT(id);

   if (Symb.type == LSQPAR)
      DeklPole(id);
   else 
      deklProm(id);

   ZbDeklProm();
   Srovnani(SEMICOLON);
}

void ZbDeklProm()
{
   if (Symb.type == COMMA) 
   {
      char id[MAX_IDENT_LEN];
      
      Symb = readLexem();
      Srovnani_IDENT(id);

      if (Symb.type == LSQPAR)
         DeklPole(id);
      else
         deklProm(id);

      ZbDeklProm();
   }
}

StatmList *SlozPrikaz()
{
   Srovnani(kwBEGIN);
   Statm *p = Prikaz();
   StatmList *su = new StatmList(p, ZbPrikazu());
   Srovnani(kwEND);
   return su;
}

StatmList *ZbPrikazu()
{
   if (Symb.type == SEMICOLON) {
      Symb = readLexem();
      Statm *p = Prikaz();
      return new StatmList(p, ZbPrikazu());
   }
   return 0;
}

Statm *Prikaz()
{
   switch (Symb.type) {
   case IDENT: {
      if (isArr(Symb.ident))
      {
         char id[MAX_IDENT_LEN];

         Srovnani_IDENT(id);
         Srovnani(LSQPAR); // [
         Arr* arr = new Arr(adrArr(id), Vyraz(), false); //tady nechci DR
         Srovnani(RSQPAR); // ]
         Srovnani(ASSIGN); // :=
         return new ArrAssign(arr, Vyraz());
      }

      Var *var = new Var(adrProm(Symb.ident),false);
      Symb = readLexem();
      Srovnani(ASSIGN);
      return new Assign(var, Vyraz());
   }
   case kwWRITE:
      Symb = readLexem();
      return new Write(Vyraz());
   case kwREAD:
      Symb = readLexem();
      char id[MAX_IDENT_LEN];
      Srovnani_IDENT(id);
      return new Read(adrProm(id));
   case kwIF: {
      Symb = readLexem();
      Expr *cond = Podminka();
      Srovnani(kwTHEN);
      Statm *prikaz = Prikaz();
      return new If(cond, prikaz, CastElse());
   }
   case kwWHILE: {
      Expr *cond;
      Symb = readLexem();
      cond = Podminka();
      Srovnani(kwDO);
      return new While(cond, Prikaz());
   }
   case kwBEGIN:
      return SlozPrikaz();

   case kwFOR: {
      Symb = readLexem(); // srovnal sem kwFOR
      Var* control_var = new Var(adrProm(Symb.ident), false);
      Symb = readLexem(); // srovnal jsem jmeno ridici promenny
      Srovnani(ASSIGN); // srovnal sem :=
      Statm* control_var_value = new Assign(control_var, Vyraz());
      switch (Symb.type) 
      { 
         case kwTO:
         {
            Symb = readLexem(); // srovnal sem kwTO
            Expr* limit = Vyraz(); 
            Srovnani(kwDO);
            return new ForTo(control_var, limit, control_var_value, Prikaz());
         }

         case kwDOWNTO:
         {
            Symb = readLexem(); // srovnal sem kwDOWNTO
            Expr* limit = Vyraz(); 
            Srovnani(kwDO);
            return new ForDownto(control_var, limit, control_var_value, Prikaz());
         }

         default:
            ChybaSrovnani(kwTO);
      }
   }
   
   default:
      return new Empty;
   }
}

Statm *CastElse()
{
   if (Symb.type == kwELSE) {
      Symb = readLexem();
      return Prikaz();
   }
   return 0;
}

Expr *Podminka()
{
   Expr *left = Vyraz();
   Operator op = RelOp();
   Expr *right = Vyraz();
   return new Bop(op, left, right);
}

Operator RelOp()
{
   switch (Symb.type) {
   case EQ:
      Symb = readLexem();
      return Eq;
   case NEQ:
      Symb = readLexem();
      return NotEq;
   case LT:
      Symb = readLexem();
      return Less;
   case GT:
      Symb = readLexem();
      return Greater;
   case LTE:
      Symb = readLexem();
      return LessOrEq;
   case GTE:
      Symb = readLexem();
      return GreaterOrEq;
   default:
      ChybaExpanze("RelOp", Symb.type);
      return Error;
   }
}

Expr *Vyraz()
{
   if (Symb.type == MINUS) {
      Symb = readLexem();
      return ZbVyrazu(new UnMinus(Term()));
   }
   return ZbVyrazu(Term());
}

Expr *ZbVyrazu(Expr *du)
{
   switch (Symb.type) {
   case PLUS:
      Symb = readLexem();
      return ZbVyrazu(new Bop(Plus, du, Term()));
   case MINUS:
      Symb = readLexem();
      return ZbVyrazu(new Bop(Minus, du, Term()));
   default:
      return du;
   }
}

Expr *Term()
{
   return ZbTermu(Faktor());
}

Expr *ZbTermu(Expr *du)
{
   switch (Symb.type) {
   case TIMES:
      Symb = readLexem();
      return ZbTermu(new Bop(Times, du, Faktor()));
   case DIVIDE:
      Symb = readLexem();
      return ZbVyrazu(new Bop(Divide, du, Faktor()));
   default:
      return du;
   }
}

Expr *Faktor()
{
   switch (Symb.type) {
   case IDENT:
      char id[MAX_IDENT_LEN];
      Srovnani_IDENT(id);
      if (isArr(id))
      {
         Srovnani(LSQPAR);
         Expr* idx = Vyraz();
         Srovnani(RSQPAR);
         return new Arr(adrArr(id), idx, true); //tady chci DR
      }
      return VarOrConst(id);
   case NUMB:
      int hodn;
      Srovnani_NUMB(&hodn);
      return new Numb(hodn);
   case LPAR: {
      Symb = readLexem();
      Expr *su = Vyraz();
      Srovnani(RPAR);
      return su;
   }
   default:
      ChybaExpanze("Faktor", Symb.type);
      return 0;
   }
}

int initParser(char *fileName) {
  if(!initLexan(fileName)) return 0;
  Symb = readLexem();
  return 1;
}
