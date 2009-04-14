#include "util.h"
#include "tddInt.h"

/** BDD Style existential quantification */
tdd_node * 
tdd_bdd_exist_abstract (tdd_manager * tdd,
			tdd_node * f,
			tdd_node * cube)
{
  DdNode *res;
  
  do
    {
      CUDD->reordered = 0;
      res = tdd_bdd_exist_abstract_recur (tdd, f, cube);
    } while (CUDD->reordered == 1);

  return res;
}


/**
 * Over-approximates existential quantification of all variables in
 * Boolean array vars.
 */
tdd_node * 
tdd_over_abstract (tdd_manager *tdd, 
		   tdd_node * f, 
		   bool * vars)
{

  tdd_node * cube, *res;
  
  cube = tdd_terms_with_vars (tdd, vars);
  
  if (cube == NULL) return NULL;
  cuddRef (cube);

  res = tdd_bdd_exist_abstract (tdd, f, cube);

  if (res != NULL)
    cuddRef (res);
  
  Cudd_IterDerefBdd (CUDD, cube);
  
  if (res != NULL)
    cuddDeref (res);
  
  return res;
}


/**
 * Constructs a cube of all of the terms that contains variables in
 * the Boolean array vars.
 */
tdd_node *
tdd_terms_with_vars (tdd_manager *tdd,
		     bool * vars)
{
  tdd_node * res;
  
  int i;

  res = DD_ONE (CUDD);
  cuddRef (res);
  
  for (i = tdd->varsSize - 1; i >= 0; i--)
    {
      if (tdd->ddVars [i] == NULL) continue;
      
      if (THEORY->term_has_vars (THEORY->get_term (tdd->ddVars [i]), vars))
	{
	  tdd_node *tmp;
	  
	  /* MUST use Cudd_bddAnd. This constructs a cube -- a set of
	     variables. TDD simplifications do not apply. */
	  tmp = Cudd_bddAnd (CUDD, res, CUDD->vars [i]);
	  if (tmp == NULL)
	    {
	      Cudd_IterDerefBdd (CUDD, res);
	      return NULL;
	    }
	  
	  cuddRef (tmp);
	  Cudd_IterDerefBdd (CUDD, res);
	  res = tmp;
	}
    }

  cuddDeref (res);
  return res;
    
}



/* taken from cuddBddAbs.c/cuddBddExistAbstractRecur */
tdd_node *
tdd_bdd_exist_abstract_recur (tdd_manager *tdd,
			      tdd_node *f,
			      tdd_node *cube)
{
  tdd_node *F, *T, *E, *res, *res1, *res2, *one;
  DdManager *manager;
  

  manager = CUDD;
  statLine (manager);
  one = DD_ONE (manager);
  F = Cudd_Regular (f);
  
  if (cube == one || F == one) return (f);
  
    /* Abstract a variable that does not appear in f. */
    while (manager->perm[F->index] > manager->perm[cube->index]) {
	cube = cuddT(cube);
	if (cube == one) return(f);
    }

    /* Check the cache. */
    if (F->ref != 1 && 
	(res = cuddCacheLookup2(manager, 
				(DD_CTFP)tdd_bdd_exist_abstract, 
				f, cube)) != NULL) {
	return(res);
    }

    /* Compute the cofactors of f. */
    T = cuddT(F); E = cuddE(F);
    if (f != F) {
	T = Cudd_Not(T); E = Cudd_Not(E);
    }

    /* If the two indices are the same, so are their levels. */
    if (F->index == cube->index) {
	if (T == one || E == one || T == Cudd_Not(E)) {
	    return(one);
	}
	res1 = tdd_bdd_exist_abstract_recur(tdd, T, cuddT(cube));
	if (res1 == NULL) return(NULL);
	if (res1 == one) {
	    if (F->ref != 1)
	      cuddCacheInsert2(manager, 
			       (DD_CTFP)tdd_bdd_exist_abstract, f, cube, one);
	    return(one);
	}
        cuddRef(res1);
	res2 = tdd_bdd_exist_abstract_recur(tdd, E, cuddT(cube));
	if (res2 == NULL) {
	    Cudd_IterDerefBdd(manager,res1);
	    return(NULL);
	}
        cuddRef(res2);
	res = tdd_and_recur(tdd, Cudd_Not(res1), Cudd_Not(res2));
	if (res == NULL) {
	    Cudd_IterDerefBdd(manager, res1);
	    Cudd_IterDerefBdd(manager, res2);
	    return(NULL);
	}
	res = Cudd_Not(res);
	cuddRef(res);
	Cudd_IterDerefBdd(manager, res1);
	Cudd_IterDerefBdd(manager, res2);
	if (F->ref != 1)
	  cuddCacheInsert2(manager, 
			   (DD_CTFP)tdd_bdd_exist_abstract, f, cube, res);
	cuddDeref(res);
        return(res);
    } else { /* if (cuddI(manager,F->index) < cuddI(manager,cube->index)) */
	res1 = tdd_bdd_exist_abstract_recur(tdd, T, cube);
	if (res1 == NULL) return(NULL);
        cuddRef(res1);
	res2 = tdd_bdd_exist_abstract_recur(tdd, E, cube);
	if (res2 == NULL) {
	    Cudd_IterDerefBdd(manager, res1);
	    return(NULL);
	}
        cuddRef(res2);
	/* ITE takes care of possible complementation of res1 and of the
        ** case in which res1 == res2. */
	res = tdd_ite_recur(tdd, manager->vars[F->index], res1, res2);
	if (res == NULL) {
	    Cudd_IterDerefBdd(manager, res1);
	    Cudd_IterDerefBdd(manager, res2);
	    return(NULL);
	}
	cuddDeref(res1);
	cuddDeref(res2);
	if (F->ref != 1)
	  cuddCacheInsert2(manager, 
			   (DD_CTFP)tdd_bdd_exist_abstract, f, cube, res);
        return(res);
    }	    
  
}