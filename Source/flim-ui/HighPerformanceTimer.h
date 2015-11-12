#pragma once
#include <string>
#include <cstdio>
#include "Windows.h"
class HighPerformanceTimer
{
public:
   HighPerformanceTimer(std::string title)
      : title(title)
   {
      QueryPerformanceFrequency( &frequency );
      dt = 1.0/(frequency.QuadPart);
      acc_t = 0;
      n_iter = 0;
   }

   void Reset()
   {
      acc_t = 0;
      n_iter = 0;
   }

   void Start()
   {
      QueryPerformanceCounter( &t_start );
   }

   void StartNew()
   {
      Reset();
      Start();
   }

   float GetElapsed()
   {
      QueryPerformanceCounter(&t_end);
      return static_cast<double>((t_end.QuadPart-t_start.QuadPart)) * dt * 1000;
   }

   float Stop()
   {
      QueryPerformanceCounter(&t_end);

      acc_t += static_cast<double>((t_end.QuadPart-t_start.QuadPart)) * dt;
      n_iter++;


      double t_ms = 1000.0 * acc_t / n_iter;

      //if ((n_iter % 1000) == 0)
      //   std::cout << "Timing for: " << title << " : " << t_ms << "ms\n";

      return t_ms;
   }


private:

   double acc_t;
   double dt;
   int n_iter;
   LARGE_INTEGER frequency;
   LARGE_INTEGER t_start;
   LARGE_INTEGER t_end;
   std::string title;

};