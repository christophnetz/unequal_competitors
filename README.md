# unequal_competitors

Developed by [Christoph Netz](https://www.rug.nl/staff/c.f.g.netz/) and [Aparajitha Ramesh](https://www.rug.nl/staff/a.ramesh/) at the Groningen Institute for Evolutionary Life Science, at the University of Groningen.

This repository contains three simulation programs written in C++ used to study the ideal free distribution (IFD) of unequal competitors. 
(1) `ifd_finder_fewpatches` models the redistribution of foragers over a few patches starting from two different initial distributions.
(2) `ifd_finder_grid` similarly models the redistribution of foragers over a larger grid of patches, starting from a random distribution of foragers acros patches. 
(3) `ifd_evol` contains model (2) at the ecological level, adding the capacity for landscape change, and allowing the evolution of competitive abilities over successive generations.


## Organization

Each simulation program is contained in a separate folder. Analysis and Visualization code for publication purposes is deposited in the `R` folder.
Each project folder contains the *.sln file, an empty results folder and a main folder holding the source code and batch files used for model runs.
Visual Studio users can simply open the *.sln file, make a build and then either run the simulation from within or execute the batch files to 
replicate the simulation data used for publication.

In the following a generic overview of the source code files shared between the three models

## Key files and code

- `rnd.hpp`, `rnd.cpp` and `rndutils.hpp` Random number generation and custom distributions (`mutable_discrete_distribution`).

- `parameter.h` and `parameter.cpp` 

    - Contains the parameter structure and a parser for parameters for input from command line (in `cmd_line.h`), default values for parameters and streaming of parameters to file.


- `main.cpp` Reads in the parameters and runs the simulation specified in `simulation.hpp`.

- `simulation.hpp` 
    
    - Defines the main simulation function that considers an indnvidual-based model of foragers spread across a number of patches offering resources. Individuals may differ in their competitive ability, 
      causing differences in intake rates obtained on a patch. Patches vary in resource densities. Individuals are randomly selected for movement and in doing so choose the patch currently offering the highest intake rate. 
      When no individual can improve its intake rate by moving to another patch, the ideal free distribution is said to be reached. The initialization of individuals, presence of competitive types and resource densities is different for the three simulation programs. 
      Only ifd_evol considers the evolution of competitive ability and therefore entails an explicit time structure of within-generation movement and foraging events, and between-generation reproduction. This project also uses parallel-threading and cached draws from distributions.
      Output is written into the `results` folder.

- `cached.hpp`

   - Contained only in ifd_evol. Describes the classes storing and refilling the caches for distribution draws (gillespie algorithm, landscape sampling).

- `config.ini`

    -parameter file passed via command line.

- `runs.bat`

   -batch file executing the model for investigated parameter combinations.