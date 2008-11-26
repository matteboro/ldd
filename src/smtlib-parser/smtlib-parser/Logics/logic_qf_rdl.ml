(*
This file is part of the SMT-LIB parser
Copyright (C) 2004-2005,2007-2008
              The University of Iowa

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*)

(** QF_RDL logic signature definitions. *)
(* Class extending logic_smt.
 *
 * Class methods (defined in an initializer):
 *  - theory_sorts lists all non-Boolean sorts of the logic
 *     This should be appended to the logic_smt values.
 *  - numeral_sort is the type that bare numerals such as "5" are assigned
 *     This also included bitvector numerals.  It also contains a list of 
 *     default indices for these numerals, possibly empty.
 *  - theory_funs are functions, predicates, relations, and constants defined
 *     in the theory.  "=", "distinct", and "ite/if_then_else" are 
 *     automatically defined for all sorts and should not be included here.
 *     This should be appended to the logic_smt values.
*)

open Types

let boolean = Sort "Bool"
let real = Sort "Real"

(** {6 Classes} *)

(***************************************************************)
class logic_QF_RDL = object (self)
  inherit Logic_smt.logic_smt

  initializer
  theory_sorts <- List.rev_append theory_sorts [real];
  numeral_sort <- real,[];
  theory_funs <- List.rev_append theory_funs [
    Fun(Sym "~",[real],real);
    Fun(Sym "-",[real],real);
    Fun(Sym "+",[real;real],real);
    Fun(Sym "-",[real;real],real);
    Fun(Sym "*",[real;real],real);
    Fun(Sym "/",[real;real],real);
    Fun(Sym "<=",[real;real],boolean);
    Fun(Sym "<",[real;real],boolean);
    Fun(Sym ">=",[real;real],boolean);
    Fun(Sym ">",[real;real],boolean)
    ]
end