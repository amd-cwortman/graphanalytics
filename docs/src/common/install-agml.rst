==================================
Install AGML Plugin for TigerGraph
==================================

Install AGML Plugin for TigerGraph
----------------------------------

* Get the installation package **amd-agml-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.
  This package contains the Maximal Independent Set as well as its dependencies.

* Install the product and its dependencies by un-tarring the package and running
  the included install script.

  .. code-block:: bash

    tar xzf amd-agml-install-%PACKAGE_VERSION.tar.gz

    # Install AGML for the specified device. Supported device name: u50, u55c, aws-f1
    cd amd-agml-install && ./install.sh -p agml -d <device-name>


* Install AGML plugin into the TigerGraph installation for the specified Alveo device. Below is an example targeting Alveo U50:"

  .. code-block:: bash

    /opt/amd/apps/agml/integration/Tigergraph-3.x/agml/%PACKAGE_VERSION/install.sh -d u50

  Supported Alveo devices:

  * u50
  * u55c
  * aws-f1

Set up the Alveo Accelerator Card
---------------------------------

The AGML requires the following Alveo cards and their corresponding firmware versions to be installed on each Alveo card to use.

* U50: xilinx_u50_gen3x16_xdma_201920_3 
* U55C: xilinx_u55c_gen3x16_xdma_base_2

.. include:: ../common/flash-alveo.rst


Uninstall AGML Plugin for TigerGraph
------------------------------------

You can uninstall the AGML plugin from TigerGraph by running the install script with the ``-u`` option:

.. code-block:: bash

   /opt/amd/apps/agml/integration/Tigergraph-3.x/agml/%PACKAGE_VERSION/install.sh -u

**TIP**: To avoid TigerGraph errors, uninstall any queries and UDFs that use the AGML plugin before
uninstalling the AGML plugin.
