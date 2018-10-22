nsims=500

times=(50 100 200 400 800 1600 3200 6400)

swarm_flags="--partition=longq --time=10-01:00:00"

# -------------------- RUNNING RACETRACK DOMAIN -------------------- #
save_dir="/home/lpineda/results_jair/anytime_racetrack"        

tracks=("roads-huge/map3" \
        "roads-huge/map4" \
        "roads-huge/map5")

models=("best-det-racetrack-greedy" "best-m02-racetrack-greedy")
for ((id_track = 0; id_track < ${#tracks[@]}; id_track++)); do
  track=${tracks[$id_track]}
  for ((id_time = 0; id_time < ${#times[@]}; id_time++)); do
    maxt=${times[$id_time]}
    output_file=${save_dir}/${track}.fullmodel.t${maxt}
    # This runs full-model planning
    sbatch ${swarm_flags} --output=${output_file} \
      ./run_testreduced_racetrack.sh \
        ${track} "0" ${nsims} "no-mkl" ${maxt} "--use-lrtdp --anytime --heuristic=aodet--use-full" 0
    # This loops different reduced models and values of k
    for ((id_model = 0; id_model < ${#models[@]}; id_model++)); do
      model=${models[$id_model]}
      for k_reduced in `seq 0 3`; do
        maxt=${times[$id_time]}
        output_file=${save_dir}/${track}.${model:5:3}.k${k_reduced}.t${maxt}
        sbatch ${swarm_flags} --output=${output_file} \
          ./run_testreduced_racetrack.sh ${track} ${k_reduced} ${nsims} ${model} ${maxt} "--use-lrtdp --anytime --heuristic=aodet" 0
      done
      # Run a planner that increases k when current state is already solved
      # (Only starting with k=0)
      maxt=${times[$id_time]}
      output_file=${save_dir}/${track}.${model:5:3}.kincrease.t${maxt}
      sbatch ${swarm_flags} --output=${output_file} \
        ./run_testreduced_racetrack.sh \
          ${track} "0" ${nsims} ${model} ${maxt} "--use-lrtdp --anytime --heuristic=aodet --increase-k" 0
          
    done  
  done
done

# -------------------- RUNNING SAILING DOMAIN -------------------- #
# save_dir="/home/lpineda/results_jair/anytime_sailing"        
# models=("best-det-sailing-greedy" "best-m02-sailing-greedy")
# size=40
# for ((id_time = 0; id_time < ${#times[@]}; id_time++)); do
#   maxt=${times[$id_time]}
#   output_file=${save_dir}/s${size}.fullmodel.t${maxt}
#   # This runs full-model planning
#   sbatch ${swarm_flags} --output=${output_file} \
#     ./run_testreduced_sailing.sh \
#       ${size} "0" ${nsims} "no-mkl" ${maxt} "--use-lrtdp --anytime --use-full --heuristic=aodet " 0
#   # This loops different reduced models and values of k
#   for ((id_model = 0; id_model < ${#models[@]}; id_model++)); do
#     model=${models[$id_model]}
#     for k_reduced in `seq 0 3`; do
#       maxt=${times[$id_time]}
#       output_file=${save_dir}/s${size}.${model:5:3}.k${k_reduced}.t${maxt}
#       sbatch ${swarm_flags} --output=${output_file} \
#         ./run_testreduced_sailing.sh ${size} ${k_reduced} ${nsims} ${model} ${maxt} "--use-lrtdp --anytime --heuristic=aodet " 0
#     done
#     # Run a planner that increases k when current state is already solved
#     # (Only starting with k=0)
#     maxt=${times[$id_time]}
#     output_file=${save_dir}/s${size}.${model:5:3}.kincrease.t${maxt}
#     sbatch ${swarm_flags} --output=${output_file} \
#       ./run_testreduced_sailing.sh \
#         ${size} "0" ${nsims} ${model} ${maxt} "--use-lrtdp --anytime --increase-k --heuristic=aodet " 0
#   done  
# done
