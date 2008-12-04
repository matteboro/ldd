#include "util.h"
#include "tddInt.h"


/* depth < 0 means no depth limit */
tdd_node *
tdd_sat_reduce (tdd_manager *tdd, 
		tdd_node *f,
		int depth)
{
  tdd_node *res;
  qelim_context_t *ctx;
  bool * vars;
  int i, n;
  

  n = THEORY->num_of_vars (THEORY);
  vars = ALLOC (bool, n);
  if (vars == NULL) return NULL;
  
  for (i = 0; i < n; i++)
    vars [i] = 1;
  
  
  ctx = THEORY->qelim_init (tdd, vars);

  if (ctx == NULL)
    {
      FREE (vars);
      return NULL;
    }

  do {
    CUDD->reordered = 0;
    res = tdd_sat_reduce_recur (tdd, f, ctx, depth);
    if (res != NULL)
      cuddRef (res);
  } while (CUDD->reordered == 1);


  if (ctx)
    {
      THEORY->qelim_destroy_context (ctx);
      ctx = NULL;
    }

  if (vars)
    {
      FREE (vars);
      vars = NULL;
    }
  

  if (res != NULL)
    cuddDeref (res);
  return res;
}


tdd_node *
tdd_sat_reduce_recur (tdd_manager *tdd, 
		      tdd_node *f,
		      qelim_context_t * ctx,
		      int depth)
{
  tdd_node *F, *T, *E;

  tdd_node *fv, *fnv;
  
  tdd_node *tmp;
  
  unsigned int v;
  lincons_t vCons, nvCons;
  
  tdd_node *root;
  tdd_node *res;

  tdd_node *zero;
  

  /* reached our limit */
  if (depth == 0) return f;
  
  F = Cudd_Regular (f);
  
  /* terminal constant */
  if (F == DD_ONE(CUDD)) return f;

  zero = Cudd_Not (DD_ONE (CUDD));
  v = F->index;
  vCons = tdd->ddVars [v];

  fv = Cudd_NotCond (cuddT (F), f != F);
  fnv = Cudd_NotCond (cuddE (F), f != F);

  T = E = NULL;


  THEORY->qelim_push (ctx, vCons);

  tmp = THEORY->qelim_solve (ctx);
  if (tmp == NULL) return NULL;

  if (tmp != zero)
    {
      cuddRef (tmp);
      Cudd_IterDerefBdd (CUDD, tmp);

      T = tdd_sat_reduce_recur (tdd, fv, ctx, depth - 1);
      
      if (T == NULL)
	return NULL;
      cuddRef (T);
    }

  tmp = NULL;
  THEORY->qelim_pop (ctx);
  nvCons = THEORY->negate_cons (vCons);
  THEORY->qelim_push (ctx, nvCons);
  
  tmp = THEORY->qelim_solve (ctx);
  THEORY->destroy_lincons (THEORY->qelim_pop (ctx));
  if (tmp == NULL)
    {
      Cudd_IterDerefBdd (CUDD, T);
      return NULL;
    }

  if (tmp != zero)
    {
      cuddRef (tmp);
      Cudd_IterDerefBdd (CUDD, tmp);
      
      E = tdd_sat_reduce_recur (tdd, fnv, ctx, depth - 1);
      
      if (E == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, T);
	  return NULL;
	}
      cuddRef (E);
    }
  
  
  if (T == NULL || E == NULL)
    res = T != NULL ? T : E;
  else
    {
      root = Cudd_bddIthVar (CUDD, v);

      if (root == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, T);
	  Cudd_IterDerefBdd (CUDD, E);
	  return NULL;
	}

      res = tdd_ite_recur (tdd, root, T, E);

      if (res != NULL)
	cuddRef (res);

      Cudd_IterDerefBdd (CUDD, T);
      Cudd_IterDerefBdd (CUDD, E);
      Cudd_IterDerefBdd (CUDD, root);
      root = NULL;
    }
  
  if (res != NULL)
    cuddDeref (res);
  return res;
  
}

