# Habrok RUG HPC cluster module load:
# Usage source habrok_module_load.sh
# before building.
module load GLFW/3.3.8-GCCcore-11.3.0
module load CMake/3.29.3-GCCcore-13.3.0
module load CUDA/12.6.0 

case "$PS1" in
    "(nbody) "*) ;;
    *) PS1="(nbody) $PS1" ;;
esac