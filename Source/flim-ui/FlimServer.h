#pragma once

#include <QTcpServer>
#include <QObject>
#include <QTcpSocket>
#include <algorithm>
#include <vector>
#include <QMutex>
#include <QWaitCondition>
#include <map>

#include "FlimServerInterface.h"
#include "ThreadedObject.h"

using namespace ServerInterface;

class OptionalData 
{
public:

   static std::pair<QString, QVariant> process(const T_OPTIONAL_DATA_HEADER& header, const QByteArray& extra_data)
   {
      QString name = header.name;
      QVariant value;
      if (header.type == PQ_OPT_DATATYPE_FLOAT)
         value = *reinterpret_cast<const float*>(extra_data.constData());
      else if (header.type == PQ_OPT_DATATYPE_LONG)
         value = *reinterpret_cast<const int32_t*>(extra_data.constData());
      else if (header.type == PQ_OPT_DATATYPE_LONG)
         value = *reinterpret_cast<const uint32_t*>(extra_data.constData());
      else if (header.type == PQ_OPT_DATATYPE_FLOATS_ARRAY)
         value.setValue(getVector<float, double>(extra_data));
      else if (header.type == PQ_OPT_DATATYPE_LONGS_ARRAY)
         value.setValue(getVector<int32_t, int64_t>(extra_data));
      else if (header.type == PQ_OPT_DATATYPE_ULONGS_ARRAY)
         value.setValue(getVector<int32_t, int64_t>(extra_data));
      else if (header.type == PQ_OPT_DATATYPE_FIXED_LENGTH_STRING)
         value = QString(extra_data);

      return std::make_pair(name, value);
   }

   static QByteArray makeRecord(std::pair<QString, QVariant> pair)
   {
      QByteArray record(sizeof(T_OPTIONAL_DATA_HEADER), 0);
      T_OPTIONAL_DATA_HEADER* header = reinterpret_cast<T_OPTIONAL_DATA_HEADER*>(record.data());
      
      QByteArray name_bytes = pair.first.toLatin1();
      QByteArray temp = name_bytes.leftJustified(PQ_OPT_DATATYPE_NAME_MAX_LEN, 0, true);
      std::copy_n(temp.constData(), PQ_OPT_DATATYPE_NAME_MAX_LEN, header->name);

      QVariant& v = pair.second;

      if (v.type() == QVariant::Type::Double)
      {
         header->type = PQ_OPT_DATATYPE_FLOAT;
         float val = v.value<float>();
         record.append(reinterpret_cast<char*>(&val),sizeof(float));
      }
      else if (v.type() == QVariant::Type::Int)
      {
         header->type = PQ_OPT_DATATYPE_LONG;
         int32_t val = v.value<int32_t>();
         record.append(reinterpret_cast<char*>(&val), sizeof(int32_t));
      }
      else if (v.type() == QVariant::Type::UInt)
      {
         header->type = PQ_OPT_DATATYPE_ULONG;
         uint32_t val = v.value<uint32_t>();
         record.append(reinterpret_cast<char*>(&val), sizeof(uint32_t));
      }
      else if (v.typeName() == QString("std::vector<double>"))
      {
         header->type = PQ_OPT_DATATYPE_FLOATS_ARRAY;
         putVector<float, double>(v, record, header);
      }
      else if (v.typeName() == QString("std::vector<int64_t>"))
      {
         header->type = PQ_OPT_DATATYPE_FLOATS_ARRAY;
         putVector<int32_t, int64_t>(v, record, header);
      }
      else if (v.typeName() == QString("std::vector<uint64_t>"))
      {
         header->type = PQ_OPT_DATATYPE_FLOATS_ARRAY;
         putVector<uint32_t, uint64_t>(v, record, header);
      }
      else if (v.type() == QVariant::Type::String)
      {
         header->type = PQ_OPT_DATATYPE_FIXED_LENGTH_STRING;
         QByteArray&& s = v.toString().toLatin1();
         uint16_t val = v.value<uint16_t>();
         record.append(reinterpret_cast<char*>(&val), sizeof(uint16_t));
         record.append(s);
      }
      else
      {
         throw std::runtime_error("Unsupported variant type");
      }

      return record;
   }

protected:

   template<class T, class U>
   static std::vector<U> getVector(const QByteArray data)
   {
      int n_el = data.size() / sizeof(T);
      std::vector<U> v(n_el);
      const T* d = reinterpret_cast<const T*>(data.constData());
      for (int i = 0; i < n_el; i++)
         v[i] = static_cast<U>(d[i]);
      return v;
   }

   template<class T, class U>
   static void putVector(QVariant v, QByteArray& ba, T_OPTIONAL_DATA_HEADER* header)
   {
      std::vector<U> d = v.value<std::vector<U>>();
      uint16_t n_el = (uint16_t) d.size();
      ba.append(reinterpret_cast<char*>(&n_el), sizeof(int16_t));
      int offset = ba.size();
      ba.resize(offset + n_el * sizeof(T));
      T* t = reinterpret_cast<T*>(ba.data() + offset);
      for (int i = 0; i < n_el; i++)
         t[i] = static_cast<T>(d[i]);
   }

};

class FlimServer : public ThreadedObject 
{
   Q_OBJECT

public:

   void init();
   
   FlimServer(QObject* parent = nullptr);
   void sendProgress(E_PQ_MEAS_TYPE measurement_type, std::vector<std::pair<QString, QVariant>> optional_data);
   void sendMeasurementComplete();
   void sendMesurementRequestResponse(E_ERROR_CODES response);
   void sendError(E_ERROR_CODES error, const QString& msg = "");

signals:
   void measurementRequest(T_DATAFRAME_SRVREQUEST request, std::map<QString, QVariant> optional_data);
   void clientError(const QString);
   void userBreakRequest();

protected:

   void newConnection();
   void processMessage();
   void processClientResponse(E_STOPREASON_CODES, const QString& msg = "");

   void sendStatusMessage(E_PQ_MESSAGE_TYPE message_type, E_ERROR_CODES status, const QString& explainatory_text = "", QTcpSocket* client = nullptr);


   std::map<QString, QVariant> readOptionalData(int opt_record_count);
   QByteArray readData(int len);
   void readData(char* ptr, int len);

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
   T t;
   readData(reinterpret_cast<char*>(&t),sizeof(T));
   return t;
}
