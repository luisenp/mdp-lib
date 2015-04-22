Minimize
  (0,0)_action_right + (0,0)_action_left + (0,0)_action_down
   + (0,0)_action_up + (1,0)_action_right + (1,0)_action_left
   + (1,0)_action_down + (1,0)_action_up + (0,3)_action_right
   + (0,3)_action_left + (0,3)_action_down + (0,3)_action_up
   + (0,1)_action_right + (0,1)_action_left + (0,1)_action_down
   + (0,1)_action_up + (0,2)_action_right + (0,2)_action_left
   + (0,2)_action_down + (0,2)_action_up + (3,1)_action_right
   + (3,1)_action_left + (3,1)_action_down + (3,1)_action_up
   + (3,0)_action_right + (3,0)_action_left + (3,0)_action_down
   + (3,0)_action_up + (2,3)_action_right + (2,3)_action_left
   + (2,3)_action_down + (2,3)_action_up + (2,0)_action_right
   + (2,0)_action_left + (2,0)_action_down + (2,0)_action_up
   + (3,3)_action_right + (3,3)_action_left + (3,3)_action_down
   + (3,3)_action_up + (1,2)_action_right + (1,2)_action_left
   + (1,2)_action_down + (1,2)_action_up + (1,1)_action_right
   + (1,1)_action_left + (1,1)_action_down + (1,1)_action_up
   + (2,2)_action_right + (2,2)_action_left + (2,2)_action_down
   + (2,2)_action_up + (2,1)_action_right + (2,1)_action_left
   + (2,1)_action_down + (2,1)_action_up
Subject To
 R0: 0.91 (0,0)_action_right + 0.19 (0,0)_action_left
   + 0.19 (0,0)_action_down + 0.91 (0,0)_action_up - 0.72 (1,0)_action_left
   - 0.09 (1,0)_action_down - 0.09 (1,0)_action_up
   - 0.09 (0,1)_action_right - 0.09 (0,1)_action_left
   - 0.72 (0,1)_action_down >= -0
 R1: - 0.72 (0,0)_action_right - 0.09 (0,0)_action_down
   - 0.09 (0,0)_action_up + 0.91 (1,0)_action_right
   + 0.91 (1,0)_action_left + 0.28 (1,0)_action_down + (1,0)_action_up
   - 0.72 (2,0)_action_left - 0.09 (2,0)_action_down - 0.09 (2,0)_action_up
   - 0.09 (1,1)_action_right - 0.09 (1,1)_action_left
   - 0.72 (1,1)_action_down >= -0
 R2: 0.91 (0,3)_action_right + 0.19 (0,3)_action_left
   + 0.91 (0,3)_action_down + 0.19 (0,3)_action_up
   - 0.09 (0,2)_action_right - 0.09 (0,2)_action_left
   - 0.72 (0,2)_action_up >= -0
 R3: - 0.09 (0,0)_action_right - 0.09 (0,0)_action_left
   - 0.72 (0,0)_action_up + (0,1)_action_right + 0.28 (0,1)_action_left
   + 0.91 (0,1)_action_down + 0.91 (0,1)_action_up
   - 0.09 (0,2)_action_right - 0.09 (0,2)_action_left
   - 0.72 (0,2)_action_down - 0.72 (1,1)_action_left
   - 0.09 (1,1)_action_down - 0.09 (1,1)_action_up >= -0
 R4: - 0.09 (0,3)_action_right - 0.09 (0,3)_action_left
   - 0.72 (0,3)_action_down - 0.09 (0,1)_action_right
   - 0.09 (0,1)_action_left - 0.72 (0,1)_action_up + (0,2)_action_right
   + 0.28 (0,2)_action_left + 0.91 (0,2)_action_down + 0.91 (0,2)_action_up
   - 0.72 (1,2)_action_left - 0.09 (1,2)_action_down - 0.09 (1,2)_action_up
   >= -0
 R5: 0.28 (3,1)_action_right + (3,1)_action_left + 0.91 (3,1)_action_down
   + 0.91 (3,1)_action_up - 0.09 (3,0)_action_right
   - 0.09 (3,0)_action_left - 0.72 (3,0)_action_up
   - 0.72 (2,1)_action_right - 0.09 (2,1)_action_down
   - 0.09 (2,1)_action_up >= -0
 R6: (-2,-2)_action_right + (-2,-2)_action_left + (-2,-2)_action_down
   + (-2,-2)_action_up >= 0.1
 R7: - 0.09 (3,1)_action_right - 0.09 (3,1)_action_left
   - 0.72 (3,1)_action_down + 0.19 (3,0)_action_right
   + 0.91 (3,0)_action_left + 0.19 (3,0)_action_down + 0.91 (3,0)_action_up
   - 0.72 (2,0)_action_right - 0.09 (2,0)_action_down
   - 0.09 (2,0)_action_up >= -0
 R8: 0.91 (2,3)_action_right + 0.91 (2,3)_action_left + (2,3)_action_down
   + 0.28 (2,3)_action_up - 0.72 (3,3)_action_left - 0.09 (3,3)_action_down
   - 0.09 (3,3)_action_up - 0.09 (2,2)_action_right
   - 0.09 (2,2)_action_left - 0.72 (2,2)_action_up >= -0
 R9: 0.1 (-1,-1)_action_right + 0.1 (-1,-1)_action_left
   + 0.1 (-1,-1)_action_down + 0.1 (-1,-1)_action_up
   - 0.9 (1,3)_action_right - 0.9 (1,3)_action_left - 0.9 (1,3)_action_down
   - 0.9 (1,3)_action_up - 0.9 (3,2)_action_right - 0.9 (3,2)_action_left
   - 0.9 (3,2)_action_down - 0.9 (3,2)_action_up >= -0
 R10: - 0.72 (1,0)_action_right - 0.09 (1,0)_action_down
   - 0.09 (1,0)_action_up - 0.72 (3,0)_action_left - 0.09 (3,0)_action_down
   - 0.09 (3,0)_action_up + 0.91 (2,0)_action_right
   + 0.91 (2,0)_action_left + 0.28 (2,0)_action_down + (2,0)_action_up
   - 0.09 (2,1)_action_right - 0.09 (2,1)_action_left
   - 0.72 (2,1)_action_down >= -0
 R11: - 0.72 (0,3)_action_right - 0.09 (0,3)_action_down
   - 0.09 (0,3)_action_up - 0.72 (2,3)_action_left - 0.09 (2,3)_action_down
   - 0.09 (2,3)_action_up + (1,3)_action_right + (1,3)_action_left
   + (1,3)_action_down + (1,3)_action_up - 0.09 (1,2)_action_right
   - 0.09 (1,2)_action_left - 0.72 (1,2)_action_up >= -0
 R12: - 0.9 (-2,-2)_action_right - 0.9 (-2,-2)_action_left
   - 0.9 (-2,-2)_action_down - 0.9 (-2,-2)_action_up
   - 0.72 (2,3)_action_right - 0.09 (2,3)_action_down
   - 0.09 (2,3)_action_up + 0.19 (3,3)_action_right
   + 0.91 (3,3)_action_left + 0.91 (3,3)_action_down + 0.19 (3,3)_action_up
   >= -0
 R13: - 0.72 (0,2)_action_right - 0.09 (0,2)_action_down
   - 0.09 (0,2)_action_up + (1,2)_action_right + (1,2)_action_left
   + (1,2)_action_down + (1,2)_action_up - 0.09 (1,1)_action_right
   - 0.09 (1,1)_action_left - 0.72 (1,1)_action_up - 0.72 (2,2)_action_left
   - 0.09 (2,2)_action_down - 0.09 (2,2)_action_up >= -0
 R14: - 0.09 (3,1)_action_right - 0.09 (3,1)_action_left
   - 0.72 (3,1)_action_up - 0.09 (3,3)_action_right
   - 0.09 (3,3)_action_left - 0.72 (3,3)_action_down + (3,2)_action_right
   + (3,2)_action_left + (3,2)_action_down + (3,2)_action_up
   - 0.72 (2,2)_action_right - 0.09 (2,2)_action_down
   - 0.09 (2,2)_action_up >= -0
 R15: - 0.09 (1,0)_action_right - 0.09 (1,0)_action_left
   - 0.72 (1,0)_action_up - 0.72 (0,1)_action_right
   - 0.09 (0,1)_action_down - 0.09 (0,1)_action_up
   - 0.09 (1,2)_action_right - 0.09 (1,2)_action_left
   - 0.72 (1,2)_action_down + (1,1)_action_right + (1,1)_action_left
   + (1,1)_action_down + (1,1)_action_up - 0.72 (2,1)_action_left
   - 0.09 (2,1)_action_down - 0.09 (2,1)_action_up >= -0
 R16: - 0.09 (2,3)_action_right - 0.09 (2,3)_action_left
   - 0.72 (2,3)_action_down - 0.72 (1,2)_action_right
   - 0.09 (1,2)_action_down - 0.09 (1,2)_action_up + (2,2)_action_right
   + (2,2)_action_left + (2,2)_action_down + (2,2)_action_up
   - 0.09 (2,1)_action_right - 0.09 (2,1)_action_left
   - 0.72 (2,1)_action_up >= -0
 R17: - 0.72 (3,1)_action_left - 0.09 (3,1)_action_down
   - 0.09 (3,1)_action_up - 0.09 (2,0)_action_right
   - 0.09 (2,0)_action_left - 0.72 (2,0)_action_up
   - 0.72 (1,1)_action_right - 0.09 (1,1)_action_down
   - 0.09 (1,1)_action_up - 0.09 (2,2)_action_right
   - 0.09 (2,2)_action_left - 0.72 (2,2)_action_down + (2,1)_action_right
   + (2,1)_action_left + (2,1)_action_down + (2,1)_action_up >= -0
Bounds
End
