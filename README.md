QD Channel integration for ns-3
================================

## QD Channel code
This repository contains in the code for the ns-3 integration of the [QD channel model implementation from NIST and University of Padova](https://github.com/signetlabdei/qd-realization). Please refer to the repository of the QD implementation for a complete documentation on how to generate new scenarios. In this repository, we provide a first example based on a simple indoor scenario.

The implementation leverages the spectrum implementation of the matrix-based channel model introduced in ns-3.31, described in [this paper](https://arxiv.org/pdf/2002.09341).
Please cite it if you intend to use this module:
```bibtex
@misc{qd-realization,
  author = {Mattia Lecci and Paolo Testolina and Michele Polese and Marco Giordani and Michele Zorzi},
  title = {Accuracy vs. Complexity for mmWave Ray-Tracing: A Full Stack Perspective},
  howpublished = {arXiv:2007.07125},
}
```

References on the QD model and its usage can be found in the following papers:
- M. Lecci, P. Testolina, M. Polese, M. Giordani, M. Zorzi, "Accuracy vs. Complexity for mmWave Ray-Tracing: A Full Stack Perspective," accepted for publication in IEEE Transaction of Wireless Communications, pre-print available: [arXiv:2007.07125](https://arxiv.org/abs/2007.07125).
- M. Lecci, T. Zugno, S. Zampato, M. Zorzi, "A Full-Stack Open-Source Framework for Antenna and Beamforming Evaluation in mmWave 5G NR," in IEEE International Conference on Communications: Mobile and Wireless Networks Symposium (ICC MWN), Montreal, Canada, Jun. 2021, pre-print available: [arXiv:2011.05800](https://arxiv.org/abs/2011.05800).
- M. Lecci, M. Polese, C. Lai, J. Wang, C. Gentile, N. Golmie, M. Zorzi, "Quasi-Deterministic Channel Model for mmWaves: Mathematical Formalization and Validation", in IEEE Global Communications Conference (GLOBECOM), Taipei, Taiwan, Dec. 2020, DOI: [10.1109/GLOBECOM42002.2020.9322374](https://doi.org/10.1109/GLOBECOM42002.2020.9322374).
- C. Gentile, P. B. Papazian, R. Sun, J. Senic, and J. Wang, “Quasi- Deterministic Channel Model Parameters for a Data Center at 60 GHz,” IEEE Antennas and Wireless Propagation Letters, vol. 17, no. 5, pp. 808–812, May 2018, DOI: [10.1109/LAWP.2018.2817066](https://doi.org/10.1109/LAWP.2018.2817066).
- C. Lai, R. Sun, C. Gentile, P. B. Papazian, J. Wang, and J. Senic, “Methodology for multipath-component tracking in millimeter-wave channel modeling,” IEEE Transactions on Antennas and Propagation, vol. 67, no. 3, pp. 1826–1836, Mar. 2019, DOI: [10.1109/TAP.2018.2888686](https://doi.org/10.1109/TAP.2018.2888686).
- M. Lecci, P. Testolina, M. Giordani, M. Polese, T. Ropitault, C. Gentile, N. Varshney, A. Bodi, and M. Zorzi, "Simplified ray tracing for the millimeter wave channel: A performance evaluation," in Workshop on Information Theory and Applications (ITA), San Diego, CA, US, Feb. 2020, DOI: [10.1109/ITA50056.2020.9244950](https://doi.org/10.1109/ITA50056.2020.9244950).

The example `qd-channel-model-example.cc` can be used as a starting point to understand how to use the ray tracer in wireless simulations with ns-3.

## Install

### Prerequisites ###

To run simulations using this module, you will need to install ns-3, and clone
this repository inside the `contrib` directory. 
Required dependencies include git and a build environment.

Please check the [release page](https://github.com/signetlabdei/qd-channel/releases) for further information on the compatible versions of ns-3.

#### Installing dependencies ####

Please refer to [the ns-3 wiki](https://www.nsnam.org/wiki/Installation) for instructions on how to set up your system to install ns-3.

#### Downloading #####

First, clone the main ns-3 repository:

```bash
git clone https://gitlab.com/nsnam/ns-3-dev ns-3-dev
```

Then, clone the qd-channel module:
```bash
git clone https://github.com/signetlabdei/qd-channel ns-3-dev/contrib/qd-channel
```

### Compilation ###

Configure and build ns-3 from the `ns-3-dev` folder:

```bash
./waf configure --enable-tests --enable-examples
./waf build
```

This module does not provide Python bindings at the moment, they will be added in the near future.

### Documentation ###

To compile the documentation, please follow the instructions from the [ns-3 manual](https://www.nsnam.org/docs/manual/html/documentation.html).

Basic steps:

1. Install the dcumentation-specific depdendencies as described in the [ns-3 installation guide](https://www.nsnam.org/wiki/Installation)
1. You might need to fix the ImageMagick permissions for ghostscript files

Compiling standalone documentation:

1. Run in your terminal, in the ns-3 root folder, `make -C contrib/qd-channel/doc html`
1. Open with your browser the file `contrib/qd-channel/doc/models/build/html/qd-channel.html` to visualize the documentation of the model

Adding documentation to ns-3:

1. In `doc/models/source/index.rst`, add `qd-channel` in the TOC tree
1. In `doc/models/Makefile`, add `../../contrib/qd-channel/doc/source/qd-channel.rst` as one of the `SOURCES`
1. Run in your terminal, in the ns-3 root folder, `make -C doc/models html`
1. Open with your browser the file `doc/models/build/html/qd-channel.html`. You should locate the `QD Channel Model` in the TOC, as added before
