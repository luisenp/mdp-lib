(define (domain pitchcatch)
(:requirements :adl :probabilistic-effects)
(:types balltype bit difficulty)
(:constants  T0 - balltype Easy Medium Hard - difficulty B0 B1 - bit)
(:predicates
  (alive)
  (ball-in-the-air-of-type ?t - balltype)
  (catchable ?t - balltype ?B0 - bit)
  (bit-on ?b - bit)
  (bit-off ?b - bit)
  (thrown)
  (not-thrown)
  (caught ?t - balltype)
  (deposited ?t - balltype)
  (similar ?t1 ?t2 - balltype)
  (difficulty-setting ?b - bit ?d - difficulty)
)

(:action catch
 :parameters (?T ?Tball - balltype ?B0 - bit)
 :precondition (and (thrown)
                    (ball-in-the-air-of-type ?Tball)
                    (catchable ?T ?B0 ))
 :effect (and (not (thrown))
              (not-thrown)
              (not (bit-on ?B0))
              (bit-off ?B0)
              (probabilistic 4/10
              (when (and 
                          (bit-on ?B0)
                          (= ?T ?Tball))
                          (caught ?Tball)))
              (probabilistic 1/10
              (when (and 
                          (bit-on ?B0)
                          (similar ?T ?Tball))
                          (caught ?Tball)))))
(:action deposit-ball
 :effect      (and (forall (?t1 - balltype) (not (ball-in-the-air-of-type ?t1)))
                   (forall (?t1 - balltype) (when (caught ?t1) (deposited ?t1)))))
(:action pass
 :precondition (thrown)
 :effect (and (not (thrown))
              (not-thrown)
              (forall (?t - balltype) (not (ball-in-the-air-of-type ?t)))
              (probabilistic 5/100 (not (alive)))))
(:action pitch
 :precondition (and (not-thrown)
                    (alive))
 :effect  (and (thrown)
               (not (not-thrown))
               (probabilistic 24/100 (ball-in-the-air-of-type T0))
               (forall (?B - bit)
                       (probabilistic
                           1/10 (and (bit-on ?B) (not (bit-off ?B)))))))
(:action set-bit
 :parameters (?B - bit)
 :precondition (not-thrown)
 :effect (and (bit-on ?B)
              (not (bit-off ?B))
              (probabilistic 5/100
                    (when (difficulty-setting ?B Easy)
                          (not (alive))))
              (probabilistic 1/10 
                    (when (difficulty-setting ?B Medium)
                          (not (alive))))
              (probabilistic 2/10
                    (when (difficulty-setting ?B Hard)
                          (not (alive))))))
)

(define (problem p02) 
(:domain pitchcatch)  
(:init (alive) 
       (not-thrown)
       (bit-on B0)
       (bit-on B1)
       (difficulty-setting B0 Easy)
       (difficulty-setting B1 Hard)
       (catchable T0 B1 )
)
(:goal (and 
            (deposited T0)
)))
