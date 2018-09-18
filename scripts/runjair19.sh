nsims=100

tracks=("roads-huge/map3" \
        "roads-huge/map4" \
        "roads-huge/map5")

models=("best-det-racetrack-greedy" "best-m02-racetrack-greedy")

times=(50 100 200 400 800 1600 3200)

save_dir="/home/lpineda/results_jair/anytime_racetrack"        

for ((id_track = 0; id_track < ${#tracks[@]}; id_track++)); do
  track=${tracks[$id_track]}
  for ((id_time = 0; id_time < ${#times[@]}; id_time++)); do    
    maxt=${times[$id_time]}
    output_file=${save_dir}/${track}.fullmodel.t${maxt}
    sbatch --output=${output_file} \
      ./run_testreduced_racetrack.sh \
        ${track} "0" ${nsims} "no-mkl" ${maxt} "--use-full"
    for ((id_model = 0; id_model < ${#models[@]}; id_model++)); do
      model=${models[$id_model]}
      for k_reduced in `seq 0 2`; do
        maxt=${times[$id_time]}
        output_file=${save_dir}/${track}.${model:5:3}.k${k_reduced}.t${maxt}
        sbatch --output=${output_file} \
          ./run_testreduced_racetrack.sh ${track} ${k_reduced} ${nsims} ${model} ${maxt}
      done
    done  
  done
done
