QD Channel Model
----------------

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

Standalone ns-3 application that adds the support for ray-tracing based channel modeling.

Model Description
*****************

The source code for the new module lives in the directory ``contrib/qd-channel``.

The module aims at modeling a MIMO wireless channel assuming that phased antenna arrays from the class ``ThreeGppAntennaArrayModel`` are used.
Channel traces are assumed to be produced by an open-source ray-tracer, available `here <https://github.com/wigig-tools/qd-realization>`_.

Design
======

The implementation leverages the spectrum implementation of the matrix-based channel model introduced in ns-3.31, described in `this paper <https://arxiv.org/pdf/2002.09341>`_.

Scope and Limitations
=====================

The module is able to import channel traces with a given format and create a spectrum channel usable by a generic (or even external) ns-3 module.
The supported format is that of the already-mentioned open-source ray-tracer, supporting a number of nodes moving over time, timestep after timestep, with a channel defined between each pair of nodes.

The current approach has the following limitations:

* The folder structure is rigid, an incorrect structure should break the simulation before it even starts, but possible silent fails are not ecluded
* Time steps are considered to be of equal duration and computed from the scenario itself
* The channel is considered constant over a timestep, with no support to fast fading for the moment
* Polarization is not supported
* Mobility is not fully supported on ns-3. While the ray-tracing software can support mobility, only its effects are imported into ns-3 in the form of a varying channel. Currently, only ``ConstantPositionMobilityModel`` is supported. Please check the `Usage`_ section on why and how this works.

References
==========

Some works using this module already exist:

* M. Lecci, P. Testolina, M. Polese, M. Giordani, M. Zorzi, "Accuracy vs. Complexity for mmWave Ray-Tracing: A Full Stack Perspective," preprint available: `arXiv:2007.07125 <https://arxiv.org/abs/2007.07125>`_
* M. Lecci, T. Zugno, S. Zampato, M. Zorzi, "A Full-Stack Open-Source Framework for Antenna and Beamforming Evaluation in mmWave 5G NR," preprint available: `arXiv:2011.05800 <https://arxiv.org/abs/2011.05800>`_

Furthermore, theoretical papers regarding the ray-tracing simulator, the Quasi-Deterministic (QD) channel model, and measurements to validate the channel can be found here:

* M. Lecci, M. Polese, C. Lai, J. Wang, C. Gentile, N. Golmie, M. Zorzi, "Quasi-Deterministic Channel Model for mmWaves: Mathematical Formalization and Validation," to be presented at IEEE IEEE Global Communications Conference (GLOBECOM), Dec. 2020, Taipei, Taiwan, available here: `arXiv:2006.01235 <https://arxiv.org/abs/2006.01235>`_
* M. Lecci, P. Testolina, M. Giordani, M. Polese, T. Ropitault, C. Gentile, N. Varshney, A. Bodi, and M. Zorzi, "Simplified ray tracing for the millimeter wave channel: A performance evaluation", in Proceedings of the Workshop on Information Theory and Applications (ITA), Feb. 2020, San Diego, USA, available here: `arXiv:2002.09179 <https://arxiv.org/abs/2002.09179>`_, DOI: 10.1109/ITA50056.2020.9244950
* C. Gentile, P. B. Papazian, R. Sun, J. Senic, and J. Wang, “Quasi- Deterministic Channel Model Parameters for a Data Center at 60 GHz,” IEEE Antennas and Wireless Propagation Letters, vol. 17, no. 5, pp. 808–812, May 2018, DOI: 10.1109/LAWP.2018.2817066
* C. Lai, R. Sun, C. Gentile, P. B. Papazian, J. Wang, and J. Senic, “Methodology for multipath-component tracking in millimeter-wave channel modeling,” IEEE Transactions on Antennas and Propagation, vol. 67, no. 3, pp. 1826–1836, Mar. 2019, DOI: 10.1109/TAP.2018.2888686


Please cite our work if you find it interesting!

Usage
*****

This section is principally concerned with the usage of your model, using
the public API.  Focus first on most common usage patterns, then go
into more advanced topics.

Building New Module
===================


First, clone the main ns-3 repository, e.g.,:

``git clone https://gitlab.com/nsnam/ns-3-dev ns-3-dev``

Then, clone the qd-channel module:

``git clone https://github.com/signetlabdei/qd-channel ns-3-dev/contrib/qd-channel``

Configure and build ns-3 from the `ns-3-dev` folder:

| ``./waf configure --enable-tests --enable-examples``
| ``./waf build``

This module does not provide Python bindings at the moment, they will be added in the near future.

.. Helpers
.. =======

.. What helper API will users typically use?  Describe it here.

Attributes
==========

TODO (Frequency? why SetFrequency?)

Setting up a scenario
=====================

TODO

.. Output
.. ======

.. What kind of data does the model generate?  What are the key trace
.. sources?   What kind of logging output can be enabled?

.. Advanced Usage
.. ==============

.. Go into further details (such as using the API outside of the helpers)
.. in additional sections, as needed.

Examples
========

We currently provide only one example: ``examples/qd-channel-model-example.cc``.
For more information, please the the source code.

Troubleshooting
===============

For any problem with the module, please open an issue. The maintainers will do their best to provide technical support!

Validation
**********

Please check the `References`_ section.
