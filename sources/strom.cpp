/* strom.cpp */

#include "strom.h"
#include "tabsym.h"
#include <stdio.h>

// konstruktory a destruktory

Var::Var(int a, bool rv)
{ addr = a; rvalue = rv; }

Numb::Numb(int v)
{ value = v; }

int Numb::Value()
{ return value; }

Bop::Bop(Operator o, Expr *l, Expr *r)
{ op = o; left = l; right = r; }

Bop::~Bop()
{ delete left; delete right; }

UnMinus::UnMinus(Expr *e)
{ expr = e; }

UnMinus::~UnMinus()
{ delete expr; }

Assign::Assign(Var *v, Expr *e)
{ var = v; expr = e; }

Assign::~Assign()
{ delete var; delete expr; }

ArrAssign::ArrAssign(Arr* a, Expr* e)
{
   array = a;
   expr  = e;
}

ArrAssign::~ArrAssign()
{ 
   delete array;
   delete expr;
}

Write::Write(Expr *e)
{ expr = e; }

Write::~Write()
{ delete expr; }

Read::Read(int a)
{ addr = a; }

Read::~Read()
{ }

If::If(Expr *c, Statm *ts, Statm *es)
{ cond = c; thenstm = ts; elsestm = es; }

If::~If()
{ delete cond; delete thenstm; delete elsestm; }

While::While(Expr *c, Statm *b)
{ cond = c; body = b; }

While::~While()
{ delete body; }

ForTo::ForTo(Expr* s, Expr* l, Statm* s_a, Statm* b)
{
   start = s;
   limit = l;
   start_assign = s_a;
   body = b;
}

ForTo::~ForTo()
{
   delete start;
   delete limit;   
   delete start_assign;
   delete body;
}

ForDownto::ForDownto(Expr* s, Expr* l, Statm* s_a, Statm* b)
{
   start = s;
   limit = l;
   start_assign = s_a;
   body = b;
}

ForDownto::~ForDownto()
{
   delete start;
   delete limit;
   delete start_assign;
   delete body;
}

Arr::Arr(int a, Expr* i, bool r)
{
   addr = a;
   idx = i;
   rvalue = r;
}

Arr::~Arr()
{
   delete idx;
}

StatmList::StatmList(Statm *s, StatmList *n)
{ statm = s; next = n; }

StatmList::~StatmList()
{ delete statm; delete next; }

Prog::Prog(StatmList *s)
{ stm = s; }

Prog::~Prog()
{ delete stm; } 

// definice metody Optimize

Node *Bop::Optimize() 
{
   Numb *l = dynamic_cast<Numb*>(left->Optimize());
   Numb *r = dynamic_cast<Numb*>(right->Optimize());
   if (!l || !r) return this;
   int res;
   int leftval = l->Value();
   int rightval = r->Value();
   switch (op) {
   case Plus:
      res = leftval + rightval;
      break;
   case Minus:
      res = leftval - rightval;
      break;
   case Times:
      res = leftval * rightval;
      break;
   case Divide:
      res = leftval / rightval;
      break;
   case Eq:
      res = leftval == rightval;
      break;
   case NotEq:
      res = leftval != rightval;
      break;
   case Less:
      res = leftval < rightval;
      break;
   case Greater:
      res = leftval > rightval;
      break;
   case LessOrEq:
      res = leftval <= rightval;
      break;
   case GreaterOrEq:
      res = leftval >= rightval;
      break;
   case Error://cannot happen
      break;
   }
   delete this;
   return new Numb(res);
}

Node *UnMinus::Optimize()
{
   expr->Optimize();
   Numb *e = dynamic_cast<Numb*>(expr);
   if (!e) return this;
   e = new Numb(-e->Value());
   delete this;
   return e;
}

Node *Assign::Optimize()
{
   // (Var *) when it is Expr* ? 
   expr = (Var*)(expr->Optimize());
   return this;
}

Node* ArrAssign::Optimize()
{
   array = (Arr *)(array->Optimize());
   expr = (Expr *)(expr->Optimize());   
   return this;
}

Node *Write::Optimize()
{
   expr = (Expr*)(expr->Optimize());
   return this;
}

Node *Read::Optimize()
{
   return this;
}

Node *If::Optimize()
{
   cond = (Expr*)(cond->Optimize());
   thenstm = (Statm*)(thenstm->Optimize());
   elsestm = (Statm*)(elsestm->Optimize());
   Numb *c = dynamic_cast<Numb*>(cond);
   if (!c) return this;
   Node *res;
   if (c->Value()) {
      res = thenstm; thenstm = 0;
   } else {
      res = elsestm; elsestm = 0;
   }
   delete this;
   return res;
}

Node *While::Optimize()
{
   cond = (Expr*)(cond->Optimize());
   body = (Statm*)(body->Optimize());
   Numb *c = dynamic_cast<Numb*>(cond);
   if (!c) return this;
   if (!c->Value()) {
      delete this;
      return new Empty;
   }
   return this;
}

Node *ForTo::Optimize()
{ 
   body = (Statm *)(body->Optimize());
   return this;
}

Node *ForDownto::Optimize()
{ 
   body = (Statm *)(body->Optimize());
   return this;
}

Node* Arr::Optimize()
{
   idx = (Expr *)(idx->Optimize());
   return this;
}

Node *StatmList::Optimize()
{
   StatmList *s = this;
   do {
      s->statm = (Statm*)(s->statm->Optimize());
      s = s->next;
   }
   while (s);
   return this;
}

Node *Prog::Optimize()
{
   stm = (StatmList*)(stm->Optimize());
   return this;
}

// definice metody Translate

void Var::Translate()
{
   Gener(TA, addr);
   if (rvalue)
      Gener(DR);
}

void Numb::Translate()
{
   Gener(TC, value);
}

void Bop::Translate()
{
   left->Translate();
   right->Translate();
   Gener(BOP, op);
}

void UnMinus::Translate()
{
   expr->Translate();
   Gener(UNM);
}

void Assign::Translate()
{
   var->Translate();
   expr->Translate();
   Gener(ST);
}

void ArrAssign::Translate()
{
   array->Translate();
   expr->Translate();
   Gener(ST);
}

void Write::Translate()
{
   expr->Translate();
   Gener(WRT);
}

void Read::Translate()
{  
   Gener(RD, addr);
}

void If::Translate()
{
   cond->Translate();
   int a1 = Gener(IFJ);
   thenstm->Translate();
   if (elsestm) {
      int a2 = Gener(JU);
      PutIC(a1);
      elsestm->Translate();
      PutIC(a2);
   } else 
      PutIC(a1);
}

void While::Translate()
{
   int a1 = GetIC();
   cond->Translate();
   int a2 = Gener(IFJ);
   body->Translate();
   Gener(JU, a1);
   PutIC(a2);
}

void ForTo::Translate()
{
   start_assign->Translate();
   
   int a1 = GetIC();
   start->Translate();
   Gener(DR);
   limit->Translate();
   Gener(BOP, LessOrEq);
   int a2 = Gener(IFJ);
   body->Translate();
   
   /* Increment control_val */
   start->Translate();
   start->Translate();
   Gener(DR);
   Gener(TC, 1);
   Gener(BOP, Plus);
   Gener(ST);

   Gener(JU, a1);
   PutIC(a2);
}

void ForDownto::Translate()
{
   start_assign->Translate();
   
   int a1 = GetIC();
   start->Translate();
   Gener(DR);
   limit->Translate();
   Gener(BOP, GreaterOrEq);
   int a2 = Gener(IFJ);
   body->Translate();
   
   /* Increment control_val */
   start->Translate();
   start->Translate();
   Gener(DR);
   Gener(TC, 1);
   Gener(BOP, Minus);
   Gener(ST);

   Gener(JU, a1);
   PutIC(a2);
}

void Arr::Translate()
{
   Gener(TA, addr);
   idx->Translate();
   Gener(BOP, Plus);
   if (rvalue)
      Gener(DR);
}

void StatmList::Translate()
{
   StatmList *s = this;
   do {
      s->statm->Translate();
      s = s->next;
   } while (s);
}

void Prog::Translate()
{
   stm->Translate();
   Gener(STOP);
}

Expr *VarOrConst(char *id)
{
   int v;
   DruhId druh = idPromKonst(id,&v);
   switch (druh) {
   case IdProm:
      return new Var(v, true);
   case IdKonst:
      return new Numb(v);
   default:
      return 0;
   }
}
