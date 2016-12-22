# csapex_jannlab 
The csapex_jannlab  package provides plugins for the CS::APEX framework which
enable the usage of the JANNLab neural networks framework. 

    [https://github.com/JANNLab/JANNLab]:https://github.com/JANNLab/JANNLab

The fast data communiction between plugins and the JANNLab framework are
realized using TCP/IP socket connections. This is necessary since a 
data exchange between C++ and Java had to be realized. 

The socket connections are provided with the cslibs_jcppsocket library 
which can be found in our repository.

    [https://github.com/cogsys-tuebingen/cslibs_jcppsocket]:https://github.com/cogsys-tuebingen/cslibs_jcppsocket
    