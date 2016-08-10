#ifndef FEATURE_CLASSIFIER_JCPP_H
#define FEATURE_CLASSIFIER_JCPP_H

/// PROJECT
#include <csapex/model/node.h>
#include <cslibs_jcppsocket/cpp/sync_client.h>

namespace jannlab {
class JANNRemoteConnection : public csapex::Node
{
public:
    JANNRemoteConnection();

    virtual void setupParameters(Parameterizable& parameters);
    virtual void process() override;
    virtual void setup(csapex::NodeModifier& node_modifier) override;

private:
    void tryMakeSocket();
    void makeSocket();

private:
    csapex::Input                    *input_;
    csapex::Output                   *output_;

    cslibs_jcppsocket::SyncClient::Ptr client_;
};
}

#endif // FEATURE_CLASSIFIER_JCPP_H
