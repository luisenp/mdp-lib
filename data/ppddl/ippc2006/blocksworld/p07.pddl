(define (problem bw_10_11511)
  (:domain blocks-domain)
  (:objects b1 b2 b3 b4 b5 b6 b7 b8 b9 b10 - block)
  (:init (emptyhand) (on-table b1) (on b2 b5) (on b3 b8) (on-table b4) (on b5 b3) (on b6 b2) (on b7 b1) (on b8 b10) (on-table b9) (on b10 b9) (clear b4) (clear b6) (clear b7))
  (:goal (and (emptyhand) (on-table b1) (on b2 b4) (on b3 b10) (on b4 b9) (on b5 b8) (on b6 b5) (on b7 b2) (on b8 b3) (on b9 b1) (on b10 b7) (clear b6)))
)
