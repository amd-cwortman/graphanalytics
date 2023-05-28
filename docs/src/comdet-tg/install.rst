Xilinx Community Detection for TigerGraph Installation
======================================================

---------------------------------------

Install the Community Detection Software
------------------------------------------

Follow the steps in *only one* of the two sections below.

Install the Community Detection from a Package
************************************************

* Get the installation package **amd-agml-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.  This package contains
  the Community Detection as well as its dependencies: XRT, XRM, the Alveo U50 Platform, and the Louvain Modularity
  Alveo Product.

* Install the Community Detection product and its dependencies by un-tarring the package and running
  the included install script.

.. code-block:: bash

   tar xzf amd-agml-install-%PACKAGE_VERSION.tar.gz
   cd amd-agml-install && ./install.sh

* Install the Community Detection plug-in into the TigerGraph installation. This step makes TigerGraph aware
  of the Community Detection features.

.. code-block:: bash

   amd-agml-installetect/%COMDETECT_TG_VERSION/install.sh

..  note:: 
    
    If you only need to build applications utilizing Xilinx GraphAnalytics 
    products, you can skip the section "Collaborating on Community Detection 
    Plugin" below.


Uninstalling the Community Detection
--------------------------------------

You can uninstall the Community Detection from TigerGraph by running the install script with the ``-u`` option:

.. code-block:: bash

   /opt/amd/apps/agml/integration/Tigergraph-3.x/agml/%PACKAGE_VERSION/comdetect/install.sh -u

**TIP**: To avoid TigerGraph errors, uninstall any queries and UDFs that use the Community Detection,
before uninstalling the Community Detection itself.
