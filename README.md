IPVS httpd
==========

Summary
-------

HTTP daemon which allows for dynamically updating the weight of load-balanced servers.

Dependencies
------------

### libipvs

This one is included in the repo, upstream doesn't seem to officially exist anywhere
except for within the keepalive source code and the ipvsadm source code 
(both upstream projects have their own copy of libipvs)

### cpp-netlib

Can be found here: https://github.com/cpp-netlib/cpp-netlib

Currently using commit: 2b910a62ee723e190231de1f2efa05618b560f97
(which solves some issues in stable 0.12.0)


