# Balatro Seed Finder

Balatro Seed Finder, is an CPU-bound engine to search for specific seeds based on filters in the the filters directory.

## Attributions

Most of the engine was taken from [TheSoul](https://github.com/SpectralPack/TheSoul) and [Immolate](https://github.com/SpectralPack/TheSoul) from the [SpectralPack](https://github.com/SpectralPack/) team. Thank you for the amazing work on the simulation.

## How to

### How to build a specific filter

You can build an existing filter with the following command : `sh tools/build.sh <filter_name>`. The `filter_name` is the same name of your filter name without `_filter.hpp`. 

For example, to build the Perkeo filter you can run : `sh tools/build.sh perkeo`. It will generate the executable in `dist/seed_finder_<filter_name>`.

### Run the seed finder

Once your build done, you can run the executable to start the search. The executable can receive as an argument the 8-char seed to begin with. It is quite helpful to resume an interrupted process.

The results will the logged in a `matches_YYYYMMDD_HHmmss.csv` file. It only logs the result with a score of at least 1.

### How to design a filter

Filters are in the `filters` directory. It contains a README.md file to understand how to write your own filters.

## Next steps

We would like to have a second, more tool that indexes all Balatro seeds, with tags about specifics of a seed (for example, Perkeo on first Charm Tag). To be able to scan a big number of seeds, we would need a lot of threads available, and a networking capability to manage the workload across several nodes.

### Opti ideas
  Better optimization approaches for this codebase would be:
  - SIMD vectorization (AVX-512) to process multiple seeds per CPU core
  - Memory optimizations (pooling, reducing string ops)
  - Algorithmic improvements (early pruning, parallel filter chains)