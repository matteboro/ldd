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

void write_ldd_to_file(LddManager* ldd, LddNode* f, const char *filename, bool verbose){
    FILE *outfile; 
    outfile = fopen(filename,"w");
    if (verbose)
      Ldd_DumpDotVerbose(ldd, f,  outfile);
    else
      Ldd_DumpDot(ldd, f,  outfile);
    fclose (outfile); 
}


void compare_ldd_and_split(LddManager* ldd, LddNode* f, lincons_t cons, bool verbose) {

  const char fn_starter[100] = "./split_compare_output/before.dot";
  const char fn_split_pos[100] = "./split_compare_output/pos_split.dot";
  const char fn_split_neg[100] = "./split_compare_output/neg_split.dot";
  const char fn_and_pos[100] = "./split_compare_output/pos_and.dot";
  const char fn_and_neg[100] = "./split_compare_output/neg_and.dot";
  
  write_ldd_to_file(ldd, f, fn_starter, verbose);

  // MY_SPLIT_IMPL

  LddNode** nodes = Ldd_SplitBoxTheory(ldd, f, cons);

  write_ldd_to_file(ldd, nodes[0], fn_split_pos, verbose);
  write_ldd_to_file(ldd, nodes[1], fn_split_neg, verbose);

  Cudd_Ref(nodes[0]);
  Cudd_Ref(nodes[1]);

  // LDD_AND_SPLIT

  LddNode* cons_node = Ldd_FromCons(ldd, cons);
  Ldd_Ref(cons_node);

  LddNode* pos_ldd = Ldd_And(ldd, f, cons_node);
  LddNode* neg_ldd = Ldd_And(ldd, f, Cudd_Not(cons_node));

  Cudd_Ref(pos_ldd);
  Cudd_Ref(neg_ldd);

  write_ldd_to_file(ldd, pos_ldd, fn_and_pos, verbose);
  write_ldd_to_file(ldd, neg_ldd, fn_and_neg, verbose);

  if (nodes[0] == pos_ldd)
    fprintf(stdout, "test passed on pos\n");
  else
    fprintf(stdout, "test failed on pos\n");

  if (nodes[1] == neg_ldd)
    fprintf(stdout, "test passed on neg\n");
  else
    fprintf(stdout, "test failed on neg\n");


  free(nodes);
}

#define False 0
#define True 1

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

  // LddNode *x1_leq_5 = Ldd_FromCons(ldd, CONS(x1,4,5));
  // Ldd_Ref(x1_leq_5);

  // LddNode *x1_leq_0 = Ldd_FromCons(ldd, CONS(x1,4,0));
  // Ldd_Ref(x1_leq_0);

  // LddNode *x1_leq_10 = Ldd_FromCons(ldd, CONS(x1,4,10));
  // Ldd_Ref(x1_leq_10);


  // LddNode *x2_leq_4 = Ldd_FromCons (ldd, CONS (x2,4,4));
  // Ldd_Ref(x2_leq_4);

  // LddNode *x2_leq_6 = Ldd_FromCons (ldd, CONS (x2,4,6));
  // Ldd_Ref(x2_leq_4);

  // LddNode *x2_leq_8 = Ldd_FromCons (ldd, CONS (x2,4,8));
  // Ldd_Ref(x2_leq_4);

  // LddNode *x2_leq_2 = Ldd_FromCons (ldd, CONS (x2,4,2));
  // Ldd_Ref(x2_leq_4);


  // LddNode *tmp1 = Ldd_And(ldd, Cudd_Not(x2_leq_2), x2_leq_4);
  // Ldd_Ref(tmp1);

  // LddNode *tmp2 = Ldd_And(ldd, Cudd_Not(x2_leq_4), x2_leq_6);
  // Ldd_Ref(tmp2);

  // LddNode *tmp3 = Ldd_And(ldd, Cudd_Not(x2_leq_6), x2_leq_8);
  // Ldd_Ref(tmp3);


  // LddNode *tmp4 = Ldd_And(ldd, x1_leq_0, tmp1);
  // Ldd_Ref(tmp4);


  // LddNode *tmp5 = Ldd_And(ldd, Cudd_Not(x1_leq_0), x1_leq_5);
  // Ldd_Ref(tmp5);

  // LddNode *tmp6 = Ldd_And(ldd, tmp5, tmp2);
  // Ldd_Ref(tmp6);


  // LddNode *tmp7= Ldd_And(ldd, Cudd_Not(x1_leq_10), tmp3);
  // Ldd_Ref(tmp7);


  // LddNode *tmp8= Ldd_Or(ldd, tmp4, tmp6);
  // Ldd_Ref(tmp8);


  // r1 = Ldd_Or(ldd, tmp8, tmp7);
  // Ldd_Ref(r1);

  // // Ldd_IsOrderedAscendingByVariable(ldd, r1) ? fprintf(stdout, "True\n") : fprintf(stdout, "False\n");

  // fprintf(stdout, "test on x1 <= -3: \n");
  // compare_ldd_and_split(ldd, r1, CONS(x1,4,-3), False);
  // fprintf(stdout, "test on x1 <= 2: \n");
  // compare_ldd_and_split(ldd, r1, CONS(x1,4, 2), False);
  // fprintf(stdout, "test on x1 <= 7: \n");
  // compare_ldd_and_split(ldd, r1, CONS(x1,4, 7), False);
  // fprintf(stdout, "test on x1 <= 12: \n");
  // compare_ldd_and_split(ldd, r1, CONS(x1,4,12), False);


  LddNode *x1_ge_5 = Cudd_Not(Ldd_FromCons(ldd, CONS(x1,4,5)));
  Ldd_Ref(x1_ge_5);

  LddNode *x2_leq_5 = Ldd_FromCons(ldd, CONS(x2,4,5));
  Ldd_Ref(x2_leq_5);

  LddNode *x3_leq_10 = Ldd_FromCons(ldd, CONS(x3,4,10));
  Ldd_Ref(x3_leq_10);

  LddNode *tmp1 = Ldd_And(ldd, x1_ge_5, x2_leq_5);
  Ldd_Ref(tmp1);

  LddNode *tmp2 = Ldd_And(ldd, tmp1, x3_leq_10);
  Ldd_Ref(tmp2);

  fprintf(stdout, "test on x3 <= 10: \n");
  compare_ldd_and_split(ldd, tmp2, CONS(x3,4,10), False);

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
