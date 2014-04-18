/* tabsym.cpp */

#include "tabsym.h"
#include <string.h>
#include <stdio.h>

struct PrvekTab {
   char *ident;
   DruhId druh;
   int hodn;
   PrvekTab *dalsi;
   PrvekTab(char *i, DruhId d, int h, PrvekTab *n);
};

PrvekTab::PrvekTab(char *i, DruhId d, int h, PrvekTab *n)
{
   ident = new char[strlen(i)+1];
   strcpy(ident, i);
   druh = d; hodn = h; dalsi = n;
}

static PrvekTab *TabSym = NULL;
static int volna_adr;

static void Chyba(const char *id, const char *txt)
{
   printf("identifikator %s: %s\n", id, txt);
}

PrvekTab *hledejId(char *id)
{
   PrvekTab *p = TabSym;
   while (p) 
      if (strcmp(id,p->ident)==0) 
         return p;
      else
         p = p->dalsi;
   return NULL;
}

void deklKonst(char *id, int val)
{
   PrvekTab *p = hledejId(id);
   if (p) {
      Chyba(id, "druha deklarace");
      return;
   }

   if (TabSym == NULL)
      TabSym = new PrvekTab(id, IdKonst, val, NULL);
   else
   {
      PrvekTab *help = TabSym;
      while (help->dalsi != NULL)
         help = help->dalsi;
      help->dalsi = new PrvekTab(id, IdKonst, val, NULL);
   }
}

void deklProm(char *id)
{
   PrvekTab *p = hledejId(id);
   if (p) {
      Chyba(id, "druha deklarace");
      return;
   }

   if (TabSym == NULL)
      TabSym = new PrvekTab(id, IdProm, volna_adr, NULL);
   else
   {
      PrvekTab *help = TabSym;
      while (help->dalsi != NULL)
         help = help->dalsi;
      help->dalsi = new PrvekTab(id, IdProm, volna_adr, NULL);
   }
   volna_adr++;
}

void deklPole(char* id, int s, int e)
{
   PrvekTab *p = hledejId(id);
   if (p) {
      Chyba(id, "druha deklarace");
      return;
   }

   for (; s < e; ++s)
   {
      if (TabSym == NULL)
         TabSym = new PrvekTab(id, IdPole, volna_adr, NULL);
      else
      {
         PrvekTab *help = TabSym;
         while (help->dalsi != NULL)
            help = help->dalsi;
         help->dalsi = new PrvekTab(id, IdPole, volna_adr, NULL);
      }
      volna_adr++;
   }
}

bool isArr(char* id)
{
   PrvekTab* help = hledejId(id);
   
   if (help == NULL)
   {
      Chyba(id, "neni deklarovan");
      return false;
   }

   if (help->druh == IdPole)
      return true;
   
   return false;
}

int adrArr(char* id)
{
   PrvekTab *p = hledejId(id);
   if (!p) {
      Chyba(id, "neni deklarovan");
      return 0;
   } else if (p->druh != IdPole) {
      Chyba(id, "neni identifikatorem pole");
      return 0;
   } else
      return p->hodn;
}

int adrProm(char *id)
{
   PrvekTab *p = hledejId(id);
   if (!p) {
      Chyba(id, "neni deklarovan");
      return 0;
   } else if (p->druh != IdProm) {
      Chyba(id, "neni identifikatorem promenne");
      return 0;
   } else
      return p->hodn;
}

DruhId idPromKonst(char *id, int *v)
{
   PrvekTab *p = TabSym;
   while (p) 
      if (strcmp(id,p->ident)==0) {
         *v = p->hodn;
         return p->druh;
      } else
         p = p->dalsi;
   Chyba(id, "neni deklarovan");
   return Nedef;
}