/// HEADER
#include "jann_remote_connection.h"

/// PROJECT
#include <csapex_core_plugins/vector_message.h>
#include <csapex_ml/features_message.h>
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <utils_param/parameter_factory.h>
#include <csapex/model/node_modifier.h>
#include <boost/lexical_cast.hpp>
#include <utils_jcppsocket/cpp/socket_msgs.h>

/// SYSTEM
#include <csapex/utility/register_apex_plugin.h>


CSAPEX_REGISTER_CLASS(jannlab::JANNRemoteConnection, csapex::Node)

using namespace jannlab;
using namespace csapex;
using namespace csapex::connection_types;

using namespace utils_jcppsocket;
using namespace serialization;

JANNRemoteConnection::JANNRemoteConnection()
{
}

void JANNRemoteConnection::setupParameters()
{
    addParameter(param::ParameterFactory::declareText("server name", "localhost"));
    addParameter(param::ParameterFactory::declareText("server port", "6666"));
    addParameter(param::ParameterFactory::declareTrigger("connect"), boost::bind(&JANNRemoteConnection::makeSocket, this));
}

namespace {

typedef std::shared_ptr<std::vector<FeaturesMessage> const > FeaturesMsgsConst;
typedef std::vector< std::vector<double> >                     DoubleBlock;

inline void processRequest(FeaturesMsgsConst &msgs,
                           SyncClient::Ptr   &client,
                           std::shared_ptr<DoubleBlock> &results)
{
    BlockMsg<double>::Ptr block(new BlockMsg<double>);
    unsigned int step = msgs->front().value.size();
    unsigned int rows = msgs->size();
    std::vector<double> data(step * rows, 0);

    for(unsigned int i = 0 ; i < rows ; ++i) {
        for(unsigned int j = 0 ; j < step ; ++j) {
            data.at(i * step + j) = msgs->at(i).value.at(j);
        }
    }

    block->assign(data, step);

    SocketMsg::Ptr res;

    if(client->query(block, res)) {
        ErrorMsg::Ptr          err = boost::dynamic_pointer_cast<ErrorMsg>(res);
        BlockMsg<double>::Ptr  dat = boost::dynamic_pointer_cast<BlockMsg<double> >(res);

        if(err.get() != nullptr) {
            std::stringstream ss;
            ss << "Got error [ " << err->get() << " ]";
            throw std::runtime_error(ss.str());
        }

        if(dat.get() != nullptr) {
            results->resize(rows);
            for(unsigned int i = 0 ; i < dat->rows() ; ++i) {
                results->at(i).resize(dat->cols());
                for(unsigned int j = 0 ; j < dat->cols() ; ++j) {
                    results->at(i).at(j) = dat->at(i,j);
                }
            }
        }
    } else {
        throw std::runtime_error("socket error");
    }
}
}


void JANNRemoteConnection::process()
{
    FeaturesMsgsConst              in = input_->getMessage<GenericVectorMessage, FeaturesMessage>();
    std::shared_ptr<DoubleBlock> out(new DoubleBlock);

    if(!client_) {
        tryMakeSocket();
    }

    if(!client_->isConnected()) {
        if(!client_->connect()) {
            throw std::runtime_error("Couldn't connect!");
        }
    }

    if(!client_) {
        setError(true, "no connection to remote classifier", EL_WARNING);
        return;
    }

    processRequest(in, client_, out);

    output_->publish<GenericVectorMessage, std::vector<double> >(out);
}


void JANNRemoteConnection::setup()
{
    input_  = modifier_->addInput<GenericVectorMessage,  FeaturesMessage>("Features");
    output_ = modifier_->addOutput<GenericVectorMessage, std::vector<double> >("Results");
}

void JANNRemoteConnection::tryMakeSocket()
{
    std::string str_name = readParameter<std::string>("server name");
    std::string str_port = readParameter<std::string>("server port");

    if(!str_name.empty() && !str_port.empty()) {
        makeSocket();
    }
}

void JANNRemoteConnection::makeSocket()
{
    std::string str_name = readParameter<std::string>("server name");
    std::string str_port = readParameter<std::string>("server port");
    int         port = boost::lexical_cast<int>(str_port);

    client_.reset(new utils_jcppsocket::SyncClient(str_name, port));

    setError(false);
}
