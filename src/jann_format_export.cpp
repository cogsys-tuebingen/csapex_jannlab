/// HEADER
#include "jann_format_export.h"

/// PROJECT
#include <csapex/msg/io.h>
#include <csapex/model/connection_type.h>
#include <csapex/msg/message.h>
#include <csapex/param/parameter_factory.h>
#include <csapex/param/path_parameter.h>
#include <csapex/utility/register_apex_plugin.h>
#include <csapex/model/node_modifier.h>
#include <csapex/factory/message_factory.h>

/// SYSTEM
#include <fstream>

CSAPEX_REGISTER_CLASS(jannlab::JANNFormatExport, csapex::Node)

using namespace jannlab;
using namespace csapex;
using namespace connection_types;

JANNFormatExport::JANNFormatExport()
{
}

void JANNFormatExport::setupParameters(Parameterizable& parameters)
{
    addParameter(param::ParameterFactory::declareFileOutputPath("path",
                                                                param::ParameterDescription("Directory to write messages to"),
                                                                "", ".nn"));

    addParameter(param::ParameterFactory::declareTrigger("save",
                                                         param::ParameterDescription("Save the obtained data!")),
                 std::bind(&JANNFormatExport::save, this));

    addParameter(param::ParameterFactory::declareTrigger("clear",
                                                         param::ParameterDescription("Clear buffered data!")),
                 std::bind(&JANNFormatExport::clear, this));
}

void JANNFormatExport::setup(NodeModifier& node_modifier)
{
    in_        = node_modifier.addOptionalInput<FeaturesMessage>("Feature");
    in_vector_ = node_modifier.addOptionalInput<GenericVectorMessage, FeaturesMessage>("Features");
}

void JANNFormatExport::process()
{
    if(msg::hasMessage(in_)) {
        FeaturesMessage::ConstPtr msg = msg::getMessage<FeaturesMessage>(in_);
        m_.lock();
        msgs_.push_back(*msg);
        m_.unlock();
    }

    if(msg::hasMessage(in_vector_)) {
        std::shared_ptr<std::vector<FeaturesMessage> const> msgs =
                   msg::getMessage<GenericVectorMessage, FeaturesMessage>(in_vector_);
        m_.lock();
        for(std::vector<FeaturesMessage>::const_iterator
            it = msgs->begin() ;
            it != msgs->end();
            ++it) {
            msgs_.push_back(*it);
        }
        m_.unlock();
    }
}

namespace {
template<typename T>
inline void exportVector(const std::vector<T> &vector,
                         std::ofstream &out)
{
    for(typename std::vector<T>::const_iterator
        it  = vector.begin() ;
        it != vector.end() ;
        ++it) {
        out << " " << *it;
    }
    out << std::endl;
}

inline void labelMap(const std::vector<FeaturesMessage> &msgs, std::map<int, std::vector<int> > &labels)
{
    for(std::vector<FeaturesMessage>::const_iterator
        it = msgs.begin() ;
        it != msgs.end() ;
        ++it) {

        int class_id = it->classification;
        if(labels.find(class_id) == labels.end()) {
            labels.insert(std::make_pair(class_id, std::vector<int>()));
        }
    }

    int class_count = labels.size();
    int current_idx = 0;
    for(std::map<int, std::vector<int> >::iterator
        it  = labels.begin() ;
        it != labels.end() ;
        ++it ) {
        it->second.resize(class_count, 0);
        it->second.at(current_idx) = 1;
        ++current_idx;
    }
}
}


void JANNFormatExport::save()
{
    std::vector<FeaturesMessage> msgs;
    m_.lock();
    msgs = msgs_;
    m_.unlock();

    std::string     path = readParameter<std::string>("path");
    std::ofstream   out_file(path.c_str());
    std::ofstream   out_mapping((path + ".mapping").c_str());

    std::map<int, std::vector<int> > labels;
    labelMap(msgs, labels);

    for(std::vector<FeaturesMessage>::iterator
        it  = msgs.begin() ;
        it != msgs.end() ;
        ++it) {
        exportVector<float>(it->value, out_file);
        exportVector<int>(labels.at(it->classification), out_file);
    }

    for(std::map<int, std::vector<int> >::iterator
        it  = labels.begin() ;
        it != labels.end() ;
        ++it) {
        out_mapping << it->first << " : ";
        exportVector<int>(it->second, out_mapping);
    }

    out_mapping.close();
    out_file.close();
}

void JANNFormatExport::clear()
{
    m_.lock();
    msgs_.clear();
    m_.unlock();
}
