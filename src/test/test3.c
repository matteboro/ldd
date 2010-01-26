#include "util.h"
#include "cudd.h"
#include "tdd.h"
#include "tvpi.h"

#include <stdio.h>

/**
 * A structure describing a test case. Each testcase is of the form C1
 * && ... && Cn => C where Ci's and C are constraints.
 */
typedef struct testcase {
  int varnum;
  int consnum;
  int **ante;
  int *cons;
} testcase_t;

DdManager *cudd = NULL;
LddManager* tdd = NULL;
theory_t * theory = NULL;

LddNode *create_cons(testcase_t *tc,int *arg)
{
  constant_t c = theory->create_int_cst(arg[tc->varnum]);
  linterm_t t = theory->create_linterm (arg,tc->varnum);
  lincons_t l = theory->create_cons (t, 0, c);
  LddNode *d = to_tdd (tdd, l);
  Cudd_Ref (d);
  return d;
}

void test(testcase_t *tc)
{
  printf ("Creating the world...\n");
  cudd = Cudd_Init (0, 0, CUDD_UNIQUE_SLOTS, 127, 0);
  theory = tvpi_create_utvpiz_theory (tc->varnum);
  tdd = Ldd_Init (cudd, theory);
  
  //create antecedent
  LddNode *ante = NULL;
  int i;
  for(i = 0;i < tc->consnum;++i) {
    LddNode *d = create_cons(tc,tc->ante[i]);
    ante = (ante == NULL) ? d : Ldd_And(tdd,ante,d);
  }

  //create consequent
  LddNode *cons = create_cons(tc,tc->cons);

  //create negation implication
  LddNode *impl = Ldd_Not(Ldd_Or (tdd, Ldd_Not (ante), cons));

  //existential abstraction
  LddNode *abs = impl;
  for(i = 0;i < tc->varnum;++i) {
    abs = Ldd_ExistAbstract (tdd, abs, i);
  }
  abs = Ldd_Not(abs);

  //check for validity
  assert(abs == Cudd_ReadOne(cudd));

  //cleanup
  printf ("Destroying the world...\n");
  Ldd_Quit (tdd);
  tvpi_destroy_theory (theory);
  Cudd_Quit (cudd);
}

int main(void)
{
  testcase_t tc1;
  tc1.varnum = 3;
  tc1.consnum = 2;
  tc1.ante = (int**)malloc(2 * sizeof(int*));
  int a11[] = {1,-1,0,5};
  int a12[] = {0,1,-1,10};
  tc1.ante[0] = a11;
  tc1.ante[1] = a12;
  int c1[] = {1,0,-1,15};
  tc1.cons = c1;
  test(&tc1);
  return 0;
}
