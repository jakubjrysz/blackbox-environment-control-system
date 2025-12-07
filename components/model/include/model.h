#pragma once
#include "open62541.h"

extern UA_NodeId tempNodeId;
extern UA_NodeId lightNodeId;

void addSensorNodes(UA_Server *server);

void setTemperatureValue(UA_Server *server, double value);
void setLightValue(UA_Server *server, double value);
