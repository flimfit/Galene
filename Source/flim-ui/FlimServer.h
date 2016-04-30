#pragma once

#include <QTcpServer>
#include <QObject>
#include <QTcpSocket>
#include "FlimServerInterface.h"
#include <algorithm>
#include <QMutex>
#include <QWaitCondition>

using namespace ServerInterface;




class OptionalData 
{
public:
   OptionalData(const T_OPTIONAL_DATA_HEADER& header, const QByteArray& extra_data)
   {

   }

   int32_t getSize()
   {
      return sizeof(T_OPTIONAL_DATA_HEADER) + extra_data.size();
   }

   T_OPTIONAL_DATA_HEADER header;
   QByteArray extra_data;
};


class FlimServer : public QObject 
{
   Q_OBJECT

public:
   
   FlimServer(QObject* parent = nullptr);
   void sendProgress(E_PQ_MEAS_TYPE measurement_type, std::vector<OptionalData> optional_data);
   void sendMeasurementComplete();
   void sendMesurementRequestResponse(E_ERROR_CODES response);
   void sendError(E_ERROR_CODES error);

signals:
   void measurementRequest(T_DATAFRAME_SRVREQUEST request, std::vector<OptionalData> optional_data);
   void clientError(const QString);
   void userBreakRequest();

protected:

   void newConnection();
   void processMessage();
   void processClientResponse(E_STOPREASON_CODES, const QString& msg = "");

   void sendStatusMessage(E_PQ_MESSAGE_TYPE message_type, E_ERROR_CODES status, const std::string& explainatory_text = "", QTcpSocket* client = nullptr);


   std::vector<OptionalData> readOptionalData(int opt_record_count);
   QByteArray readData(int len);

   template<class T>
   T readStruct(bool retry = true);

   QTcpServer *server;
   QTcpSocket *client = nullptr;
   const int port = 6000;
   const int timeout_ms = 4000;

   int32_t nack_rec_number = 0;
};

template<class T>
T FlimServer::readStruct(bool retry)
{
   if (client->bytesAvailable() > sizeof(T))
   {
      T data;
      client->read((char*)&data, sizeof(T));
      return data;
   }
   else if (retry)
   {
      client->waitForReadyRead(timeout_ms);
      return readStruct<T>(false);
   }
   else
   {
      T t; // todo garbage
      return t;
   }

}
