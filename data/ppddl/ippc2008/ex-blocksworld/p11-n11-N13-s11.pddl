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

(define (problem ex_bw_13_p11)
  (:domain exploding-blocksworld)
  (:objects b1 b2 b3 b4 b5 b6 b7 b8 b9 b10 b11 b12 b13 - block)
  (:init (emptyhand) (on-table b1) (on b2 b12) (on b3 b13) (on b4 b5) (on b5 b7) (on b6 b11) (on-table b7) (on-table b8) (on b9 b2) (on b10 b9) (on-table b11) (on b12 b4) (on b13 b8) (clear b1) (clear b3) (clear b6) (clear b10) (no-detonated b1) (no-destroyed b1) (no-detonated b2) (no-destroyed b2) (no-detonated b3) (no-destroyed b3) (no-detonated b4) (no-destroyed b4) (no-detonated b5) (no-destroyed b5) (no-detonated b6) (no-destroyed b6) (no-detonated b7) (no-destroyed b7) (no-detonated b8) (no-destroyed b8) (no-detonated b9) (no-destroyed b9) (no-detonated b10) (no-destroyed b10) (no-detonated b11) (no-destroyed b11) (no-detonated b12) (no-destroyed b12) (no-detonated b13) (no-destroyed b13) (no-destroyed-table))
  (:goal (and (on b2 b3) (on b3 b6) (on b4 b1) (on-table b5) (on-table b6) (on b7 b8) (on b8 b10) (on b9 b12) (on b10 b11) (on b11 b9) (on b12 b4)  )
)
  (:goal-reward 1)
  (:metric maximize (reward))
)
