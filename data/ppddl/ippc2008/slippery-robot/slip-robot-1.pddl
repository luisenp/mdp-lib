(define (domain slip-robot)
  (:requirements :typing :strips :equality :probabilistic-effects :rewards)
  (:types location)
  (:predicates (vehicle-at ?loc - location)
	       (umbrella-in ?loc - location)
	       (road ?from - location ?to - location)
	       (has-umbrella))
   
  (:action pickumbrella
    :parameters (?loc - location)
    :precondition (and (not(has-umbrella)) (umbrella-in ?loc) (vehicle-at ?loc))
    :effect (has-umbrella))

(:action move-car-without-umbrella
    :parameters (?from ?to - location)
    :precondition (and (vehicle-at ?from) (road ?from ?to))
    :effect
      (probabilistic
        0.4 (and (vehicle-at ?to) (not(vehicle-at ?from)))
        0.6 (vehicle-at ?from))
  )
 (:action move-car-umbrella
    :parameters (?from ?to - location)
    :precondition (and (vehicle-at ?from) (road ?from ?to) (has-umbrella))
    :effect
      (probabilistic
        0.8 (and (vehicle-at ?to) (not(vehicle-at ?from)))
        0.2 (vehicle-at ?from))
  )
)

(define (problem slip-robot-1)
                   (:domain slip-robot)
                   (:objects l-1-1 l-1-2 l-1-3 l-2-1 l-2-2 l-2-3 l-3-1 l-3-2 l-3-3 - location)
                   (:init (vehicle-at l-1-1)(road l-1-1 l-1-2)(road l-1-2 l-1-3)(road l-1-1 l-2-1)(road l-1-2 l-2-2)(road l-2-1 l-1-2)(road l-2-2 l-1-3) (road l-2-1 l-3-1)(road l-3-1 l-2-2)(umbrella-in l-2-1)(umbrella-in l-2-2) (umbrella-in l-3-1)(umbrella-in l-1-1) (umbrella-in l-1-2)(umbrella-in l-1-3)(umbrella-in l-2-3)(umbrella-in l-3-2)(umbrella-in l-3-3))
                   (:goal (vehicle-at l-1-3)) (:goal-reward 100) (:metric maximize (reward)))

