nsims=500

tracks=("roads-huge/map3" \
        "roads-huge/map4" \
        "roads-huge/map5")

models=("best-det-racetrack-greedy" "best-m02-racetrack-greedy")

times=(50 100 200 400 800 1600 3200 6400)

save_dir="/home/lpineda/results_jair/anytime_racetrack"        

for ((id_track = 0; id_track < ${#tracks[@]}; id_track++)); do
  track=${tracks[$id_track]}
  for ((id_time = 0; id_time < ${#times[@]}; id_time++)); do    
    maxt=${times[$id_time]}
    output_file=${save_dir}/${track}.fullmodel.t${maxt}
    # This runs full-model planning
    sbatch --output=${output_file} \
      ./run_testreduced_racetrack.sh \
        ${track} "0" ${nsims} "no-mkl" ${maxt} "--use-full"
    # This loops different reduced models and values of k
    for ((id_model = 0; id_model < ${#models[@]}; id_model++)); do
      model=${models[$id_model]}
      for k_reduced in `seq 0 3`; do
        maxt=${times[$id_time]}
        output_file=${save_dir}/${track}.${model:5:3}.k${k_reduced}.t${maxt}
        sbatch --output=${output_file} \
          ./run_testreduced_racetrack.sh ${track} ${k_reduced} ${nsims} ${model} ${maxt}
      done
      # Run a planner that increases k when current state is already solved
      # (Only starting with k=1)
      maxt=${times[$id_time]}
      output_file=${save_dir}/${track}.${model:5:3}.kincrease.t${maxt}
      sbatch --output=${output_file} \
        ./run_testreduced_racetrack.sh \
          ${track} "1" ${nsims} ${model} ${maxt} "--increase_k"
          
    done  
  done
done
