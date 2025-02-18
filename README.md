# Talbot

This repository contains the code of the tool developed as a part of the master's thesis "Evaluation of Virtual to Physical Address Translation". It focuses on reverse engineering the translation caches implemented by Intel.

## Structure

This repository is split into three different parts:

- Talbot, which contains the code for the automated tool itself
- CacheQuery, which contains the code for a tool to conveniently query translation caches for manual analysis
- CrossHyperthread, which separately implements a cross-hyperthread experiment

More information on the tools and their usage can be found in their READMEs.

Please note that all of our tools require root access, as they use kernel modules to interact with page tables.

# Warning

As our tools are running in kernel mode, errors may crash your system.