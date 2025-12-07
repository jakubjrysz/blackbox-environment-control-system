#include "model.h"

UA_NodeId tempNodeId;
UA_NodeId lightNodeId;


void setTemperatureValue(UA_Server *server, double value) {
    UA_Variant val;
    UA_Variant_setScalar(&val, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
    UA_Server_writeValue(server, tempNodeId, val);
}

void setLightValue(UA_Server *server, double value) {
    UA_Variant val;
    UA_Variant_setScalar(&val, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
    UA_Server_writeValue(server, lightNodeId, val);
}


void addSensorNodes(UA_Server *server) {

    // temp node
    UA_VariableAttributes tAttr = UA_VariableAttributes_default;
    tAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Temperature");
    tAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    tAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    double initialTemp = 0.0;
    UA_Variant_setScalar(&tAttr.value, &initialTemp, &UA_TYPES[UA_TYPES_DOUBLE]);

    tempNodeId = UA_NODEID_STRING(1, "Temperature");
    UA_Server_addVariableNode(server,
                               tempNodeId,
                               UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                               UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                               UA_QUALIFIEDNAME(1, "Temperature"),
                               UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                               tAttr,
                               NULL, NULL);


    // light node
    UA_VariableAttributes lAttr = UA_VariableAttributes_default;
    lAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Light");
    lAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    lAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    double initialLight = 0.0;
    UA_Variant_setScalar(&lAttr.value, &initialLight, &UA_TYPES[UA_TYPES_DOUBLE]);

    lightNodeId = UA_NODEID_STRING(1, "Light");
    UA_Server_addVariableNode(server,
                               lightNodeId,
                               UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                               UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                               UA_QUALIFIEDNAME(1, "Light"),
                               UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                               lAttr,
                               NULL, NULL);
}
