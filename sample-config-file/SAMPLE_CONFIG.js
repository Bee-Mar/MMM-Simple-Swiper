/* Magic Mirror Config Sample
 *
 * By Michael Teeuw http://michaelteeuw.nl
 * MIT Licensed.
 *
 * For more information how you can configurate this file
 * See https://github.com/MichMich/MagicMirror#configuration
 *
 */

var config = {
  address: "localhost", // Address to listen on, can be:
  // - "localhost", "127.0.0.1", "::1" to listen on loopback interface
  // - another specific IPv4/6 to listen on a specific interface
  // - "", "0.0.0.0", "::" to listen on any interface
  // Default, when address config is left out, is "localhost"
  port: 8080,
  ipWhitelist: ["127.0.0.1", "::ffff:127.0.0.1", "::1"], // Set [] to allow all IP addresses
  // or add a specific IPv4 of 192.168.1.5 :
  // ["127.0.0.1", "::ffff:127.0.0.1", "::1", "::ffff:192.168.1.5"],
  // or IPv4 range of 192.168.3.0 --> 192.168.3.15 use CIDR format :
  // ["127.0.0.1", "::ffff:127.0.0.1", "::1", "::ffff:192.168.3.0/28"],

  language: "en",
  timeFormat: 24,
  units: "metric",

  modules: [{
      module: "MMM-pages", // REQUIRED
      config: {
        modules: [
          [
            "weatherforecast", // feel free to remove or swap out
            "newsfeed", // feel free to remove or swap out
          ],
          [
            "calendar", // feel free to remove or swap out
            "compliments", // feel free to remove or swap out
          ],
        ],
        excludes: [
          "clock", // feel free to remove or swap out
          "currentweather", // feel free to remove or swap out
          "MMM-page-indicator", // REQUIRED
          "MMM-Simple-Swiper", // REQUIRED
        ],
      },
    },
    {
      module: "MMM-page-indicator", //REQUIRED
      position: "bottom_bar", // feel free to adjust
      config: {
        pages: 3, // feel free to adjust
      },
    },
    {
      module: "MMM-Simple-Swiper",
      disabled: false,
      config: {
        echoLeftPin: 24, // GPIO #
        triggerLeftPin: 23, // GPIO #
        echoRightPin: 26, // GPIO #
        triggerRightPin: 25, // GPIO #
        threshold: 175, // in centimeters
        distanceDiff: 1.25, // difference between both sensors
        debug: false, // if true, the raw data is printed to stdout while MagicMirror is running
        magicMirrorDelay: 1000, // number of milliseconds between passing data from C executable to the node_helper for MagicMirror to act upon
        sensorDelay: 10, // the number of microseconds each sensor waits to check for changes in voltage (fine tune this for CPU performance)
      },
    },
    {
      module: "alert", // feel free to remove or swap out
      disabled: false,
    },
    {
      module: "updatenotification", // feel free to remove or swap out
      position: "top_bar",
      disabled: false,
    },
    {
      module: "clock", // feel free to remove or swap out
      position: "top_right",
      timeFormat: 12,
      showPeriodUpper: true,
      disabled: false,
    },
    {
      module: "calendar", // feel free to remove or swap out
      header: "US Holidays",
      position: "top_right",
      disabled: false,
      config: {
        calendars: [{
          symbol: "calendar-check-o ",
          url: "webcal://www.calendarlabs.com/templates/ical/US-Holidays.ics",
        }, ],
      },
    },

    {
      module: "compliments", // feel free to remove or swap out
      position: "lower_third",
      disabled: true,
    },

    {
      module: "weatherforecast", // feel free to remove or swap out
      position: "top_right",
      header: "Weather Forecast",
      disabled: false,
      config: {
        location: "New York, NY, USA",
        units: "imperial",
        appid: "c0520f8e8537b2c7555a9f7d5c2d53ec",
      },
    },

    {
      module: "currentweather", // feel free to remove or swap out
      position: "top_right",
      disabled: false,
      config: {
        location: "New York, NY, USA",
        units: "imperial",
        appid: "c0520f8e8537b2c7555a9f7d5c2d53ec",
      },
    },

    {
      module: "newsfeed", // feel free to remove or swap out
      position: "bottom_bar",
      config: {
        feeds: [{
            title: "New York Times",
            url: "http://www.nytimes.com/services/xml/rss/nyt/HomePage.xml",
          },
          {
            title: "CNET",
            url: "https://www.cnet.com/rss/news/",
          },
          {
            title: "TechRepublic",
            url: "https://www.techrepublic.com/rssfeeds/articles/",
          },
        ],
        showSourceTitle: true,
        showPublishDate: true,
      },
    },
  ],
};

/*************** DO NOT EDIT THE LINE BELOW ***************/
if (typeof module !== "undefined") {
  module.exports = config;
}
