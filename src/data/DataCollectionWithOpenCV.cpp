//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 01-Apr-21.
//

#include <comm/data/DataCollectionWithOpenCV.h>
#include <comm/data/dataUtilsWithOpenCV.h>

using namespace comm;
using namespace std;

DataCollectionWithOpenCV::DataCollectionWithOpenCV() : DataCollection(createCommunicationDataPtrWithOpenCV) {}

DataCollectionWithOpenCV::DataCollectionWithOpenCV(function<CommunicationData *(MessageType)> dataCreationFunction)
        : DataCollection(move(dataCreationFunction)) {}

DataCollectionWithOpenCV::~DataCollectionWithOpenCV() = default;
