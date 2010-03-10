#include "util.h"
#include "tddInt.h"



LddNode *
Ldd_BoxWiden (LddManager *ldd, LddNode *f, LddNode *g)
{
  LddNode *res;
  
  do
    {
      CUDD->reordered = 0;
      res = lddBoxWidenRecur (ldd, f, g);
    } while (CUDD->reordered == 1);
  
  return (res);
}


/** 
 * Extrapolation for intervals. 
 * Extrapolates f by g. Assumes that f IMPLIES g
 * Untested, unproven, use at your own risk 
 */
LddNode * 
Ldd_BoxExtrapolate (LddManager *ldd, LddNode *f, LddNode *g)
{
  LddNode *res;
  
  do 
    {
      CUDD->reordered = 0;
      res = lddBoxExtrapolateRecur (ldd, f, g);
    } while (CUDD->reordered == 1);
  return (res);
}

/**
 * Apply t1 <- a*t2 + [kmin,kmax], i.e, replace all
 * constraints on term t1 with a linear combination on constraints
 * with term t2.
 *
 * t2 can be NULL, in which case a is ignored
 * kmin == NULL means kmin is negative infinity
 * kmax == NULL means kmax is positive infinity
 */
LddNode * 
Ldd_TermReplace (LddManager *ldd, LddNode *f, linterm_t t1, 
		  linterm_t t2, constant_t a, constant_t kmin, constant_t kmax)
{
  LddNode *res;
  DdHashTable * table;

  /* local copy of t2 */
  linterm_t lt2;
  /* local copy of a */
  constant_t la;
  

  /* if (t2 != NULL) then a != NULL */
  assert (t2 == NULL || a != NULL);
  /* if t2 != NULL then at least one of kmin or kmax is not NULL */
  assert (t2 == NULL || (kmin != NULL || kmax != NULL));
  
  /* simplify */
  if (a != NULL && THEORY->sgn_cst (a) == 0)
    {
      lt2 = NULL;
      la = NULL;
    }
  else
    {
      lt2 = t2;
      la = a;
    }
  
  do 
    {
      CUDD->reordered = 0;
      table = cuddHashTableInit (CUDD, 1, 2);
      if (table == NULL) return NULL;
      
      res = lddTermReplaceRecur (ldd, f, t1, lt2, 
				    la, kmin, kmax,table);
      if (res != NULL)
	cuddRef (res);
      cuddHashTableQuit (table);
    } while (CUDD->reordered == 1);
  
  if (res != NULL) cuddDeref (res);
  return (res);
}

/**
 * Constrain f with  t1 <= t2 + k. 
 * f is an LDD, t1 and t2 are terms, k a constant
 */
LddNode * 
Ldd_TermConstrain (LddManager *ldd, LddNode *f, linterm_t t1, 
		    linterm_t t2, constant_t k)
{
  LddNode *res;
  DdLocalCache *cache;
  
  do 
    {
      CUDD->reordered = 0;
      cache = cuddLocalCacheInit (CUDD, 1, 2, CUDD->maxCacheHard);
      if (cache == NULL) return NULL;
      
      res = lddTermConstrainRecur (ldd, f, t1, t2, k, cache);
      if (res != NULL)
	cuddRef (res);
      cuddLocalCacheQuit (cache);
    } while (CUDD->reordered == 1);
  
  if (res != NULL) cuddDeref (res);
  return (res);
}


/**
 * Approximates f by computing the min and max bounds for all terms.
 */
LddNode * 
Ldd_TermMinmaxApprox (LddManager *ldd, LddNode *f)
{
  LddNode *res;
  
  do 
    {
      CUDD->reordered = 0;
      res = lddTermMinmaxApproxRecur (ldd, f);
    } while (CUDD->reordered == 1);
  return (res);
}




LddNode *
lddBoxExtrapolateRecur (LddManager *ldd,
			   LddNode *f,
			   LddNode *g)
{
  DdManager * manager;
  DdNode *F, *fv, *fnv, *G, *gv, *gnv, *Fnv, *Gnv;
  DdNode *one, *zero, *r, *t, *e;
  unsigned int topf, topg, index;

  lincons_t vCons;

  manager = CUDD;
  statLine(manager);
  one = DD_ONE(manager);
  zero = Cudd_Not (one);

  F = Cudd_Regular(f);
  G = Cudd_Regular(g);


  /* Terminal cases. */
  if (F == one || G == one || F == G) return g;

  /* At this point f and g are not constant. */
  /* widen is asymmetric. cannot switch operands */

  /* Check cache. */
  if (F->ref != 1 || G->ref != 1) {
    r = cuddCacheLookup2(manager, (DD_CTFP)Ldd_BoxExtrapolate, f, g);
    if (r != NULL) return(r);
  }
  
  /* Get the levels */
  /* Here we can skip the use of cuddI, because the operands are known
  ** to be non-constant.
  */
  topf = manager->perm[F->index];
  topg = manager->perm[G->index];

  /* Compute cofactors. */
  if (topf <= topg) {
    index = F->index;
    fv = cuddT(F);
    fnv = cuddE(F);
    if (Cudd_IsComplement(f)) {
      fv = Cudd_Not(fv);
      fnv = Cudd_Not(fnv);
    }
  } else {
    index = G->index;
    fv = fnv = f;
  }
  
  if (topg <= topf) {
    gv = cuddT(G);
    gnv = cuddE(G);
    if (Cudd_IsComplement(g)) {
      gv = Cudd_Not(gv);
      gnv = Cudd_Not(gnv);
    }
  } else {
    gv = gnv = g;
  }


  /** 
   * Get the constraint of the root node 
   */
  vCons = ldd->ddVars [index];

  /** 
   * If f and g have the same term, simplify the THEN part of the
   * non-root diagram. This eliminates a redundant test. This assumes
   * that if a constraint A is less than in diagram ordering than B
   * then A implies B.
   * 
   * We check whether f and g have the same constraint using the
   * following facts: 
   *   vInfo is the constraint of the root diagram
   *   gv == g iff g is not the root diagram
   *   fv == f iff f is not the root diagram
   */
  if (gv == g)
    {
      lincons_t gCons = ldd->ddVars [G->index];
      
      if (THEORY->is_stronger_cons (vCons, gCons))
	{
	  gv = cuddT (G);
	  if (Cudd_IsComplement (g))
	    gv = Cudd_Not (gv);
	}
    }
  else if (fv == f)
    {
      lincons_t fCons = ldd->ddVars [F->index];
      
      if (THEORY->is_stronger_cons (vCons, fCons))
	{
	  fv = cuddT (F);
	  if (Cudd_IsComplement (f))
	    fv = Cudd_Not (fv);
	}
    }
  
  
  
  /* Here, fv, fnv are lhs and rhs of f, 
           gv, gnv are lhs and rhs of g,
	   index is the index of the  root variable 
  */

  t = lddBoxExtrapolateRecur (ldd, fv, gv);
  if (t == NULL) return (NULL);
  cuddRef (t);
  
  e = lddBoxExtrapolateRecur (ldd, fnv, gnv);
  if (e == NULL)
    {
      Cudd_IterDerefBdd (manager, t);
      return NULL;
    }
  cuddRef (e);
  
  r = NULL;

  Fnv = Cudd_Regular (fnv);
  Gnv = Cudd_Regular (gnv);

  if (t == e)
    {
      r = t;
      cuddRef (r);
    }
  /** 
   ** extrapolate to the right
   ** perm[F->index] < perm[G->index]  implies that F and G are different
   ** gv != g  implies that C(f) implies C(g)
   ** perm[G->index] < perm[Fnv->index] implies that C(g) implies C(fnv)
   **/
  else if ( (CUDD->perm[F->index] < CUDD->perm[G->index]) &&
	    (gv != g) &&
	    (Fnv->index == CUDD_CONST_INDEX ||
	     CUDD->perm[G->index] < CUDD->perm[Fnv->index]) &&
	    (fnv == zero || 
	     (Fnv != one &&
	      cuddT (Fnv) == ((fnv == Fnv) ? zero : one))))
    {      
      /* r = OR (t, e) */
      r = lddAndRecur (ldd, Cudd_Not (t), Cudd_Not (e));
      r = Cudd_NotCond (r, r != NULL);

      if (r == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, t);
	  Cudd_IterDerefBdd (CUDD, e);
	  return NULL;
	}
      cuddRef (r);

    }
  /** extrapolate to the left */
  else if (CUDD->perm[G->index] < CUDD->perm[F->index] && 
	   fv == zero &&
	   f != fv &&
	   (Gnv->index == CUDD_CONST_INDEX ||
	    CUDD->perm[F->index] < CUDD->perm[Gnv->index]))

    {
      DdNode *h, *k, *w;
      lincons_t fCons;
      
      fnv = cuddE (F);
      fnv = Cudd_NotCond (fnv, F != f);
      Fnv = Cudd_Regular (fnv);
      
      fCons = ldd->ddVars [F->index];
      
      if (Fnv->index != CUDD_CONST_INDEX && 
          THEORY->is_stronger_cons (fCons, ldd->ddVars [Fnv->index]))
	h = Cudd_NotCond (cuddT(Fnv), Fnv != fnv);
      else
	h = fnv;
      
      if (Gnv->index != CUDD_CONST_INDEX && 
          THEORY->is_stronger_cons (vCons, ldd->ddVars [Gnv->index]))
	k = Cudd_NotCond (cuddT (Gnv), Gnv != gnv);
      else
	k = gnv;
      
      /* clear out t and e since we will recompute them */
      Cudd_IterDerefBdd (CUDD, t);
      t = NULL;
      Cudd_IterDerefBdd (CUDD, e);
      e = NULL;
      
      w = lddBoxExtrapolateRecur (ldd, h, k);
      
      if (w == NULL) return NULL;      
      cuddRef (w);

      /* t = OR (gv, w) */
      t = lddAndRecur (ldd, Cudd_Not (gv), Cudd_Not (w));
      t = Cudd_NotCond (t, t != NULL);

      if (t == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, w);
	  return NULL;
	}
      cuddRef (t);
      Cudd_IterDerefBdd (CUDD, w);
      
      e = lddBoxExtrapolateRecur (ldd, fnv, gnv);
      if (e == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, t);
	  return NULL;
	}
      cuddRef (e);

    }
  

  if (r == NULL)
    {
      DdNode *root;
      
      root = Cudd_bddIthVar (CUDD, index);

      if (root == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, t);
	  Cudd_IterDerefBdd (CUDD, e);
	  return NULL;
	}
      cuddRef (root);

      /** XXX should use Ldd_unique_inter instead */
      r = lddIteRecur (ldd, root, t, e);
      if (r == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, t);
	  Cudd_IterDerefBdd (CUDD, e);
	  return NULL;
	}
      cuddRef (r);
      Cudd_IterDerefBdd (CUDD, root);
    }
  

  Cudd_IterDerefBdd (CUDD, t);
  Cudd_IterDerefBdd (CUDD, e);

  if (F->ref != 1 || G->ref != 1)
    cuddCacheInsert2(manager, (DD_CTFP)Ldd_BoxExtrapolate, f, g, r);

  cuddDeref (r);
  return r;
  
}

LddNode *
lddBoxWidenRecur (LddManager *ldd,
		  LddNode *f,
		  LddNode *g)
{
  DdManager * manager;
  DdNode *F, *fv, *fnv, *G, *gv, *gnv;
  DdNode *one, *zero, *r, *t, *e;
  unsigned int topf, topg, index;

  lincons_t vCons;

  manager = CUDD;
  statLine(manager);
  one = DD_ONE(manager);
  zero = Cudd_Not (one);

  F = Cudd_Regular(f);
  G = Cudd_Regular(g);


  /* Terminal cases. */
  if (F == one || G == one || F == G) return g;

  /* At this point f and g are not constant. */
  /* widen is asymmetric. cannot switch operands */

  /* Check cache. */
  if (F->ref != 1 || G->ref != 1) {
    r = cuddCacheLookup2(manager, (DD_CTFP)Ldd_BoxWiden, f, g);
    if (r != NULL) return(r);
  }
  else
    r = NULL;
  
  /* Get the levels */
  /* Here we can skip the use of cuddI, because the operands are known
  ** to be non-constant.
  */
  topf = manager->perm[F->index];
  topg = manager->perm[G->index];

  /* Compute cofactors. */
  if (topf <= topg) {
    index = F->index;
    fv = cuddT(F);
    fnv = cuddE(F);
    if (Cudd_IsComplement(f)) {
      fv = Cudd_Not(fv);
      fnv = Cudd_Not(fnv);
    }
  } else {
    index = G->index;
    fv = fnv = f;
  }
  
  if (topg <= topf) {
    gv = cuddT(G);
    gnv = cuddE(G);
    if (Cudd_IsComplement(g)) {
      gv = Cudd_Not(gv);
      gnv = Cudd_Not(gnv);
    }
  } else {
    gv = gnv = g;
  }


  /** 
   * Get the constraint of the root node 
   */
  vCons = lddC (ldd, index);

  /** 
   * Compute LDD cofactors w.r.t. the top term.  
   * 
   * We check whether f and g have the same constraint using the
   * following facts: 
   *   vCons is the constraint of the root diagram
   *   gv == g iff g is not the root diagram
   *   fv == f iff f is not the root diagram
   */
  if (gv == g)
    {
      lincons_t gCons = lddC (ldd, G->index);
      
      if (THEORY->is_stronger_cons (vCons, gCons))
	{
	  gv = cuddT (G);
	  if (Cudd_IsComplement (g))
	    gv = Cudd_Not (gv);
	}
    }
  else if (fv == f)
    {
      lincons_t fCons = lddC (ldd, F->index);
      
      if (THEORY->is_stronger_cons (vCons, fCons))
	{
	  fv = cuddT (F);
	  if (Cudd_IsComplement (f))
	    fv = Cudd_Not (fv);
	}
    }
  
  
  e = lddBoxWidenRecur (ldd, fnv, gnv);
  if (e == NULL) return NULL;
  cuddRef (e);

  if (index != topf && fv == gv)
    /* reference to e is migrated into r */
    r = e;
  else
    {
      t = lddBoxWidenRecur (ldd, fv, gv);
      if (t == NULL)
	{
	  Cudd_IterDerefBdd (manager, e);
	  return NULL;
	}
      cuddRef (t);
  
      if (index != topf)
	{
	  unsigned int u;
	  DdNode *E, *eu, *enu;
	  lincons_t eCons = NULL;
	  
	  E = Cudd_Regular (e);	
	  
	  if (E != one)
	    eCons = lddC (ldd, E->index);
	  
	  if (eCons != NULL && THEORY->is_stronger_cons (vCons, eCons))
	    eu = Cudd_NotCond (cuddT (e), e != E);
	  else
	    eu = e;
	  
	  if (eu == fv)
	    {

	      if (eu == e && fv == f)
		{
		  r = t;
		  Cudd_IterDerefBdd (CUDD, e);
		}
	      else
		{
		  if (fv == f)
		    u = E->index;
		  else if (eu == e)
		    u = F->index;
		  else
		    u = F->index < E->index ? F->index : E->index;
		  

		  if (E->index == u)
		    enu = Cudd_NotCond (cuddE (e), e != E);
		  else
		    enu = e;
		  
		  cuddRef (enu);
		  Cudd_IterDerefBdd (CUDD, e);

		  e = enu;
		}
	    }
	} /* if (index != topf) */
      
      
      if (r == NULL)
	{
	  if (t == e)
	    r = t;
	  else if (Cudd_IsComplement (t))
	    r = Cudd_Not (lddUniqueInter (ldd, index, 
					  Cudd_Not (t), Cudd_Not (e)));
	  else
	    r = lddUniqueInter (ldd, index, t, e);
	  
	  if (r != NULL) cuddRef (r);
	  Cudd_IterDerefBdd (CUDD, t);
	  Cudd_IterDerefBdd (CUDD, e);
	  if (r == NULL) return NULL;
	}
    }
  
  
  if (F->ref != 1 || G->ref != 1)
    cuddCacheInsert2(manager, (DD_CTFP)Ldd_BoxWiden, f, g, r);
  
  cuddDeref (r);
  return r;
}


LddNode * 
lddTermReplaceRecur (LddManager * ldd, LddNode *f, 
			linterm_t t1, linterm_t t2, 
			constant_t a,
			constant_t kmin, constant_t kmax,
			DdHashTable* table)
{
  DdNode *F, *res, *fv, *fnv, *t, *e;
  DdNode *rootT, *rootE;
  
  lincons_t fCons;
  linterm_t fTerm;
  

  F = Cudd_Regular (f);
  
  if (F == DD_ONE (CUDD)) 
    {
      if (t2 == NULL && f == DD_ONE(CUDD))
	{
          /* add   kmin <= t1 <= kmax */

	  lincons_t maxCons;
	  lincons_t minCons;
	  DdNode *tmp1, *tmp2;
	  
	  res = NULL;
	  
	  /* kmax is not +oo */
          if (kmax != NULL)
            {
              maxCons = THEORY->create_cons (THEORY->dup_term (t1), 0,
                                             THEORY->dup_cst (kmax));
              if (maxCons == NULL) 
                  return NULL;
          
              res = Ldd_FromCons (ldd, maxCons);
              THEORY->destroy_lincons (maxCons);
              if (res == NULL) return NULL;
              cuddRef (res); 
            }
	  else
	    {
	      /* simplify reference counting */
	      res = DD_ONE(CUDD);
	      cuddRef (res);
	    }
	  
	  /* here, ref count of res is 1 */

	  /* kmin is not -oo */
          if (kmin != NULL)
            {
              minCons = THEORY->create_cons (THEORY->negate_term (t1), 0,
                                             THEORY->negate_cst (kmin));
              if (minCons == NULL) 
                {
                  Cudd_IterDerefBdd (CUDD, res);
                  return NULL;
                }
              tmp1 = Ldd_FromCons (ldd, minCons);
              if (tmp1 == NULL)
                {
                  Cudd_IterDerefBdd (CUDD, res);
                  return NULL;
                }
              cuddRef (tmp1);

              tmp2 = lddAndRecur (ldd, res, tmp1);
              if (tmp2 != NULL) cuddRef (tmp2);
              Cudd_IterDerefBdd (CUDD, res);
              Cudd_IterDerefBdd (CUDD, tmp1);
              if (tmp2 == NULL) return NULL;
              res = tmp2;
            }

	  cuddDeref (res);
	  return res;
	}
      
      return f;  
    }


  /* XXX if terms t1 and t2 do not appear in f, can stop here */

  /* check cache */
  if (F->ref != 1 && ((res = cuddHashTableLookup1 (table, f)) != NULL))
    return res;

  /* cofactors */
  fv = Cudd_NotCond (cuddT(F), F != f);
  fnv = Cudd_NotCond (cuddE(F), F != f);
  

  /* roots of the result diagram */
  rootT = NULL;
  rootE = NULL;
  
  fCons = ldd->ddVars [F->index];
  fTerm = THEORY->get_term (fCons);
  
  /* remove top term if it is t1 */
  if (THEORY->term_equals (fTerm, t1))
    {
      rootT = DD_ONE(CUDD);
      cuddRef (rootT);
      
      rootE = DD_ONE(CUDD);
      cuddRef (rootE);
    }
  /* keep top term if it is not t1 */
  else
    {
      rootT = Cudd_bddIthVar (CUDD, F->index);
      if (rootT == NULL) return NULL;
      cuddRef (rootT);
      
      rootE = Cudd_Not (rootT);
      cuddRef (rootE);
    }
  
  
  /* if top constraint is  t2 < c and a >0 
   *       add t1 < a*c + kmax to the THEN branch
   *      and !(t1 < a*c + kmin) to the ELSE branch 
   *
   * if top constraint is t2 < c and a < 0
   *     add t1 > a*c + kmin to the THEN branch
   *     and t1 < a*c + kmax to the ELSE branch
   */
  if (t2 != NULL && THEORY->term_equals (fTerm, t2))
    {
      DdNode *tmp, *d;
      constant_t fCst, tmpCst, nCst;
      lincons_t newCons;

      int pos; int strict;
      
      pos = (THEORY->sgn_cst (a) >= 0);
      strict = THEORY->is_strict (fCons);
      fCst = THEORY->get_constant (fCons);

      d = NULL;

      
      /* kmax == NULL means kmax is positive infinity, same for kmin
       * Don't add constraints if the constant is infinity
       */
      if ((pos && kmax != NULL) || (!pos && kmin != NULL))
	{
	  /* nCst = a * fCst + kmax */
	  tmpCst = THEORY->mul_cst (a, fCst);
	  nCst = THEORY->add_cst (tmpCst, pos ? kmax : kmin);
	  THEORY->destroy_cst (tmpCst);
	  tmpCst = NULL;

	  if (!pos) strict = !strict;

	  /* since t2 < fCst, 
	     newCons is  
                 t1 < a * fCst + kmax, if a>0, and
	         t1 <= a * fCst + kmin, if a<0
	   */
	  newCons = THEORY->create_cons (THEORY->dup_term (t1), 
					 strict, 
					 nCst);
	  /* nCst is now managed by createCons */

	  d = Ldd_FromCons (ldd, newCons);
	  THEORY->destroy_lincons (newCons);
	  newCons = NULL;
	  if (d == NULL)
	    {
	      Cudd_IterDerefBdd (CUDD, rootT);
	      Cudd_IterDerefBdd (CUDD, rootE);
	      return NULL;
	    }
	  cuddRef (d);

	  /* if a<0, need negation of newCons */
	  d = Cudd_NotCond (d, !pos);

	  /* add d to the THEN branch */
	  tmp = lddAndRecur (ldd, rootT, d);
	  if (tmp != NULL) cuddRef (tmp);
	  Cudd_IterDerefBdd (CUDD, rootT);
	  if (tmp == NULL)
	    {
	      Cudd_IterDerefBdd (CUDD, rootE);
	      Cudd_IterDerefBdd (CUDD, d);
	      return NULL;
	    }
	  rootT = tmp;
	  tmp = NULL;
	}

      /* have a lower bound */
      if ((pos && kmin != NULL) || (!pos && kmax != NULL))
	{
	  /* if t2 is not NULL then kmin and kmax cannot be both NULL */
	  if (kmin == kmax)
	    /* Same min and max bounds. 
	     * Use negation of the THEN constraint */
	    d = Cudd_Not (d);
	  else /* kmin != kmax */
	    /* compute d for the ELSE branch */
	    {
	      if (d != NULL)
		Cudd_IterDerefBdd (CUDD, d);
	      d = NULL;
	  
	      /* nCst = a * fCst + kmin */
	      tmpCst = THEORY->mul_cst (a, fCst);
	      nCst = THEORY->add_cst (tmpCst, pos ? kmin : kmax);
	      THEORY->destroy_cst (tmpCst);
	      tmpCst = NULL;
      
	      /* since !(t2 <= fCst),
		  newCons is 
		    t1 <=  a*fCst + kmin if a>0, and
                    t1 < a*fCst + kmax if a<0 
		    
		 Note that newCons is used negated if a>0, and as is otherwise
	      */
	      newCons = THEORY->create_cons (THEORY->dup_term (t1), 
					     strict,
					     nCst);
	      /* nCst is now managed by createCons */

	      d = Ldd_FromCons (ldd, newCons);
	      THEORY->destroy_lincons (newCons);
	      newCons = NULL;
	      if (d == NULL)
		{
		  Cudd_IterDerefBdd (CUDD, rootT);
		  Cudd_IterDerefBdd (CUDD, rootE);
		  return NULL;
		}
	      cuddRef (d);
	      /* if a>0, need negation of newCons */
	      d = Cudd_NotCond (d, pos);
	    }
      
	  if (d != NULL)
	    {
	      /* add d to the ELSE branch */
	      tmp = lddAndRecur (ldd, rootE, d);
	      if (tmp != NULL) cuddRef (tmp);
	      Cudd_IterDerefBdd (CUDD, rootE);
	      if (tmp == NULL)
		{
		  Cudd_IterDerefBdd (CUDD, rootT);
		  Cudd_IterDerefBdd (CUDD, d);
		  return NULL;
		}
	      rootE = tmp;
	      Cudd_IterDerefBdd (CUDD, d);
	    }
	}
      
    }
  
      
  /* recursive step */

  t = lddTermReplaceRecur (ldd, fv, t1, t2, a, kmin, kmax, table);
  if (t == NULL)
    {
      Cudd_IterDerefBdd (CUDD, rootT);
      Cudd_IterDerefBdd (CUDD, rootE);
      return NULL;
    }
  cuddRef (t);
  
  e = lddTermReplaceRecur (ldd, fnv, t1, t2, a, kmin, kmax, table);
  if (e == NULL)
    {
      Cudd_IterDerefBdd (CUDD, rootT);
      Cudd_IterDerefBdd (CUDD, rootE);
      Cudd_IterDerefBdd (CUDD, t);
      return NULL;
    }
  cuddRef (e);

  /* Add rootT to the THEN branch of the result */
  if (rootT != DD_ONE(CUDD))
    {
      DdNode *tmp;
      tmp = lddAndRecur (ldd, rootT, t);
      if (tmp != NULL) cuddRef (tmp);
      Cudd_IterDerefBdd (CUDD, rootT);
      Cudd_IterDerefBdd (CUDD, t);
      if (tmp == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, rootE);
	  Cudd_IterDerefBdd (CUDD, e);
	  return NULL;
	}
      t = tmp; 
    }
  else
    /* rootT == DD_ONE(CUDD) */
    cuddDeref (rootT);
  
  /* Add rootE to the ELSE branch of the result */
  if (rootE != DD_ONE(CUDD))
    {
      DdNode *tmp;
      tmp = lddAndRecur (ldd, rootE, e);
      if (tmp != NULL) cuddRef (tmp);
      Cudd_IterDerefBdd (CUDD, rootE);
      Cudd_IterDerefBdd (CUDD, e);
      if (tmp == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, t);
	  return NULL;
	}
      e = tmp; 
    }
  else
    /* rootE == DD_ONE(CUDD) */
    cuddDeref (rootE);


  /* res = OR (t, e) */
  res = lddAndRecur (ldd, Cudd_Not (t), Cudd_Not (e));
  if (res != NULL)
    {
      res = Cudd_Not (res);
      cuddRef (res);
    }
  Cudd_IterDerefBdd (CUDD, t);
  Cudd_IterDerefBdd (CUDD, e);
  if (res == NULL) return NULL;
  
  /* update cache */
  if (F->ref != 1)
    {
      ptrint fanout = (ptrint) F->ref;
      cuddSatDec (fanout);
      if (!cuddHashTableInsert1 (table, f, res, fanout))
	{
	  Cudd_IterDerefBdd (CUDD, res);
	  return NULL;
	}
    }

  cuddDeref (res);
  return res;
    
}


LddNode *
lddTermMinmaxApproxRecur (LddManager *ldd,
			      LddNode *f)
{
  DdManager * manager;
  DdNode *F, *fv, *fnv, *h, *k;
  DdNode *one, *zero, *r, *t;

  unsigned int minIndex, maxIndex;
  lincons_t minCons, maxCons;
  
  manager = CUDD;
  statLine(manager);
  one = DD_ONE(manager);
  zero = Cudd_Not (one);

  F = Cudd_Regular(f);


  /* Terminal cases. */
  if (F == one) return f;


  /* Check cache. */
  if (F->ref != 1) {
    r = cuddCacheLookup1(manager, (DD_CTFP1)Ldd_TermMinmaxApprox, f);
    if (r != NULL) return(r);
  }
  
  minIndex = F->index;

  fv = Cudd_NotCond (cuddT(F), F != f);
  fnv = Cudd_NotCond (cuddE(F), F != f);

  /** 
   * Get the constraint of the root node 
   */
  minCons = ldd->ddVars [minIndex];

  maxCons = minCons;
  maxIndex = minIndex;

  k = fv;
  cuddRef (k);
  
  h = fnv;
  
  while (!cuddIsConstant (Cudd_Regular (h)))
    {
      lincons_t hCons;
      DdNode *H, *tmp, *ht;
      
      H = Cudd_Regular (h);
      hCons = ldd->ddVars [H->index];
      
      if (!THEORY->is_stronger_cons (maxCons, hCons)) break;
      
      maxCons = hCons;
      maxIndex = H->index;
      
      /* THEN branch of h */
      ht = Cudd_NotCond (cuddT(H), H != h);

      /* k = OR (k, ht) */
      tmp = lddAndRecur (ldd, Cudd_Not (k), Cudd_Not (ht));
      if (tmp != NULL) cuddRef (tmp);
      Cudd_IterDerefBdd (CUDD, k);
      if (tmp == NULL) return NULL;
      k = Cudd_Not (tmp);

      h = Cudd_NotCond (cuddE (H), H != h);
    }
  
  if (h != zero)
    {
      DdNode *tmp;
      
      tmp = lddAndRecur (ldd, Cudd_Not (k), Cudd_Not (h));
      if (tmp != NULL) cuddRef (tmp);
      Cudd_IterDerefBdd (CUDD, k);
      if (tmp == NULL) return NULL;
      k = Cudd_Not (tmp);
    }
  
  t = lddTermMinmaxApproxRecur (ldd, k);
  if (t != NULL) cuddRef (t);
  Cudd_IterDerefBdd (CUDD, k);
  if (t == NULL) return NULL;
  k = t; t = NULL;
  

  if (h != zero)
    {
      r = k; k = NULL;
    }
  
  else
    {
      DdNode *root;
      
      root = Cudd_bddIthVar (CUDD, maxIndex);
      if (root == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, k);
	  return NULL;
	}
      cuddRef (root);
      
      r = lddIteRecur (ldd, root, k, zero);
      if (r != NULL) cuddRef (r);
      Cudd_IterDerefBdd (CUDD, root);
      Cudd_IterDerefBdd (CUDD, k);
      k = NULL;
      if (r == NULL) return NULL;
    }
  
  /* k is dead here, r is referenced once */

  if (fv == zero)
    {
      DdNode *root, *tmp;
      
      root = Cudd_bddIthVar (CUDD, minIndex);
      if (root == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, r);
	  return NULL;
	}
      cuddRef (root);
      
      tmp = lddIteRecur (ldd, root, zero, r);
      if (tmp != NULL) cuddRef (tmp);
      Cudd_IterDerefBdd (CUDD, r);
      Cudd_IterDerefBdd (CUDD, root);
      if (tmp == NULL) return NULL;
      r = tmp;
    }
  
  if (F->ref != 1)
    cuddCacheInsert1(CUDD, (DD_CTFP1)Ldd_TermMinmaxApprox, f, r);
  
  cuddDeref (r);
  return r;
}


LddNode * 
lddTermConstrainRecur (LddManager *ldd, LddNode *f, linterm_t t1, 
			  linterm_t t2, constant_t k, DdLocalCache *cache)
{
  DdNode *F, *t, *e, *r;
  lincons_t fCons;
  linterm_t fTerm;
  constant_t fCst;
  DdNode *zero, *tmp;
  
  
  
  F = Cudd_Regular (f);
  zero = Cudd_Not (DD_ONE(CUDD));
  
  if (F == DD_ONE(CUDD)) return f;

  if (F->ref != 1 && (r = cuddLocalCacheLookup (cache, &f)) != NULL)
    return r;

  fCons = ldd->ddVars [F->index];
  fTerm = THEORY->get_term (fCons);

  t = Cudd_NotCond (cuddT(F), f != F);
  cuddRef (t);
  
  if (t != zero && THEORY->term_equals (fTerm, t2))
    {
      DdNode *d;
      constant_t nCst;
      lincons_t nCons;
      
      /* create  t1 <= fCst + k */
      fCst = THEORY->get_constant (fCons);
      nCst = THEORY->add_cst (fCst, k);
      nCons = THEORY->create_cons (THEORY->dup_term (t1),
				   THEORY->is_strict (fCons),
				   nCst);
      d = Ldd_FromCons (ldd, nCons);
      THEORY->destroy_lincons (nCons);
      nCons = NULL;
      if (d == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, t);
	  return NULL;
	}
      cuddRef (d);
      
      /* add new constraint to t */
      tmp = lddAndRecur (ldd, t, d);
      if (tmp != NULL) cuddRef (tmp);
      Cudd_IterDerefBdd (CUDD, t);
      Cudd_IterDerefBdd (CUDD, d);
      if (tmp == NULL) return NULL;
      t = tmp;
    }
  
  /* recurse on the THEN branch */
  tmp = lddTermConstrainRecur (ldd, t, t1, t2, k, cache);
  if (tmp != NULL) cuddRef (tmp);
  Cudd_IterDerefBdd (CUDD, t);
  if (tmp == NULL) return NULL;
  t = tmp;
  
  e = Cudd_NotCond (cuddE(F), f != F);
  cuddRef (e);

  if (e != zero && THEORY->term_equals (fTerm, t1))
    {
      DdNode *d;
      constant_t tmpCst, nCst;
      lincons_t nCons;
     
      /* Create  t2 <= fCst - k */
      fCst = THEORY->get_constant (fCons);
      tmpCst = THEORY->negate_cst (k);
      nCst = THEORY->add_cst (tmpCst, fCst);
      THEORY->destroy_cst (tmpCst);
      tmpCst = NULL;
      
      nCons = THEORY->create_cons 
	(THEORY->dup_term (t2), THEORY->is_strict (fCons), nCst);
      d = Ldd_FromCons (ldd, nCons);
      THEORY->destroy_lincons (nCons);
      nCons = NULL;
      if (d == NULL)
	{
	  Cudd_IterDerefBdd (CUDD, t);
	  Cudd_IterDerefBdd (CUDD, e);
	  return NULL;
	}
      cuddRef (d);
      d = Cudd_Not (d);

      /* add new constraint to e */
      tmp = lddAndRecur (ldd, e, d);
      if (tmp != NULL) cuddRef (tmp);
      Cudd_IterDerefBdd (CUDD, e);
      Cudd_IterDerefBdd (CUDD, d);
      if (tmp == NULL) 
	{
	  Cudd_IterDerefBdd (CUDD, t);
	  return NULL;
	}
      e = tmp;
      
    }

  /* recurse on the ELSE branch */
  tmp = lddTermConstrainRecur (ldd, e, t1, t2, k, cache);
  if (tmp != NULL) cuddRef (tmp);
  Cudd_IterDerefBdd (CUDD, e);
  if (tmp == NULL)
    {
      Cudd_IterDerefBdd (CUDD, t);
      return NULL;
    }
  e = tmp;
  
  
  /* construct the result */
  r = lddIteRecur (ldd, CUDD->vars[F->index], t, e);

  
  if (r != NULL) cuddRef (r);
  Cudd_IterDerefBdd (CUDD, t);
  Cudd_IterDerefBdd (CUDD, e);
  if (r == NULL) return NULL;

  if (F->ref != 1)
    cuddLocalCacheInsert (cache, &f, r);

  cuddDeref (r);
  return r;
  
}


