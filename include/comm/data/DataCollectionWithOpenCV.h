//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 01-Apr-21.
//

#ifndef COMM_DATA_DATACOLLECTIONWITHOPENCV_H
#define COMM_DATA_DATACOLLECTIONWITHOPENCV_H

#include <comm/data/DataCollection.h>

namespace comm {
    class DataCollectionWithOpenCV : public DataCollection {
    public:
        DataCollectionWithOpenCV();

        DataCollectionWithOpenCV(std::function<CommunicationData*(MessageType)> dataCreationFunction);

        ~DataCollectionWithOpenCV() override;
    };
}


#endif //COMM_DATA_DATACOLLECTIONWITHOPENCV_H
