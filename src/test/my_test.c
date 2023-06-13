#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "cudd.h"
#include "util.h"
#include "ldd.h"
#include "tvpi.h"

LddManager* ldd;

#define T(c,n) t->create_linterm ((c),(n))
#define C(n) t->create_int_cst(n)
#define CONS(tm,n,c) t->create_cons (T(tm,n),0,C(c))

void and_accum (LddNode **r, LddNode *n)
{
  /* compute: r <- r && n */
  LddNode *tmp;

  tmp = Ldd_And (ldd, *r, n);
  Ldd_Ref (tmp);

  Ldd_RecursiveDeref (ldd, *r); 
  *r = tmp;
}

void test0() {
  DdManager *cudd;

  theory_t * t;

  // constant_t i5, i10, i15;
  // int cf1[3] = {1, -1, 0};
  // int cf2[3] = {0, 1, -1};
  // int cf3[3] = {1, 0, -1};
  
  // linterm_t t1, t2, t3;
  // lincons_t l1, l2, l3;

  // LddNode *d1, *d2, *d3, *d4, *d5, *d6, *d7;

  printf ("Creating the world...\n");
  cudd = Cudd_Init (0, 0, CUDD_UNIQUE_SLOTS, 127, 0);
  t = tvpi_create_theory (5);

  ldd = Ldd_Init (cudd, t);

  int x1[4] = {0,1,0,0};
  int x2[4] = {0,0,1,0};
  int x3[4] = {0,0,0,1};

  LddNode *d[5];
  LddNode *r1, *r2; //*r3, *r4;

  int i = 0;

  fprintf (stdout, "\n\nTEST 1\n");
  
  // /* 0: x1 <= -1 */
  // d[i] = Ldd_FromCons (ldd, CONS (x1,4,-1));
  // Ldd_Ref (d[i++]);

  // /* 1: x1 <= 1 */
  // d[i] = Ldd_FromCons (ldd, CONS (x1,4,1));
  // Ldd_Ref (d[i++]);

  // /* 2: x2 <= 0 */
  // d[i] = Ldd_FromCons (ldd, CONS (x2,4,0));
  // Ldd_Ref (d[i++]);

  // /* 3: x3 <= 0 */
  // d[i] = Ldd_FromCons (ldd, CONS (x3,4,0));
  // Ldd_Ref (d[i++]);

  // /* 4: x1 <= 4 */
  // d[i] = Ldd_FromCons (ldd, CONS (x1,4,4));
  // Ldd_Ref (d[i++]);

  /* 4: x1 <= 4 */

  LddNode *x1_leq_4 = Ldd_FromCons (ldd, CONS (x1,4,4));
  Ldd_Ref(x1_leq_4);

  Ldd_DumpDotVerbose(ldd, x1_leq_4, stdout);

  // MY_SPLIT_IMPL

  fprintf(stdout, "my split implementation:\n\n");

  LddNode** nodes2 = Ldd_SplitBoxTheory(ldd, x1_leq_4, CONS(x1,4,1));

  Ldd_DumpDotVerbose(ldd, nodes2[0], stdout);
  Ldd_DumpDotVerbose(ldd, nodes2[1], stdout);

  free(nodes2);

  // LDD_AND_SPLIT

  fprintf(stdout, "\nldd and implementation:\n\n");

  LddNode* cons_node = Ldd_FromCons(ldd, CONS(x1,4,1));

  LddNode* pos_ldd = Ldd_And(ldd, x1_leq_4, cons_node);
  LddNode* neg_ldd = Ldd_And(ldd, x1_leq_4, Ldd_Not(cons_node));

  Ldd_DumpDotVerbose(ldd, pos_ldd, stdout);
  Ldd_DumpDotVerbose(ldd, neg_ldd, stdout);

/*


*/

/*
  DdNode **ddnodearray = (DdNode**)malloc(sizeof(DdNode*)); // initialize the function array
  ddnodearray[0] = r2;
  Cudd_DumpDot(cudd, 1, ddnodearray, NULL, NULL, stdout); // dump the function to .dot file
  printf ("\n");
  printf ("\n");


  r1 = Ldd_And (ldd, d[0], d[2]);
  Ldd_Ref(r1);

  printf ("d[0] in SMTLIB:\n");
  Ldd_DumpSmtLibV1 (ldd, d[0], NULL, "d[0]", stdout);
  printf ("\n");
  printf ("\n");

  DdNode **ddnodearray = (DdNode**)malloc(sizeof(DdNode*)); // initialize the function array
  ddnodearray[0] = d[0];
  Cudd_DumpDot(cudd, 1, ddnodearray, NULL, NULL, stdout); // dump the function to .dot file
  printf ("\n");
  printf ("\n");

  printf ("r1 in SMTLIB:\n");
  Ldd_DumpSmtLibV1 (ldd, r1, NULL, "r1", stdout);
  printf ("\n");
  printf ("\n");

  ddnodearray[0] = r1;
  Cudd_DumpDot(cudd, 1, ddnodearray, NULL, NULL, stdout); // dump the function to .dot file
  printf ("\n");
  printf ("\n");

  r2 = Ldd_And (ldd, Ldd_Not(d[1]), d[4]);
  Ldd_Ref(r2);

  printf ("r2 in SMTLIB:\n");
  Ldd_DumpSmtLibV1 (ldd, r2, NULL, "r2", stdout);
  printf ("\n");
  printf ("\n");

  ddnodearray[0] = r2;
  Cudd_DumpDot(cudd, 1, ddnodearray, NULL, NULL, stdout); // dump the function to .dot file
  printf ("\n");
  printf ("\n");
*/

/*
  r1 = Ldd_And (ldd, Cudd_Not(d[0]), d[1]);
  Ldd_Ref(r1);

  and_accum (&r1, Cudd_Not(d[2]));
  and_accum (&r1, Cudd_Not (d[3]));

  r2 = Ldd_And (ldd, Cudd_Not (d[1]), d[4]);
  Ldd_Ref (r2);
  
  and_accum (&r2, Cudd_Not (d[2]));
  and_accum (&r2, d[3]);

  Ldd_PrintMinterm (ldd, r1);
  Ldd_PrintMinterm (ldd, r2);

  r3 = Ldd_Or (ldd, r1, r2);
  Ldd_Ref (r3);

  printf ("r3\n");
  Ldd_PrintMinterm (ldd, r3);

  r4 = Ldd_TermMinmaxApprox (ldd, r3);
  Ldd_Ref (r4);

  printf ("r4\n");
  Ldd_PrintMinterm (ldd, r4);
*/
}

void write_dd (DdManager *gbm, DdNode *dd, char* filename)
{
    FILE *outfile; // output file pointer for .dot file
    outfile = fopen(filename,"w");
    DdNode **ddnodearray = (DdNode**)malloc(sizeof(DdNode*)); // initialize the function array
    ddnodearray[0] = dd;
    Cudd_DumpDot(gbm, 1, ddnodearray, NULL, NULL, outfile); // dump the function to .dot file
    free(ddnodearray);
    fclose (outfile); // close the file */
}

void test1(){
  DdManager *gbm; /* Global BDD manager. */
  char filename[60] = "/home/matteo/github/ldd/indagine/ite.dot";
  char filename_then[60] = "/home/matteo/github/ldd/indagine/then.dot";
  char filename_else[60] = "/home/matteo/github/ldd/indagine/else.dot";
  char filename_and[60] = "/home/matteo/github/ldd/indagine/and.dot";
  gbm = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0); /* Initialize a new BDD manager. */

  Cudd_ReduceHeap(gbm, CUDD_REORDER_NONE, 0);

  // Create variables
  DdNode *x1 = Cudd_bddNewVar(gbm);
  DdNode *x2 = Cudd_bddNewVar(gbm);
  DdNode *x3 = Cudd_bddNewVar(gbm);
  DdNode *x4 = Cudd_bddNewVar(gbm);
  DdNode *x5 = Cudd_bddNewVar(gbm);

  DdNode *intr1, *intr2, *bdd, *and;

  intr1 = Cudd_bddAnd(gbm, x1, x2);
  Cudd_Ref(intr1);
  intr1 = Cudd_Not(intr1);
  write_dd (gbm, intr1, filename_then);

  intr2 = Cudd_bddAnd(gbm, x3, x4);
  Cudd_Ref(intr1);
  write_dd (gbm, intr2, filename_else);

  // BDD result = ite(x5, !(x1 && x2), x3 && x4);
  bdd = Cudd_bddIte(gbm, x5, intr1, intr2);

  // BDD result = !(x1 && x2) && (x3 && x4);
  and = Cudd_bddAnd(gbm, intr1, intr2); 

  Cudd_RecursiveDeref(gbm, intr1);
  Cudd_RecursiveDeref(gbm, intr2); 

  // bdd = Cudd_BddToAdd(gbm, bdd);
  write_dd (gbm, bdd, filename);
  write_dd (gbm, and, filename_and);

  Cudd_Quit(gbm);
}

int main(int argc, char** argv) {

  test0();
}
