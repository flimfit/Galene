#pragma once

#include <QTcpServer>
#include <QObject>
#include <QTcpSocket>

#define E_PQ_MESSAGE_TYPE uint8_t
#define E_PQ_MEAS_TYPE int32_t
#define E_PQ_OPT_DATATYPE uint8_t
#define E_ERROR_CODES int16_t
#define E_STOPREASON_CODES int16_t

const int PQ_OPT_DATATYPE_NAME_MAX_LEN = 30;

#pragma pack(push, 1)

struct T_MESSAGEHDR
{
   uint16_t msg_len;
   E_PQ_MESSAGE_TYPE msg_type;
   char magic[5];
};

struct T_DATAFRAME_SRVREQUEST
{
   T_MESSAGEHDR hdr; //obligatory message header identifying this message type
   char rec_version[4]; //for future record version management current record version is 1.0.2.0   (i.e.  0x00, 0x02, 0x00, 0x01  on stream)
   E_PQ_MEAS_TYPE measurement_type; //type of requested measurement
   int32_t n_x; //if type of measurement is image, this denotes the image width
   int32_t n_y; //if type of measurement is image, this denotes the image height
   int32_t scanning_pattern; //0:   monodirectional  1 : bi - directional

   float spatial_resolution; //if type of measurement is image, this denotes the size of the pixels in units of m.
   int32_t opt_record_count; //number of optional data records to follow
};

struct T_DATAFRAME_SRVNACK
{
   T_MESSAGEHDR hdr; //obligatory message header identifying this message type
   char rec_version[4]; //for future record version management current record version is 1.0.2.0   (i.e.  0x00, 0x02, 0x00, 0x01  on stream)
   E_PQ_MEAS_TYPE measurement_type; //type of running measurement
   int32_t nack_rec_number; //consecutive increasing record number
   int32_t opt_record_count; //number of optional data records to follow
};

struct T_OPTIONAL_DATA_HEADER
{
   char name[PQ_OPT_DATATYPE_NAME_MAX_LEN];
   E_PQ_OPT_DATATYPE type;
   uint32_t data;
};

#pragma pack(pop)

// E_PQ_MESSAGE_TYPE
const uint8_t PQ_MSGTYP_DATAFRAME_SRVREQUEST = 0x44;
const uint8_t PQ_MSGTYP_DATAFRAME_SRVREPLY = 0x64;
const uint8_t PQ_MSGTYP_DATAFRAME_SRVNACK = 0x78;
const uint8_t PQ_MSGTYP_ENCODED_STATUSMSG = 0x43;
const uint8_t PQ_MSGTYP_ENCODED_STATUSREPLY = 0x63;
const uint8_t PQ_MSGTYP_EXPLAINED_STATUSMSG = 0x53;
const uint8_t PQ_MSGTYP_EXPLAINED_STATUSREPLY = 0x73;

// E_PQ_MES_TYPE
const uint32_t PQ_MEASTYPE_POINTMEAS = 0x00000000;
const uint32_t PQ_MEASTYPE_IMAGESCAN = 0x00000001;
const uint32_t PQ_MEASTYPE_TEST_POINTMEAS = 0x00000080;
const uint32_t PQ_MEASTYPE_TEST_IMAGESCAN = 0x00000081;

// E_PQ_OPT_DATATYPE
const uint8_t PQ_OPT_DATATYPE_FLOAT = 0x00;
const uint8_t PQ_OPT_DATATYPE_LONG = 0x01;
const uint8_t PQ_OPT_DATATYPE_ULONG = 0x02;
const uint8_t PQ_OPT_DATATYPE_FLOATS_ARRAY = 0xF0;
const uint8_t PQ_OPT_DATATYPE_LONGS_ARRAY = 0xF1;
const uint8_t PQ_OPT_DATATYPE_ULONGS_ARRAY = 0xF2;
const uint8_t PQ_OPT_DATATYPE_FIXED_LENGTH_STRING = 0xFF;

// E_STOPREASON_CODES
const int16_t PQ_STOPREASON_CODE_CONTINUE_OK = 0;
const int16_t PQ_STOPREASON_CODE_FINISHED_OK = 1;
const int16_t PQ_STOPREASON_CODE_USER_BREAK = 2;
const int16_t PQ_STOPREASON_CODE_ERROR = -1;

// E_ERROR_CODES
const int16_t PQ_ERRCODE_NO_ERROR = 0;
const int16_t PQ_ERRCODE_MEASUREMENT_READY = 1;
const int16_t PQ_ERRCODE_USER_BREAK = 2;
const int16_t PQ_ERRCODE_MESSAGE_CORRUPTED = -1;
const int16_t PQ_ERRCODE_SERVER_BUSY = -2;
const int16_t PQ_ERRCODE_MESSAGE_TIMEOUT = -3;
const int16_t PQ_ERRCODE_INVALID_REC_VERSION = -10;
const int16_t PQ_ERRCODE_MEASUREMENT_TIMEOUT = -100;
const int16_t PQ_ERRCODE_FIFO_OVERRUN = -101;
const int16_t PQ_ERRCODE_DMA_ERROR = -102;
const int16_t PQ_ERRCODE_OSCILLOSCOPE_RUNNING = -103;
const int16_t PQ_ERRCODE_HARDWARE_INIT = -104;
const int16_t PQ_ERRCODE_TTTR_INIT = -105;
const int16_t PQ_ERRCODE_TTTR_RUNNING = -106;
const int16_t PQ_ERRCODE_NO_WORKSPACE = -107;
const int16_t PQ_ERRCODE_FILE_EXISTS = -108;
const int16_t PQ_ERRCODE_FILE_CREATE = -109;
const int16_t PQ_ERRCODE_GROUPNAME_TOO_LONG = -110;
const int16_t PQ_ERRCODE_FILENAME_TOO_LONG = -111;
const int16_t PQ_ERRCODE_TIMESTAMP_ARRAY_TOO_LONG = -112;
const int16_t PQ_ERRCODE_INVALID_LICENSE = -999;
const int16_t PQ_ERRCODE_UNKNOWN_ERROR = -9999;

class OptionalData 
{

};


class FlimServer : public QObject 
{
   Q_OBJECT

public:
   
   FlimServer(QObject* parent = nullptr) : 
      QObject(parent)
   {
      server = new QTcpServer(this);
      if (!server->listen(QHostAddress::Any, port)) 
      {
         qWarning() << server->errorString();
         return;
      }

      connect(server, &QTcpServer::newConnection, this, &FlimServer::newConnection);

   }

protected:

   void newConnection()
   {
      // could send status message here

      // Check that we're not already connected to a client
      if (client != nullptr)
      {
         QTcpSocket *client_connection = server->nextPendingConnection();
         connect(client, &QAbstractSocket::disconnected, client, &QObject::deleteLater);
         //TODO: sendErrorMessage(client_connection, PQ_ERRCODE_SERVER_BUSY);
         client_connection->disconnectFromHost();
      }

      client = server->nextPendingConnection();
      connect(client, &QAbstractSocket::disconnected, client, &QObject::deleteLater);
      connect(client, &QAbstractSocket::disconnected, [this]() { client = nullptr; });
      connect(client, &QAbstractSocket::readyRead, this, &FlimServer::processMessage);
   }

   void processMessage()
   {
      auto header = readStruct<T_MESSAGEHDR>();
      if (header.msg_type == PQ_MSGTYP_DATAFRAME_SRVREQUEST)
      {
         auto request = readStruct<T_DATAFRAME_SRVREQUEST>();
         auto optional_data = readOptionalData(request.opt_record_count);
      }
   }

   std::vector<OptionalData> readOptionalData(int opt_record_count)
   {
      std::vector<OptionalData> optional_data;
      for (int i = 0; i < opt_record_count; i++)
      {
         auto header = readStruct<T_OPTIONAL_DATA_HEADER>();
         if (header.type & 0xF)
            QByteArray sup_data = client->read(header.data);
      }
   }

   template<class T> 
   T readStruct(bool retry = true)
   {
      if (client->bytesAvailable() > sizeof(T))
      {
         T data;
         client->read((char*) &data, sizeof(T));
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

   QTcpServer *server;
   QTcpSocket *client = nullptr;
   const int port = 6000;
   const int timeout_ms = 4000;
};