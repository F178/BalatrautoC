# Third-Party Notices

## Balatro Seed Finder

Balatrauto includes a modified subset of `izanagi1995/balatro-seed-finder`,
commit `b3a112f8e6678f463e5bdce4298a04810bbc1594`, for deterministic seed
simulation and search.

Source: https://github.com/izanagi1995/balatro-seed-finder

The upstream project is licensed under Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International. Its full license is
preserved at `third_party/balatro-seed-finder/LICENSE`.

Local modifications:

- Added a Microsoft Visual C++ definition for the upstream force-inline macro.
- Added the standard functional header required by the public item-choice API.
- Added a narrow public method for applying the user's Joker unlock state to a
  simulated run.

The engine is derived in part from TheSoul and Immolate by the SpectralPack
team, as credited by the upstream project.
