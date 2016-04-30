#pragma once

#include <cstdint>

namespace ServerInterface
{

   typedef uint8_t E_PQ_MESSAGE_TYPE ;
   typedef int32_t E_PQ_MEAS_TYPE ;
   typedef uint8_t E_PQ_OPT_DATATYPE ;
   typedef int16_t E_ERROR_CODES ;
   typedef int16_t E_STOPREASON_CODES ;

   const int PQ_OPT_DATATYPE_NAME_MAX_LEN = 30;

   // E_PQ_MESSAGE_TYPE
   const E_PQ_MESSAGE_TYPE PQ_MSGTYP_DATAFRAME_SRVREQUEST = 0x44;
   const E_PQ_MESSAGE_TYPE PQ_MSGTYP_DATAFRAME_SRVREPLY = 0x64;
   const E_PQ_MESSAGE_TYPE PQ_MSGTYP_DATAFRAME_SRVNACK = 0x78;
   const E_PQ_MESSAGE_TYPE PQ_MSGTYP_ENCODED_STATUSMSG = 0x43;
   const E_PQ_MESSAGE_TYPE PQ_MSGTYP_ENCODED_STATUSREPLY = 0x63;
   const E_PQ_MESSAGE_TYPE PQ_MSGTYP_EXPLAINED_STATUSMSG = 0x53;
   const E_PQ_MESSAGE_TYPE PQ_MSGTYP_EXPLAINED_STATUSREPLY = 0x73;

   // E_PQ_MEAS_TYPE
   const E_PQ_MEAS_TYPE PQ_MEASTYPE_POINTMEAS = 0x00000000;
   const E_PQ_MEAS_TYPE PQ_MEASTYPE_IMAGESCAN = 0x00000001;
   const E_PQ_MEAS_TYPE PQ_MEASTYPE_TEST_POINTMEAS = 0x00000080;
   const E_PQ_MEAS_TYPE PQ_MEASTYPE_TEST_IMAGESCAN = 0x00000081;

   // E_PQ_OPT_DATATYPE
   const E_PQ_OPT_DATATYPE PQ_OPT_DATATYPE_FLOAT = 0x00;
   const E_PQ_OPT_DATATYPE PQ_OPT_DATATYPE_LONG = 0x01;
   const E_PQ_OPT_DATATYPE PQ_OPT_DATATYPE_ULONG = 0x02;
   const E_PQ_OPT_DATATYPE PQ_OPT_DATATYPE_FLOATS_ARRAY = 0xF0;
   const E_PQ_OPT_DATATYPE PQ_OPT_DATATYPE_LONGS_ARRAY = 0xF1;
   const E_PQ_OPT_DATATYPE PQ_OPT_DATATYPE_ULONGS_ARRAY = 0xF2;
   const E_PQ_OPT_DATATYPE PQ_OPT_DATATYPE_FIXED_LENGTH_STRING = 0xFF;

   // E_STOPREASON_CODES
   const E_STOPREASON_CODES PQ_STOPREASON_CODE_CONTINUE_OK = 0;
   const E_STOPREASON_CODES PQ_STOPREASON_CODE_FINISHED_OK = 1;
   const E_STOPREASON_CODES PQ_STOPREASON_CODE_USER_BREAK = 2;
   const E_STOPREASON_CODES PQ_STOPREASON_CODE_ERROR = -1;

   // E_ERROR_CODES
   const E_ERROR_CODES PQ_ERRCODE_NO_ERROR = 0;
   const E_ERROR_CODES PQ_ERRCODE_MEASUREMENT_READY = 1;
   const E_ERROR_CODES PQ_ERRCODE_USER_BREAK = 2;
   const E_ERROR_CODES PQ_ERRCODE_MESSAGE_CORRUPTED = -1;
   const E_ERROR_CODES PQ_ERRCODE_SERVER_BUSY = -2;
   const E_ERROR_CODES PQ_ERRCODE_MESSAGE_TIMEOUT = -3;
   const E_ERROR_CODES PQ_ERRCODE_INVALID_REC_VERSION = -10;
   const E_ERROR_CODES PQ_ERRCODE_MEASUREMENT_TIMEOUT = -100;
   const E_ERROR_CODES PQ_ERRCODE_FIFO_OVERRUN = -101;
   const E_ERROR_CODES PQ_ERRCODE_DMA_ERROR = -102;
   const E_ERROR_CODES PQ_ERRCODE_OSCILLOSCOPE_RUNNING = -103;
   const E_ERROR_CODES PQ_ERRCODE_HARDWARE_INIT = -104;
   const E_ERROR_CODES PQ_ERRCODE_TTTR_INIT = -105;
   const E_ERROR_CODES PQ_ERRCODE_TTTR_RUNNING = -106;
   const E_ERROR_CODES PQ_ERRCODE_NO_WORKSPACE = -107;
   const E_ERROR_CODES PQ_ERRCODE_FILE_EXISTS = -108;
   const E_ERROR_CODES PQ_ERRCODE_FILE_CREATE = -109;
   const E_ERROR_CODES PQ_ERRCODE_GROUPNAME_TOO_LONG = -110;
   const E_ERROR_CODES PQ_ERRCODE_FILENAME_TOO_LONG = -111;
   const E_ERROR_CODES PQ_ERRCODE_TIMESTAMP_ARRAY_TOO_LONG = -112;
   const E_ERROR_CODES PQ_ERRCODE_INVALID_LICENSE = -999;
   const E_ERROR_CODES PQ_ERRCODE_UNKNOWN_ERROR = -9999;

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

   struct T_ENCODED_STATUSMSG
   {
      T_MESSAGEHDR hdr; // obligatory message header identifying message type
      E_ERROR_CODES status; // status code
   };

   struct T_ENCODED_STATUSREPLY
   {
      T_MESSAGEHDR hdr; // obligatory message header identifying message type
      E_STOPREASON_CODES status; // status code
   };


   struct T_EXPLAINED_STATUSMSG
   {
      T_MESSAGEHDR hdr; // obligatory message header identifying message type
      E_ERROR_CODES status; // status code
      uint32_t exp_length; // length of explaining text;
   };

   struct T_EXPLAINED_STATUSREPLY
   {
      T_MESSAGEHDR hdr; // obligatory message header identifying message type
      E_STOPREASON_CODES status; // status code
      uint32_t exp_length; // length of explaining text;
   };

   #pragma pack(pop)
}

