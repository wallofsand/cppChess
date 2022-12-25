#ifndef TIME_H
#define TIME_H
#include <string>
/**
   A class that describes a time of day
   (between 00:00:00 and 23:59:59)
*/
class Time {
public:
    /**
       Constructs a time of day.
       @param hour the hours
       @param min the minutes
       @param sec the seconds
    */
    Time(int hour, int min, int sec);
    /**
       Constructs a Time object that is set to
       the time at which the constructor executes.
    */
    Time(); // default constructor

    /**
       Gets the hours of this time.
       @return the hours
    */
    int get_hours() const; // accessor function
    /**
       Gets the minutes of this time.
       @return the minutes
    */
    int get_minutes() const;
    /**
       Gets the seconds of this time.
       @return the seconds
    */
    int get_seconds() const;
    /**
       Gets the number of seconds in this time.
       @return the hours + min + seconds in seconds
    */
    int get_time_in_seconds() const;

    /**
       Computes the seconds between this time and another.
       @param t the other time
       @return the number of seconds between this time and t
    */
    int seconds_from(Time t) const;

    /**
       Adds a number of seconds to this time.
       @param s the number of seconds to add
    */
    void add_seconds(int s);

    // This function will print a Time object as such:
    // hh:mm:ss
    void print();

    // This function returns the time_in_secs formatted in hours, minutes, and seconds as a string.
    std::string to_string();

private:
    int time_in_secs; // data member
};

#endif
