#include "FlimServer.h"
#include <QThread>
#include <cassert>
#include <iostream>
#include <QMetaType>

FlimServer::FlimServer(QObject* parent) :
   ThreadedObject(parent)
{
   startThread();
}

void FlimServer::init()
{
   server = new QTcpServer(this);
   if (!server->listen(QHostAddress::Any, port))
   {
      qWarning() << server->errorString();
      return;
   }

   connect(server, &QTcpServer::newConnection, this, &FlimServer::newConnection);
}

void FlimServer::sendStatusMessage(E_PQ_MESSAGE_TYPE message_type, E_ERROR_CODES status, const QString& explainatory_text, QTcpSocket* client)
{
   if (client == nullptr)
      client = this->client;
   if (client == nullptr)
      return;

   T_MESSAGEHDR hdr;
   hdr.msg_type = message_type;
   std::memcpy(hdr.magic, "PQSPT", 5);

   if (explainatory_text.isEmpty())
   {
      hdr.msg_len = sizeof(T_ENCODED_STATUSMSG) + sizeof(hdr);

      T_ENCODED_STATUSMSG msg;
      msg.status = status;
      std::cout << "sending T_ENCODED_STATUSMSG\n";
      client->write((char*)&hdr, sizeof(hdr)); 
      client->write((char*)&msg, sizeof(msg));
   }
   else
   {
      int max_explain_size = std::numeric_limits<uint16_t>::max() - sizeof(T_EXPLAINED_STATUSMSG);
      uint16_t explain_size = (uint16_t) std::min(max_explain_size, explainatory_text.size());

      hdr.msg_len = sizeof(T_EXPLAINED_STATUSMSG) + sizeof(hdr) + explain_size;

      std::cout << "sending T_EXPLAINED_STATUSMSG\n";

      T_EXPLAINED_STATUSMSG msg;
      msg.status = status;
      msg.exp_length = explain_size;

      client->write((char*)&hdr, sizeof(hdr));
      client->write((char*)&msg, sizeof(msg));
      client->write(explainatory_text.toLatin1(), explainatory_text.size());
   }

}

void FlimServer::sendProgress(E_PQ_MEAS_TYPE measurement_type, std::vector<std::pair<QString, QVariant>> optional_data)
{
   if (client == nullptr)
      return;

   assert(optional_data.size() < std::numeric_limits<int32_t>::max());

   std::cout << "sending PQ_MSGTYP_DATAFRAME_SRVNACK\n";


   T_MESSAGEHDR hdr;
   
   T_DATAFRAME_SRVNACK frame;
   hdr.msg_type = PQ_MSGTYP_DATAFRAME_SRVNACK;
   std::memcpy(hdr.magic, "PQSPT", 5);
   frame.rec_version = 0x01000200;
   frame.measurement_type = measurement_type;
   frame.nack_rec_number = nack_rec_number++;
   frame.opt_record_count = (int32_t) optional_data.size();


   QByteArray all_optional;
   for (auto&& od : optional_data)
      all_optional.append(OptionalData::makeRecord(od));

   hdr.msg_len = sizeof(frame) + sizeof(hdr) + all_optional.size();

   const char* a = (char*)&frame;
   const char* b = all_optional.constData();

   client->write((char*)&hdr, sizeof(hdr));
   client->write((char*)&frame, sizeof(frame));
   client->write(all_optional);
}

void FlimServer::sendError(E_ERROR_CODES error, const QString& msg)
{
   sendStatusMessage(PQ_MSGTYP_ENCODED_STATUSMSG, error, msg);
}

void FlimServer::sendMesurementRequestResponse(E_ERROR_CODES response)
{
   sendStatusMessage(PQ_MSGTYP_DATAFRAME_SRVREPLY, response);
}

void FlimServer::sendMeasurementComplete()
{
   sendStatusMessage(PQ_MSGTYP_ENCODED_STATUSMSG, PQ_ERRCODE_NO_ERROR);
}

void FlimServer::newConnection()
{
   std::cout << "new connection\n";

   // Check that we're not already connected to a client
   if (client != nullptr)
   {
      QTcpSocket *new_client = server->nextPendingConnection();
      connect(new_client, &QAbstractSocket::disconnected, new_client, &QObject::deleteLater);
      sendStatusMessage(PQ_MSGTYP_ENCODED_STATUSMSG, PQ_ERRCODE_SERVER_BUSY, "", new_client);
      new_client->disconnectFromHost();
   }

   client = server->nextPendingConnection();
   connect(client, &QAbstractSocket::disconnected, client, &QObject::deleteLater);
   connect(client, &QAbstractSocket::disconnected, [this]() { client = nullptr; });
   connect(client, &QAbstractSocket::readyRead, this, &FlimServer::processMessage);

   sendStatusMessage(PQ_MSGTYP_ENCODED_STATUSMSG, PQ_ERRCODE_NO_ERROR);
}

void FlimServer::processMessage()
{
   while (client != nullptr && client->bytesAvailable())
   {
      auto header = readStruct<T_MESSAGEHDR>();
      if (header.msg_type == PQ_MSGTYP_DATAFRAME_SRVREQUEST)
      {
         auto request = readStruct<T_DATAFRAME_SRVREQUEST>();
         auto optional_data = readOptionalData(request.opt_record_count);
         std::cout << "new PQ_MSGTYP_DATAFRAME_SRVREQUEST\n";
         emit measurementRequest(request, optional_data);
      }
      else if (header.msg_type == PQ_MSGTYP_ENCODED_STATUSMSG)
      {
         auto msg = readStruct<T_ENCODED_STATUSMSG>();
         std::cout << "new PQ_MSGTYP_ENCODED_STATUSMSG : " << msg.status << "\n";
         processClientResponse(msg.status);
      }
      else if (header.msg_type == PQ_MSGTYP_EXPLAINED_STATUSMSG)
      {
         auto msg = readStruct<T_EXPLAINED_STATUSMSG>();
         QByteArray msg_bytes = readData(msg.exp_length);
         std::cout << "new PQ_MSGTYP_EXPLAINED_STATUSMSG : " << msg.status << "\n";
         processClientResponse(msg.status, msg_bytes);
      }
      else if (header.msg_type == PQ_MSGTYP_ENCODED_STATUSREPLY)
      {
         auto msg = readStruct<T_ENCODED_STATUSMSG>();
         std::cout << "new PQ_MSGTYP_ENCODED_STATUSREPLY : " << msg.status << "\n";
      }
      else if (header.msg_type == PQ_MSGTYP_EXPLAINED_STATUSREPLY)
      {
         auto msg = readStruct<T_EXPLAINED_STATUSREPLY>();
         QByteArray msg_bytes = readData(msg.exp_length);
         std::cout << "new PQ_MSGTYP_EXPLAINED_STATUSREPLY : " << msg.status << "\n";
      }
      else
      {
         std::cout << "unknown message: " << header.msg_type << "\n";
      }
   }
}

void FlimServer::processClientResponse(E_STOPREASON_CODES status, const QString& msg)
{
   switch (status)
   {
   case PQ_STOPREASON_CODE_CONTINUE_OK:
      break;
   case PQ_STOPREASON_CODE_FINISHED_OK:
   case PQ_STOPREASON_CODE_USER_BREAK:
      emit userBreakRequest();
      sendStatusMessage(PQ_MSGTYP_ENCODED_STATUSREPLY, PQ_ERRCODE_NO_ERROR);
      break;
   case PQ_STOPREASON_CODE_ERROR:
      emit clientError(msg);
   }
}



std::map<QString, QVariant> FlimServer::readOptionalData(int opt_record_count)
{
   std::map<QString, QVariant> optional_data;
   for (int i = 0; i < opt_record_count; i++)
   {
      QByteArray extra_data;
      int a = sizeof(T_OPTIONAL_DATA_HEADER);
      auto header = readStruct<T_OPTIONAL_DATA_HEADER>();
      if (header.type & 0xF) // this is pretty ugly - but then so is the format
      {
         uint16_t extra_data_size = readStruct<uint16_t>();
         if (header.type == PQ_OPT_DATATYPE_FIXED_LENGTH_STRING)
            extra_data = readData(extra_data_size);
         else
            extra_data = readData(extra_data_size * 4);
      }
      else
      {
         extra_data = readData(4);
      }
      optional_data.insert(OptionalData::process(header, extra_data));
   }
   return optional_data;
}

QByteArray FlimServer::readData(int len)
{
   QByteArray data;
   while (client != nullptr && client->bytesAvailable() < len)
      client->waitForReadyRead(timeout_ms);
   
   if (client != nullptr)
      data = client->read(len);
   else
      data.fill(0, len);
   return data;
}

void FlimServer::readData(char* ptr, int len)
{
   while (client != nullptr && client->bytesAvailable() < len)
      client->waitForReadyRead(timeout_ms);
 
   if (client != nullptr)
      client->read(ptr, len);
   else
      std::fill(ptr, ptr + len, 0);
}