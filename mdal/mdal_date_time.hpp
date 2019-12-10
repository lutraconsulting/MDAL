#ifndef MDAL_DATE_TIME_HPP
#define MDAL_DATE_TIME_HPP

#include <string>
#include <vector>

namespace MDAL
{

  class Duration
  {
    public:
      enum Unit
      {
        milliseconds = 0,
        seconds,
        minutes,
        hours,
        days,
        weeks,
        months_CF,
        exact_years
      };

      Duration();

      ///TODO :  move operation

      Duration( double duration, Unit unit );
      Duration( const Duration &other );

      double value( Unit unit ) const;

      Duration &operator=( const Duration &other );
      Duration operator-( const Duration &other ) const;
      Duration operator+( const Duration &other ) const;
      Duration &operator+=( const Duration &other );
      Duration &operator-=( const Duration &other );
      bool operator==( const Duration &other ) const;
      bool operator!=( const Duration &other ) const;
      bool operator<( const Duration &other ) const;
      bool operator>( const Duration &other ) const;
      bool operator>=( const Duration &other ) const;
      bool operator<=( const Duration &other ) const;

    private:
      Duration( int64_t ms );
      int64_t mDuration; //in ms

      friend class DateTime;
  };

  class DateTime
  {
    public:

      enum Calendar
      {
        Gregorian = 0,
        Gregorian_proleptic,
        Julian,
      };

      //! Defaul constructor
      DateTime();
      //! Copy constructor
      DateTime( const DateTime &other );
      //! Constructor with date/time value and calendar type
      DateTime( int year, int month, int day, int hours = 0, int minutes = 0, double seconds = 0, Calendar calendar = Gregorian );
      //! Constructor with Jlian day
      DateTime( double julianDay );

      //! Returns a string with the date/time expressed in Greogrian/Julian calendar with ISO8601 format (local time zone)
      std::string toStandartCalendarISO8601() const;

      //! Returns the Julian day value
      double toJulianDay() const;

      //! Returns the Julain day value expressed with a string
      std::string toJulianDayString() const;

      //! operators
      DateTime &operator=( const DateTime &other );
      Duration operator-( const DateTime &other ) const;
      DateTime operator+( const Duration &duration ) const;
      DateTime &operator+=( const Duration &duration );
      DateTime &operator-=( const Duration &duration );
      DateTime operator-( const Duration &duration ) const;
      bool operator==( const DateTime &other ) const;
      bool operator!=( const DateTime &other ) const;
      bool operator<( const DateTime &other ) const;
      bool operator>( const DateTime &other ) const;
      bool operator>=( const DateTime &other ) const;
      bool operator<=( const DateTime &other ) const;

    private:

      struct DateTimeValues
      {
        int year;
        int month;
        int day;
        int hours;
        int minutes;
        double seconds;
      };

      DateTime( int64_t julianTime );

      DateTimeValues dateTimeGregorianJulianCalendar() const;

      void setWithGregorianCalendarDate( DateTimeValues values );
      void setWithJulianCalendarDate( DateTimeValues values );
      void setWithGregorianJulianCalendarDate( DateTimeValues values );//Uses the adapted formula depending of the date (< or > 1582-10-15)

      void setWith365dayCalendarDate( DateTimeValues values )
      {
        ///TODO
      }
      void setWith366dayCalendarDate( DateTimeValues values )
      {
        ///TODO
      }
      void setWith360dayCalendarDate( DateTimeValues values )
      {
        ///TODO
      }
      void setWithNonStandartCalendarDate( DateTimeValues values, std::vector<int> month_lengths, int leap_year, int leap_month )
      {
        ///TODO
      }

      std::string toString( DateTimeValues values ) const;

      int64_t mJulianTime = 0; //Julian day in ms

      bool mValid = true;
  };
}

#endif // MDAL_DATE_TIME_HPP
