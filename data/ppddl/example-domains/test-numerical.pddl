(define (domain test-numerical)
  (:requirements :typing :equality :fluents)
  (:predicates (has-0) (has-1) (has-2))
  (:functions (score))
  (:action increase-1
    :parameters ()
    :precondition (has-0)
    :effect (has-1))
  (:action increase-2
    :parameters ()
    :precondition (has-1)
    :effect (has-2))
  (:action increase-score
    :parameters ()
    :precondition (has-1)
    :effect (increase score 1))
)


(define (problem p01)
  (:domain test-numerical)
  (:init (has-0) (= score 0))
  (:goal (and (has-2) (> (score) 0)))
)

