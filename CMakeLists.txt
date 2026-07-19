cmake_minimum_required(VERSION 3.18)
project(KriptoWorkspace)

# CGBN is header-only, no need to add_subdirectory

# Add our subprojects
add_subdirectory(miller_rabin_searcher)
add_subdirectory(trial_division_gpu_searcher)
add_subdirectory(rsa_cracker_experiment)
