# Habrok RUG HPC cluster module load:
# Usage source habrok_module_load.sh
# before building.
module load GLFW/3.3.8-GCCcore-11.3.0
module load CMake/3.31.3-GCCcore-14.2.0
module load CUDA/13.2.0 

case "$PS1" in
    "(nbody) "*) ;;
    *) PS1="(nbody) $PS1" ;;
esac