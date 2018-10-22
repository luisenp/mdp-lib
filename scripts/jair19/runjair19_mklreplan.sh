nsims=500
maxt=-1

swarm_flags="--partition=longq --time=10-01:00:00"

# -------------------- RUNNING RACETRACK DOMAIN -------------------- #
save_dir="/home/lpineda/results_jair/mklreplan_racetrack"        

tracks=("roads-huge/map3" \
        "roads-huge/map4" \
        "roads-huge/map5")

models=("best-det-racetrack-greedy" "best-m02-racetrack-greedy")
for ((id_track = 0; id_track < ${#tracks[@]}; id_track++)); do
  track=${tracks[$id_track]}
  output_file=${save_dir}/${track}.fullmodel
  # This runs full-model planning
  sbatch ${swarm_flags} --output=${output_file} \
    ./run_testreduced_racetrack.sh \
      ${track} "0" ${nsims} "no-mkl" ${maxt} "--use-full --heuristic=aodet" 1000
  # This loops different reduced models and values of k
  for ((id_model = 0; id_model < ${#models[@]}; id_model++)); do
    model=${models[$id_model]}
    for k_reduced in `seq 0 3`; do
      output_file=${save_dir}/${track}.${model:5:3}.k${k_reduced}
      sbatch ${swarm_flags} --output=${output_file} \
        ./run_testreduced_racetrack.sh ${track} ${k_reduced} ${nsims} ${model} ${maxt} "--heuristic=aodet" 1000
    done
  done
done

# -------------------- RUNNING SAILING DOMAIN -------------------- #
# save_dir="/home/lpineda/results_jair/mklreplan_sailing"        
# models=("best-det-sailing-greedy" "best-m02-sailing-greedy")
# size=40
# output_file=${save_dir}/s${size}.fullmodel
# # This runs full-model planning
# sbatch ${swarm_flags} --output=${output_file} \
#   ./run_testreduced_sailing.sh \
#     ${size} "0" ${nsims} "no-mkl" ${maxt} " --heuristic=aodet --use-full" 1000
# # This loops different reduced models and values of k
# for ((id_model = 0; id_model < ${#models[@]}; id_model++)); do
#   model=${models[$id_model]}
#   for k_reduced in `seq 0 1`; do
#     output_file=${save_dir}/s${size}.${model:5:3}.k${k_reduced}
#     sbatch ${swarm_flags} --output=${output_file} \
#       ./run_testreduced_sailing.sh ${size} ${k_reduced} ${nsims} ${model} ${maxt} "--heuristic=aodet" 1000
#   done
# done
