Xilinx Cosine Similarity Alveo Product Installation
===================================================

Follow the steps below to install the Cosine Similarity Alveo Product. Skip the collaboration section if
only deploying.

Install Cosine Similarity Library from a Pre-built Package
-------------------------------------------------------------
* Get the installation package **amd-agml-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.

* Install Xilinx CosineSim and Recommendation Engine Alveo Products and dependencies 
  (XRT, XRM, and Alveo firmware packages)

.. code-block:: bash

   tar xzf amd-agml-install-%PACKAGE_VERSION.tar.gz
   cd amd-agml-install && ./install.sh -p cosinesim


Set up the Alveo Accelerator Card
-------------------------------------

The Cosine Similarity Alveo Product requires the following Alveo cards and their 
corresponding firmware versions to be installed on each Alveo card to use.

* U50: xilinx_u50_gen3x16_xdma_201920_3 
* U55C: xilinx_u55c_gen3x16_xdma_base_2
 
.. include:: ../common/flash-alveo.rst