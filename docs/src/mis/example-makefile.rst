===========================================
C++ Example using Makefile
===========================================

* Open a terminal using your Linux credentials
* Execute following commands to run the example

    .. code-block:: bash

        cp -r /opt/amd/apps/agml/mis/%MIS_VERSION/examples mis-examples
        cd mis-examples/cpp
    
        # Print makefile usage and options
        make help
    
        # Run the demo on U50 Alveo card
        make run
    
        # Run the demo on U55C Alveo card
        make run deviceNames=u55c