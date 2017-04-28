(define (domain triangle-tire-60)
  (:requirements :typing :strips :equality :probabilistic-effects :rewards)
  (:types location)
  (:predicates (vehicle-at ?loc - location)
	       (spare-in ?loc - location)
	       (road ?from - location ?to - location)
	       (not-flattire) (hasspare))
  (:action move-car
    :parameters (?from - location ?to - location)
    :precondition (and (vehicle-at ?from) (road ?from ?to) (not-flattire))
    :effect (and (vehicle-at ?to) (not (vehicle-at ?from))
		 (probabilistic 0.4 (not (not-flattire)))))
  (:action loadtire
    :parameters (?loc - location)
    :precondition (and (vehicle-at ?loc) (spare-in ?loc))
    :effect (and (hasspare) (not (spare-in ?loc))))
  (:action changetire
    :precondition (hasspare)
    :effect (and (not (hasspare)) (not-flattire)))

)

(define (problem p01)
                   (:domain triangle-tire-60)
                   (:objects l-1-1 l-1-2 l-1-3 l-2-1 l-2-2 l-2-3 l-3-1 l-3-2 l-3-3 - location)
                   (:init (vehicle-at l-1-1)(road l-1-1 l-1-2)(road l-1-2 l-1-3)(road l-1-1 l-2-1)(road l-1-2 l-2-2)(road l-2-1 l-1-2)(road l-2-2 l-1-3)(spare-in l-2-1)(spare-in l-2-2)(road l-2-1 l-3-1)(road l-3-1 l-2-2)(spare-in l-3-1)(spare-in l-3-1)(not-flattire))
                   (:goal (vehicle-at l-1-3)) (:goal-reward 100) (:metric maximize (reward)))

