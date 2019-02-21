# Game of Life

A C++ implementation of Conway's Game of Life. Uses [SFML](https://www.sfml-dev.org/) to render the graphics. 

The game initially starts out with a random configuration of cells.

Stops the simulation whenever "equilibrium" is detected, i.e. when nothing is really changing anymore.

The logic for detecting equilibrium is this:
  1. Get the hash of the current state and store it in a circular buffer of size 64
  2. Check if this hash has occured previously in the past 64 states
  3. If it has, increment a counter. If it has not, reset the counter.
  4. When that counter reaches 64 (the size of the buffer), we can conclude that things aren't really changing anymore
 
This method detects static cells and cycling structures (assuming that these structures' cycles last shorter than 64 states). 

The hash is calculated using [PicoSHA2](https://github.com/okdshin/PicoSHA2).
