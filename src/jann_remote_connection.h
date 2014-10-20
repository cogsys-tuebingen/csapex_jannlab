#ifndef FEATURE_CLASSIFIER_JCPP_H
#define FEATURE_CLASSIFIER_JCPP_H

/// PROJECT
#include <csapex/model/node.h>
#include <utils_jcppsocket/cpp/socket.h>

namespace jannlab {
class JANNRemoteConnection : public csapex::Node
{
public:
    JANNRemoteConnection();

    virtual void setupParameters();
    virtual void process();
    virtual void setup();

private:
    void tryMakeSocket();
    void makeSocket();

private:
    csapex::Input                    *input_;
    csapex::Output                   *output_;

    utils_jcppsocket::SyncSocket::Ptr socket_;
};
}

#endif // FEATURE_CLASSIFIER_JCPP_H
