(define (domain example)
  (:requirements :typing :strips :equality :probabilistic-effects :conditional-effects :rewards)
  (:predicates (p) (q) (r) (z))
  (:action action1
    :precondition (p)
    :effect (probabilistic 
              0.5 (when (p) 
                (probabilistic 
                  0.5 (q) 
                  0.2 (when (q)
                        (not (p)))
                )
              )              
              0.5 (z)
            )
  )
  
  (:action action2
    :precondition (p)
    :effect (probabilistic 
              0.2 (when (p) 
                (probabilistic 
                  0.3 (r) 
                  0.7 (when (r)
                        (not (p)))
                )
              )              
              0.8 (z)
            )
  )
)

(define (problem p01)
  (:domain example)
  (:init (p))
  (:goal (not (p))))