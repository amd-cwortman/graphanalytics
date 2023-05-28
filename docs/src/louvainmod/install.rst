.. _louvain-install-label:

AMD Louvain Modularity Alveo Product Installation
=================================================

Follow the steps below to install the Louvain Modularity Alveo Product. 

Installing Louvain Modularity Library from a Pre-built Package
-------------------------------------------------------------
* Get the installation package **amd-agml-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.

* Install Xilinx Louvain Modularity and Community Detection Alveo products and dependencies 
  (XRT, XRM, and Alveo firmware packages)

.. code-block:: bash

   tar xzf amd-agml-install-%PACKAGE_VERSION.tar.gz
   cd amd-agml-install && ./install.sh -p louvainmod


Setting up the Alveo Accelerator Card
-------------------------------------

The Louvain Modularity Alveo Product requires the following Alveo cards and their 
corresponding firmware versions to be installed on each Alveo card to use.
* U50: xilinx_u50_gen3x16_xdma_201920_3 
* U55C: xilinx_u55c_gen3x16_xdma_base_2
 
.. include:: ../common/flash-alveo.rst

