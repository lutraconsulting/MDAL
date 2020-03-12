.. _faq:

================================================================================
FAQ
================================================================================

.. only:: not latex

    .. contents::
       :depth: 3
       :backlinks: none

What does MDAL stand for?
+++++++++++++++++++++++++

MDAL - Mesh Data Abstraction Library

When would I use it?
+++++++++++++++++++++++++

Most often this kind of representation is used when preparing data for simulation software or when viewing results of physical simulations, typically for meteorology, oceanography, hydrological or hydraulic models. All computation in such software is done on meshes, with values (physical quantities) usually stored in vertices (less commonly in edges or faces). Results usually comprise of various quantities (e.g. wind speed, water depth) which may be also time-varying (e.g. calculated water flow estimates in 5 minute intervals).

Some of the modelling software packages are free and open source (e.g. AnuGA, EPANET), there are some freeware packages (e.g. Basement, HEC-RAS) as well as many commercial pieces of software.

When was the MDAL project started?
++++++++++++++++++++++++++++++++++

In 2012, Peter Wells and Saber Razmjooei started QGIS Crayfish plugin.

In 2018, Lutra Consulting Ltd. ported and rewritten some parts of Crayfish plugin for separate C++ library that
was used in the early QGIS 3.x released as base for QGIS's Mesh Layer.

Is MDAL proprietary software?
+++++++++++++++++++++++++++++++++

No, MDAL is a Free and Open Source Software.

What license does MDAL use?
+++++++++++++++++++++++++++++++

See :ref:`license`

What operating systems does MDAL run on?
++++++++++++++++++++++++++++++++++++++++++++

You can use MDAL on all modern flavors of Unix: Linux, FreeBSD, Mac OS X; all supported versions of Microsoft Windows; mobile environements (Android and iOS).

Is there a graphical user interface to MDAL?
++++++++++++++++++++++++++++++++++++++++++++++++

See :ref:`software_using_mdal`

.. toctree::
   :hidden:

   software_using_mdal

What compiler can I use to build MDAL?
++++++++++++++++++++++++++++++++++++++++++++++++

MDAL can be compiled with a C++11 capable compiler.

I have a question that's not answered here. Where can I get more information?
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

See :ref:`community`

Keep in mind, the quality of the answer you get does bear some relation to the quality of the question. If you need more detailed explanation of this, you can find it in essay `How To Ask Questions The Smart Way <http://www.catb.org/~esr/faqs/smart-questions.html>`_ by Eric S. Raymond.

How do I add support for a new format?
++++++++++++++++++++++++++++++++++++++

To some extent this is now covered by the :ref:`add_driver_tut`

How do I cite MDAL ?
++++++++++++++++++++

See `CITATION`_

.. _`CITATION`: https://github.com/lutraconsulting/MDAL/blob/master/CITATION
