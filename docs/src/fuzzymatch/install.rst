========================================
Install Xilinx Fuzzy Match Alveo Product
========================================

Install Fuzzy Match Alveo from a Pre-built Package
------------------------------------------------------------------
* Get the installation package **amd-agml-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.
  This package contains the Fuzzy Match as well as all dependencies (XRT, XRM, Alveo 
  firmware, etc)

* Install Xilinx Fuzzy Match Alveo Products and dependencies: 

  .. code-block:: bash

    tar xzf amd-agml-install-%PACKAGE_VERSION.tar.gz
    cd amd-agml-install && ./install.sh -p fuzzymatch


.. include:: install-collaborate.rst
