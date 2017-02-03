
/// PROJECT
#include <csapex/model/node.h>
#include <csapex/msg/io.h>
#include <csapex/param/parameter_factory.h>
#include <csapex/model/node_modifier.h>
#include <csapex/utility/register_apex_plugin.h>
#include <csapex/msg/generic_value_message.hpp>
#include <csapex/msg/generic_vector_message.hpp>
#include <csapex_ml/features_message.h>

/// SYSTEM
#include <mutex>
#include <fstream>

using namespace csapex;
using namespace csapex::connection_types;

namespace jannlab {



class JANNTimeSeriesExport : public csapex::Node
{
public:
    JANNTimeSeriesExport()
    {
    }

    void setup(csapex::NodeModifier& modifier) override
    {
        in_ = modifier.addInput<GenericVectorMessage, FeaturesMessage>("Input");
    }

    void setupParameters(csapex::Parameterizable& params) override
    {
        addParameter(csapex::param::ParameterFactory::declareFileOutputPath("path",
                                                                            csapex::param::ParameterDescription("Directory to write messages to"),
                                                                            "", ".nn"));

        addParameter(csapex::param::ParameterFactory::declareTrigger("save",
                                                                     csapex::param::ParameterDescription("Save the obtained data!")),
                     std::bind(&JANNTimeSeriesExport::save, this));

        addParameter(csapex::param::ParameterFactory::declareTrigger("clear",
                                                                     csapex::param::ParameterDescription("Clear buffered data!")),
                     std::bind(&JANNTimeSeriesExport::clear, this));
    }

    void process() override
    {
        if(msg::hasMessage(in_)) {

            std::shared_ptr<std::vector<FeaturesMessage> const> series = msg::getMessage<GenericVectorMessage, FeaturesMessage>(in_);
            m_.lock();
            msgs_.push_back(*series);
            m_.unlock();
        }

    }

    template<typename T>
    void exportSeries(const std::vector<FeaturesMessage> &features,
                             const std::map<int, std::vector<int> > &labels,
                             std::ofstream &out)
    {
        for(auto feature : features) {
            exportVector(feature.value, out);
        }
        out << std::endl;

        for(auto feature : features) {
            exportVector<int>(labels.at(feature.classification), out);
        }
        out << std::endl;
    }

    template<typename T>
    void exportVector(const std::vector<T> &vector,
                             std::ofstream &out)
    {
        for(typename std::vector<T>::const_iterator
            it  = vector.begin() ;
            it != vector.end() ;
            ++it) {
            out << " " << *it;
        }
    }

    void labelMap(const std::vector<std::vector<FeaturesMessage>> &msgs,
                         std::map<int, std::vector<int> > &labels)
    {
        for (auto data : msgs){
            for(std::vector<FeaturesMessage>::const_iterator
                it = data.begin() ;
                it != data.end() ;
                ++it) {

                int class_id = it->classification;
                if(labels.find(class_id) == labels.end()) {
                    labels.insert(std::make_pair(class_id, std::vector<int>()));
                }
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


    void save()
    {
        std::vector<std::vector<FeaturesMessage>> msgs;
        m_.lock();
        msgs = msgs_;
        m_.unlock();

        std::string     path = readParameter<std::string>("path");
        std::ofstream   out_file(path.c_str());
        std::ofstream   out_mapping((path + ".mapping").c_str());

        std::map<int, std::vector<int> > labels;
        labelMap(msgs, labels);

        for(auto signal : msgs) {
            exportSeries<float>(signal, labels, out_file);
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

    void clear()
    {
        m_.lock();
        msgs_.clear();
        m_.unlock();
    }


private:
    Input* in_;
    Output* out_;
    std::mutex                                            m_;
    std::vector<std::vector<csapex::connection_types::FeaturesMessage>>  msgs_;

};

} // jannlab


CSAPEX_REGISTER_CLASS(jannlab::JANNTimeSeriesExport, csapex::Node)

