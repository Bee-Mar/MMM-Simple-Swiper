"use strict";

Module.register("MMM-Simple-Swiper", {
  // value must be given for each of the parameters
  defaults: {
    echoLeftPin: 24, // GPIO #
    triggerLeftPin: 23, // GPIO #
    echoRightPin: 26, // GPIO #
    triggerRightPin: 25, // GPIO #
    threshold: 175, // centimeters
    distanceDiff: 1.25, // scaling difference between both sensors
    debug: false, // if true, the raw data is printed to stdout
    delay: 750 // time between passing data to node_helper in milliseconds
  },

  start: function() {
    this.sendSocketNotification("SWIPER_START_SWIPING", this.config);
  },

  socketNotificationReceived: function(notification, payload) {
    this.sendNotification(notification);
  }
});
