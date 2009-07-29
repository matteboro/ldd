#include "util.h"
#include "cudd.h"
#include "tdd.h"
#include "tdd-ddd.h"


#include <stdio.h>

DdManager *cudd;
tdd_manager *tdd;
theory_t *t;


#define T(c,n) t->create_linterm ((c),(n))
#define C(n) t->create_int_cst(n)
#define CONS(tm,n,c) t->create_cons (T(tm,n),0,C(c))
  

void test0 ()
{

  /*
     1 <= x <= 3 && 2 <= y <= 3
     do  x <- y
     expect: 2 <= x <= 3 && 2 <= y <= 3 */
  


  /* variable ordering:
   * 0:z, 1:x, 2:y 
   * z is used as a special ZERO variable
   */

  constant_t k1, k2;
  
  int xMz[3] = {-1, 1, 0};
  int zMx[3] = {1, -1, 0};
  int yMz[3] = {-1, 0, 1};
  int zMy[3] = {1, 0, -1};
  
  
  lincons_t l1, l2, l3, l4, l5;
  tdd_node *d1, *d2, *d3, *d4, *d5;

  tdd_node *box1, *box2, *box3, *box4, *box5, *box6;

  /* x <= 3 */
  l1 = CONS(xMz, 3, 3);
  d1 = to_tdd (tdd, l1);
  Cudd_Ref (d1);  
  
  /* 1 <= x */
  l2 = CONS(zMx, 3, -1);
  d2 = to_tdd(tdd, l2);
  Cudd_Ref (d2);

  /* 2 <= x */
  l3 = CONS(zMx, 3, -2);
  d3 = to_tdd(tdd, l3);
  Cudd_Ref (d3);

  /* y <= 5 */
  l4 = CONS(yMz, 3, 3);
  d4 = to_tdd (tdd, l4);
  Cudd_Ref (d4);  

  /* 2 <= y */
  l5 = CONS(zMy, 3, -2);
  d5 = to_tdd(tdd, l5);
  Cudd_Ref (d5);


  /* 1 <= x < = 3 */
  box1 = tdd_and (tdd, d1, d2);
  Cudd_Ref (box1);
  
  /* 2 <= y <= 3 */
  box2 = tdd_and (tdd, d4, d5);
  Cudd_Ref (box2);
  
  /* 1 <= x <= 3 && 2 <= y <= 5 */
  box3 = tdd_and (tdd, box1, box2);
  Cudd_Ref (box3);

  k1 = C(1);
  k2 = C(0);

  box4 = tdd_term_replace (tdd, box3, 
			   t->get_term (l2), t->get_term (l5),
			   k1, k1, k2, k2);
  Cudd_Ref (box4);

  t->destroy_cst (k1); k1 = NULL;
  t->destroy_cst (k2); k2 = NULL;

  /* 2 <= x <= 3 */
  box5 = tdd_and (tdd, d1, d3);
  Cudd_Ref (box5);
  
  box6 = tdd_and (tdd, box5, box2);
  Cudd_Ref (box6);

  
  Cudd_PrintMinterm (cudd, box3);
  printf ("\n");
  
  Cudd_PrintMinterm (cudd, box4);
  printf ("\n");
  Cudd_PrintMinterm (cudd, box6);
  
  assert (box6 == box4);


}


int main(void)
{
  cudd = Cudd_Init (0, 0, CUDD_UNIQUE_SLOTS, 127, 0);
  t = ddd_create_int_theory (3);
  tdd = tdd_init (cudd, t);
  test0 ();  

  return 0;
}