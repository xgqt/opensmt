(set-logic QF_ALIA)
(set-info :status unsat)
(declare-fun a () (Array Int Int))
(assert (distinct 0 (select (store (store a 3 0) 0 0) 3)))
(check-sat)
