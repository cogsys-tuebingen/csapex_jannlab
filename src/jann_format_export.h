#ifndef JANN_FORMAT_EXPORTER_H
#define JANN_FORMAT_EXPORTER_H

/// PROJECT
#include <csapex/model/node.h>
#include <csapex/model/token_data.h>
#include <csapex/msg/generic_vector_message.hpp>
#include <csapex_ml/features_message.h>

/// SYSTEM
#include <mutex>

namespace jannlab {
class JANNFormatExport : public csapex::Node
{
public:
    JANNFormatExport();

    virtual void setup(csapex::NodeModifier& node_modifier) override;
    virtual void setupParameters(Parameterizable& parameters);
    virtual void process() override;

protected:
    void setExportPath();

    void save();
    void clear();

private:
    csapex::Input*                                          in_;
    csapex::Input*                                          in_vector_;

    std::mutex                                            m_;
    std::vector<csapex::connection_types::FeaturesMessage>  msgs_;

};
}


#endif // JANN_FORMAT_EXPORTER_H
