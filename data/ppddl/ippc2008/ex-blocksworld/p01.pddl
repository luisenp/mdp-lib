;; Authors: Michael Littman and David Weissman
;; Modified by: Blai Bonet

;; Comment: Good plans are those that avoid putting blocks on table since the probability of detonation is higher

(define (domain exploding-blocksworld)
  (:requirements :typing :conditional-effects :probabilistic-effects :equality :rewards)
  (:types block)
  (:predicates (on ?b1 ?b2 - block) (on-table ?b - block) (clear ?b - block) (holding ?b - block) (emptyhand) (no-detonated ?b - block) (no-destroyed ?b - block) (no-destroyed-table))

  (:action pick-up
   :parameters (?b1 ?b2 - block)
   :precondition (and (emptyhand) (clear ?b1) (on ?b1 ?b2) (no-destroyed ?b1))
   :effect (and (holding ?b1) (clear ?b2) (not (emptyhand)) (not (on ?b1 ?b2)))
  )
  (:action pick-up-from-table
   :parameters (?b - block)
   :precondition (and (emptyhand) (clear ?b) (on-table ?b) (no-destroyed ?b))
   :effect (and (holding ?b) (not (emptyhand)) (not (on-table ?b)))
  )
  (:action put-down
   :parameters (?b - block)
   :precondition (and (holding ?b) (no-destroyed-table))
   :effect (and (emptyhand) (on-table ?b) (not (holding ?b))
                (probabilistic 2/5 (when (no-detonated ?b) (and (not (no-destroyed-table)) (not (no-detonated ?b))))))
  )
  (:action put-on-block
   :parameters (?b1 ?b2 - block)   
   ;; Remove (not (= ?b1 ?b2)) from precondition (not in original)
   :precondition (and (holding ?b1) (clear ?b2) (no-destroyed ?b2) (not (= ?b1 ?b2)))
   :effect (and (emptyhand) (on ?b1 ?b2) (not (holding ?b1)) (not (clear ?b2))
                (probabilistic 1/10 (when (no-detonated ?b1) (and (not (no-destroyed ?b2)) (not (no-detonated ?b1))))))
  )
)

(define (problem p01)
  (:domain exploding-blocksworld)
  (:objects b1 b2 b3 b4 b5 - block)
  (:init (emptyhand) (on b1 b4) (on-table b2) (on b3 b2) (on b4 b5) (on-table b5) (clear b1) (clear b3) (no-detonated b1) (no-destroyed b1) (no-detonated b2) (no-destroyed b2) (no-detonated b3) (no-destroyed b3) (no-detonated b4) (no-destroyed b4) (no-detonated b5) (no-destroyed b5) (no-destroyed-table))
  (:goal (and (on b2 b4) (on-table b4)  )
)
  (:goal-reward 1)
  (:metric maximize (reward))
)
