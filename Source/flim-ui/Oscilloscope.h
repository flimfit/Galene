#ifndef OSCILLOSCOPE_
#define OSCILLOSCOPE_

#include <QVector>

template<class T>

//Circular buffer used for plotting on the screen

class Oscilloscope
{

public:
   Oscilloscope(int buffer_length)
   {
      idx = 0;
      y = QVector<T>(buffer_length);//std::numeric_limits<double>::quiet_NaN());
      x = QVector<double>(buffer_length);//std::numeric_limits<double>::quiet_NaN());
      it_y = y.begin();
      it_x = x.begin();
   };

   void AddPoint(T p)
   {
      *it_y = p;
      *it_x = (idx++);

      if (++it_y == y.end())
         it_y = y.begin();
      if (++it_x == x.end())
         it_x = x.begin();
   };

   QVector<double> GetX() { return x; }
   QVector<T> GetY() { return y; }

   void GetTrace(QVector<double>& tr_x, QVector<T>& tr_y) const
   {
      int n = x.size();

      tr_x.resize(n);
      tr_y.resize(n);

      for (int i = 0; i < n; i++)
      {
         tr_x[i] = x[i];
         tr_y[i] = y[i];
      }
   }


private:
   QVector<T> y;
   QVector<double> x;
   typename QVector<T>::iterator it_y;
   typename QVector<double>::iterator it_x;
   int idx;
};

#endif