/* tabsym.h */

enum DruhId {Nedef, IdProm, IdKonst, IdPole};

void deklKonst(char*, int);
void deklProm(char*);
void deklPole(char*, int, int);
bool isArr(char*);
int adrArr(char*);
int adrProm(char*);
DruhId idPromKonst(char*, int*);

