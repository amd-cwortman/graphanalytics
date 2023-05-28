C++ Example of Fuzzy Match using Makefile
===========================================

* Open a terminal using your Linux credentials
* Execute following commands to run the example

  .. code-block:: bash
  
    cp -r /opt/amd/apps/agml/fuzzymatch/%FUZZYMATCH_VERSION/examples fuzzymatch-examples
    cd fuzzymatch-examples/cpp
    
    # Print makefile usage and options
    make help

    # Run the demo on U50 card
    make run

    # Run the demo on AWS F1
    make run deviceNames=aws-f1

    # Run the demo on U55C
    make run deviceNames=u55c    
