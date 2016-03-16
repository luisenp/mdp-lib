(define (domain example)
  (:requirements :typing :strips :equality :probabilistic-effects :conditional-effects :rewards)
  (:predicates (p) (q) (r) )
  (:action action1
    :precondition (p)
    :effect (and 
              (when (p) (and (r) (q))) 
              (when (q) (and (not (r)) (not (p)))))
  )
)

(define (problem p01)
  (:domain example)
  (:init (p))
  (:goal (not (p))))

