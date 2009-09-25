#include "util.h"
#include "tddInt.h"

tdd_nodeset *
tdd_emtpy_nodeset (tdd_manager* tdd)
{
  return (tdd_nodeset*) tdd_get_true (tdd);
}

tdd_nodeset * 
tdd_nodeset_add (tdd_manager* tdd, tdd_nodeset* s, tdd_node *f)
{
  return tdd_nodeset_union (tdd, tdd_to_nodeset (f), s);
}

tdd_nodeset *
tdd_nodeset_union (tdd_manager* tdd, tdd_nodeset *f, tdd_nodeset* g)
{
  DdNode *res;
  int rs = CUDD->autoDyn;
  CUDD->autoDyn = 0;
  
  do 
    {
      CUDD->reordered = 0;
      res = tdd_nodeset_union_recur (tdd, f, g);
    } while (CUDD->reordered == 1);
  
  CUDD->autoDyn = rs;
  return res;
}

int
tdd_is_valid_nodeset (tdd_manager *tdd, tdd_nodeset *f)
{
  tdd_node *r;

  r = f;
  while (r != DD_ONE(CUDD))
    {
      assert (Cudd_Regular (r) == r && "NODESETs are positive ");
      assert (cuddT(r) == DD_ONE(CUDD) && "NON 1 THEN CHILD");
      assert (Cudd_Regular (cuddE(r)) != cuddE(r) && "ELSE must be negated");
      r = Cudd_Regular (cuddE (r));
    }
  return 1;
}


tdd_nodeset * 
tdd_nodeset_union_recur (tdd_manager* tdd, tdd_nodeset *f, tdd_nodeset *g)
{
  tdd_node *fnv, *gnv;
  tdd_node *res, *e;
  unsigned topf, topg, index;

  assert (Cudd_Regular (f) == f);
  assert (Cudd_Regular (g) == g);

  if (f == g) return g;
  if (f == DD_ONE(CUDD)) return g;
  if (g == DD_ONE(CUDD)) return f;

  /* f and g are not constants */
  assert (cuddT(f) == DD_ONE (CUDD));
  assert (cuddT(g) == DD_ONE (CUDD));
  
  /* Try to increase cache efficiency */
  if (f > g)
    {
      DdNode *tmp = f;
      f = g;
      g = tmp;
    }

  if (f->ref != 1 || g->ref != 1)
    {
      res = cuddCacheLookup2 (CUDD, (DD_CTFP)tdd_nodeset_union, f, g);
      if (res !=  NULL) 
	return res;
    }
  

  topf = CUDD->perm [f->index];
  topg = CUDD->perm [g->index];
  
  if (topf <= topg)
    {
      index = f->index;
      fnv = Cudd_Regular (cuddE(f));
      assert (fnv != cuddE(f));
    }
  else
    {
      index = g->index;
      fnv = f;
    }
  
  if (topg <= topf)
    {
      gnv = Cudd_Regular (cuddE (g));
      assert (gnv != cuddE(g));
    }
  else
    gnv = g;
  

  e = tdd_nodeset_union_recur (tdd, fnv, gnv);
  if (e == NULL)
    return NULL;
  cuddRef (e);

  res = tdd_unique_inter (tdd, index, DD_ONE(CUDD), Cudd_Not (e));
  if (res != NULL) cuddRef (res);
  Cudd_IterDerefBdd (CUDD, e);
  if (res == NULL) return NULL;
  
  if (f->ref != 1 || g->ref != 1)
    cuddCacheInsert2 (CUDD, (DD_CTFP)tdd_nodeset_union, f, g, res);
  
  cuddDeref (res);
  return res;
}


