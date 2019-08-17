# agsfastwfc

[![Build Status](https://dev.azure.com/ericoporto/agsfastwfc/_apis/build/status/ericoporto.agsfastwfc?branchName=master)](https://dev.azure.com/ericoporto/agsfastwfc/_build/latest?definitionId=7&branchName=master)

🚨THIS IS BROKEN🚨

**AGS Fast Wave Function Collapse Plugin**

Using the [fast-wfc library code](https://github.com/math-fehr/fast-wfc) from Mathieu Fehr and Nathanaël Courant.

I just made a simple interface with Adventure Game Studio.

## Building agsfastwfc plugin

first you need to clone this repository to the folder you clone your repos (ex: ~/git). 

If you haven't already, also clone AGS and set the branch for ags3.

```
git clone https://github.com/ericoporto/agsfastwfc.git
git clone https://github.com/adventuregamestudio/ags.git 
pushd ags 
git checkout ags3
popd
```

### Windows

Just load the solution on Visual Studio 2019 or 2017, and hit build.

### Linux

You need gcc that supports C++17. Load a terminal and do the below after cloning.

```
cd agsfastwfc/agsfastwfc/
make
```

### MacOS

MacOS build is untested, but should work the same way as Linux.

## Image samples

The image samples come from [https://github.com/mxgmn/WaveFunctionCollapse](https://github.com/mxgmn/WaveFunctionCollapse)

## Licence

This plugin interface is made with MIT License Copyright (c) 2019 Érico Vieira Porto.

fast-wfc Copyright (c) 2018-2019 Mathieu Fehr and Nathanaël Courant.

MIT License, see [`LICENSE`](LICENSE) for further details.
