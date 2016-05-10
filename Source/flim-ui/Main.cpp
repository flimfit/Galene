#include <QApplication>
#include <QCoreApplication>
#include "FlimDisplay.h"
#include "BHRatesWidget.h"
#include <memory>
#include <vector>
#include <iostream>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)

{
   QByteArray localMsg = msg.toLocal8Bit();

   switch (type)
   {
   case QtInfoMsg:
      fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;
      
   case QtDebugMsg:
      fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;

   case QtWarningMsg:
      fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;

   case QtCriticalMsg:
      fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;

   case QtFatalMsg:
      fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      abort();
         
   }
}

// timetagger4_user_guide_example.cpp : Example application for the TimeTagger4
#include "timetagger4_interface.h"
#include "stdio.h"
#include <windows.h>

typedef unsigned int uint32;
typedef unsigned __int64 uint64;

int main0()
{
   // prepare initialization
   timetagger4_init_parameters params;
   timetagger4_get_default_init_parameters(&params);
   params.buffer_size[0] = 8 * 1024 * 1024;	// use 8 MByte as packet buffer
   params.board_id = 0;						// value copied to "card" field of every packet, allowed range 0..255
   params.card_index = 0;						// initialize first TimeTagger4 board found in the system

   // initialize card
   int error_code;
   const char * err_message;
   timetagger4_device * device = timetagger4_init(&params, &error_code, &err_message);
   if (error_code != CRONO_OK)
   {
      printf("Could not init TimeTagger4 compatible board: %s\n", err_message);
      return 1;
   }

   // prepare configuration
   timetagger4_configuration config;

   // fill configuration data structure with default values
   // so that the configuration is valid and only parameters
   // of interest have to be set explicitly
   timetagger4_get_default_configuration(device, &config);

   // set config of the 4 TDC channels
   for (int i = 0; i < TIMETAGGER4_TDC_CHANNEL_COUNT; i++)
   {
      // enable recording hits on channel
      config.channel[i].enabled = true;

      // define range of the group
      config.channel[i].start = 0;	// range begins right after start pulse
      config.channel[i].stop = 30000;	// recording window stops after ~15 us

      // measure only falling edge
      config.trigger[i + 1].falling = 1;
      config.trigger[i + 1].rising = 1;

      config.dc_offset[i + 1] = 1;
   }

   // start group on falling edges on the start channel
   config.trigger[0].falling = 0;	// enable packet generation on falling edge of start pulse
   config.trigger[0].rising = 1;	// disable packet generation on rising edge of start pulse
   config.dc_offset[0] = TIMETAGGER4_DC_OFFSET_N_NIM;

   // write configuration to board
   int status = timetagger4_configure(device, &config);
   if (status != CRONO_OK)
   {
      printf("Could not configure TimeTagger4: %s\n", err_message);
      return error_code;
   }

   // print board information
   timetagger4_static_info staticinfo;
   timetagger4_get_static_info(device, &staticinfo);
   printf("Board Serial        : %d.%d\n", staticinfo.board_serial >> 24, staticinfo.board_serial & 0xffffff);
   printf("Board Configuration : TimeTagger4-");
   switch (staticinfo.board_configuration&TIMETAGGER4_BOARDCONF_MASK)
   {
   case TIMETAGGER4_1G_BOARDCONF:
      printf("1G\n"); break;
   case TIMETAGGER4_2G_BOARDCONF:
      printf("2G\n"); break;
   default:
      printf("unknown\n");
   }
   printf("Board Revision      : %d\n", staticinfo.board_revision);
   printf("Firmware Revision   : %d.%d\n", staticinfo.firmware_revision, staticinfo.subversion_revision);
   printf("Driver Revision     : %d.%d.%d.%d\n", ((staticinfo.driver_revision >> 24) & 255), ((staticinfo.driver_revision >> 16) & 255), ((staticinfo.driver_revision >> 8) & 255), (staticinfo.driver_revision & 255));

   timetagger4_param_info parinfo;
   timetagger4_get_param_info(device, &parinfo);
   printf("\nTDC binsize         : %0.2f ps\n", parinfo.binsize);

   // configure readout behaviour
   timetagger4_read_in read_config;
   // automatically acknowledge all data as processed
   // on the next call to timetagger4_read()
   // old packet pointers are invalid after calling timetagger4_read()
   read_config.acknowledge_last_read = 1;

   // structure with packet pointers for read data
   timetagger4_read_out read_data;

   // start data capture
   status = timetagger4_start_capture(device);
   if (status != CRONO_OK) {
      printf("Could not start capturing %s", timetagger4_get_last_error_message(device));
      timetagger4_close(device);
      return status;
   }

   // some book keeping
   int packet_count = 0;
   int empty_packets = 0;
   int packets_with_errors = 0;
   bool last_read_no_data = false;

   _int64 GroupAbsTime = 0;
   _int64 GroupAbsTime_old = 0;
   int UpdateCount = 1000;

   printf("Reading packets:\n");
   // read 10000 packets
   while (packet_count < 1e10)
   {
      // get pointers to acquired packets
      status = timetagger4_read(device, &read_config, &read_data);
      if (status != CRONO_OK)
      {
         Sleep(100);
         //printf(" - No data! -\n");
      }
      else
      {
         // iterate over all packets received with the last read
         crono_packet* p = read_data.first_packet;
         while (p <= read_data.last_packet)
         {
            // do something with the data, e.g. calculate current rate
            GroupAbsTime = p->timestamp;
            if (packet_count%UpdateCount == 0) {
               // group timestamp increments at 2 GHz
               double Rate = (2e9 / ((double)(GroupAbsTime - GroupAbsTime_old) / (double)UpdateCount));
               printf("\r%.2f MHz\n", Rate / 1e6);
               GroupAbsTime_old = GroupAbsTime;
            }

//            if (p->flags > 1)
//               printf("\r ----> Flag: %d\n", p->flags);
            
            // ...or print hits (not a good idea at high data rates,
            // so let's do that only for every 1000th packet...)
            // print packet information
            if (true) {
               //printf("Card %d - Flags %d - Length %d - Type %d - TS %llu\n", p->card, p->flags, p->length, p->type, p->timestamp);

               int hit_count = 2 * (p->length);
               if ((p->flags & 0x1) == 1)
                  hit_count -= 1;

               uint32* packet_data = (uint32*)(p->data);
               for (int i = 0; i < hit_count; i++)
               {
                  uint32* hit = (packet_data + i);

                  // extract channel number (A-D)
                  char channel = 65 + (*hit & 0xf);

                  // extract hit flags
                  int flags = (*hit >> 4 & 0xf);

                  // extract hit timestamp
                  int ts_offset = (*hit >> 8 & 0xffffff);

                  // TDC bin size is 500 ps. Convert timestamp to ns.
                  double ts_offset_ns = ts_offset;
                  ts_offset_ns *= parinfo.binsize / 1000.0;
                  /*
                  if (*hit & 0x10)
                     printf("Rising \n");
                  else
                     printf("Falling \n\n");
                     */
                  //printf("Hit @Channel %c - Flags %d - Offset %u (raw) / %f ns\n", channel, flags, ts_offset, ts_offset_ns);
               }
               //printf("\n\n");
            }

            // go to next packet
            p = crono_next_packet(p);
            packet_count++;
         }
      }
   }

   // shut down packet generation and DMA transfers
   timetagger4_stop_capture(device);

   // deactivate timetagger4
   timetagger4_close(device);

   return 0;
}

int main(int argc, char *argv[])
{
 //  main0();
 //  return 0;

   qRegisterMetaType<cv::Point2d>("cv::Point2d");
   qRegisterMetaType<FlimRates>("FlimRates");
   qRegisterMetaType<std::vector<int64_t>>("std::vector<int32_t>");
   qRegisterMetaType<std::vector<uint64_t>>("std::vector<uint32_t>");
   qRegisterMetaType<std::vector<double>>("std::vector<double>");
   qRegisterMetaType<T_DATAFRAME_SRVREQUEST>("T_DATAFRAME_SRVREQUEST");
   qRegisterMetaType<E_ERROR_CODES>("E_ERROR_CODES");
   qRegisterMetaType<E_PQ_MEAS_TYPE>("E_PQ_MEAS_TYPE");
   qRegisterMetaType<std::map<QString,QVariant>>("std::map<QString, QVariant>");
   qRegisterMetaType<std::vector<std::pair<QString, QVariant>>>("std::vector<std::pair<QString, QVariant>>");

   QCoreApplication::setOrganizationName("Garvan Institute of Medical Research");
   QCoreApplication::setOrganizationDomain("garvan.org.au");
   QCoreApplication::setApplicationName("FifoFlim");

   qInstallMessageHandler(myMessageOutput);

   QApplication a(argc, argv);
       
   FlimDisplay display;
   display.showMaximized();

   a.exec();

   return 0;
}