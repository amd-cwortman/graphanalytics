===============================================
Running Entity Resolution Using JupyterNotebook
===============================================

* Open a terminal using your Linux credentials
* Set up a directory that is readable and writable by the user "tigergraph"
* Execute the commands below to run the demo

  .. code-block:: bash

    # Copy examples from the installation to your own working directory mis-examples
    cp -r /opt/amd/apps/agml/integration/Tigergraph-3.x/agml/%PACKAGE_VERSION/fuzzymatch/examples fuzzymatch-examples

    cd fuzzymatch-examples/entity-resolution
    
    # Install demo specific UDFs. This is needed to run **ONLY ONCE**.
   ./bin/install-udf.sh

    # Print entity resolution demo help messages
    ./bin/run.sh -h

   # Run entity resolution demo with default settings. Settings can be changed via command
   # line options as shown above
   ./bin/run.sh